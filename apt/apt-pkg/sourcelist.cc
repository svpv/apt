// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: sourcelist.cc,v 1.25 2001/11/12 16:34:00 kojima Exp $
/* ######################################################################

   List of Sources and Vendors
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/sourcelist.h"
#endif

#include <apt-pkg/sourcelist.h>
#include <apt-pkg/error.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/tagfile.h>

#include <i18n.h>

#include <fstream.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
									/*}}}*/

// SourceList::pkgSourceList - Constructors				/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgSourceList::pkgSourceList()
{
}
/*
pkgSourceList::pkgSourceList(string File)
{
   ReadVendors(_config->FindFile("Dir::Etc::vendorlist"));

   Read(File);
}
*/									/*}}}*/
// SourceList::ReadMainList - Read the main source list from etc	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgSourceList::ReadMainList()
{
   ReadVendors(_config->FindFile("Dir::Etc::vendorlist"));

   return Read(_config->FindFile("Dir::Etc::sourcelist"));
}
									/*}}}*/

bool pkgSourceList::ReadVendors(string File)
{
   Configuration Cnf;
   
   ReadConfigFile(Cnf, File, true);
   
   // Process 'simple-key' type sections
   const Configuration::Item *Top = Cnf.Tree("simple-key");
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
   {
      Configuration Block(Top);
      VendorItem *vendor;

      vendor = new VendorItem;

      vendor->VendorID = Top->Tag;
      vendor->Fingerprint = Block.Find("Fingerprint");
      vendor->Name = Block.Find("Name"); // Description?

      if (vendor->Fingerprint.empty() == true || vendor->Name.empty() == true)
         _error->Error(_("Block %s is invalid"), vendor->VendorID.c_str());

      Vendors.push_back(vendor);
   }

   if (_error->PendingError())
       return false;

   return true;
}

// SourceList::Read - Parse the sourcelist file				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgSourceList::Read(string File)
{   
   // Open the stream for reading
   ifstream F(File.c_str(),ios::in | ios::nocreate);
   if (!F != 0)
      return _error->Errno("ifstream::ifstream","Opening %s",File.c_str());
   
   List.erase(List.begin(),List.end());
   char Buffer[300];

   int CurLine = 0;
   while (F.eof() == false)
   {
      F.getline(Buffer,sizeof(Buffer));
      CurLine++;
      _strtabexpand(Buffer,sizeof(Buffer));
      _strstrip(Buffer);
      
      // Comment or blank
      if (Buffer[0] == '#' || Buffer[0] == 0)
	 continue;
      
      // Grok it
      string Type;
      string URI;
      string VendorID;

      RepositoryItem *Rep = new RepositoryItem;

      const char *C = Buffer;
      if (ParseQuoteWord(C,Type) == false)
	 return _error->Error(_("Malformed line %u in source list %s (type)"),CurLine,File.c_str());
      if (ParseQuoteWord(C,URI) == false)
	 return _error->Error(_("Malformed line %u in source list %s (URI)"),CurLine,File.c_str());

      if (URI[0] == '[') {	 
	 const char *begin = URI.c_str()+1;
	 const char *end = strchr(begin, ']');
	 
	 if (!end)
	     return _error->Error(_("Malformed line %u in source list %s (vendor ID)"),CurLine,File.c_str());

	 VendorID = string(begin, end);

	 if (ParseQuoteWord(C,URI) == false)
	     return _error->Error(_("Malformed line %u in source list %s (URI)"),CurLine,File.c_str());
      } else {
	 VendorID = "";
	 Rep->Vendor = NULL;
      }
      if (ParseQuoteWord(C,Rep->Dist) == false)
	 return _error->Error(_("Malformed line %u in source list %s (dist)"),CurLine,File.c_str());

      if (!VendorID.empty()) {
	 if (Rep->SetVendor(Vendors, VendorID) == false)
	     return _error->Error(_("Malformed line %u in source list %s (bad vendor ID)"),CurLine,File.c_str());
      }
      if (Rep->SetType(Type) == false)
	 return _error->Error(_("Malformed line %u in source list %s (bad type)"),CurLine,File.c_str());
            
      if (Rep->SetURI(URI) == false)
	 return _error->Error(_("Malformed line %u in source list %s (bad URI)"),CurLine,File.c_str());

      // Check for an absolute dists specification.
      if (Rep->Dist.empty() == false && Rep->Dist[Rep->Dist.size() - 1] == '/')
      {
	 Item Itm;
	 Itm.Dist = Rep->Dist;
	 Itm.Repository = Rep;

	 if (ParseQuoteWord(C,Itm.Section) == true)
	    return _error->Error(_("Malformed line %u in source list %s (Absolute dist)"),CurLine,File.c_str());
	 Rep->Dist = SubstVar(Rep->Dist,"$(ARCH)",_config->Find("APT::Architecture"));

	 List.push_back(Itm);
	 
	 Repositories.push_back(Rep);
	 continue;
      }
            
      string Section;

      // Grab the rest of the dists
      if (ParseQuoteWord(C,Section) == false)
	  return _error->Error(_("Malformed line %u in source list %s (dist parse)"),CurLine,File.c_str());

      do
      {
	 Item Itm;

	 Itm.Section = Section;
	 Itm.Dist = SubstVar(Rep->Dist,"$(ARCH)",_config->Find("APT::Architecture"));
	 Itm.Repository = Rep;
	 List.push_back(Itm);
      }
      while (ParseQuoteWord(C, Section) == true);
      
      Repositories.push_back(Rep);
      
   
   }
   return true;
}
									/*}}}*/
// SourceList::Item << - Writes the item to a stream			/*{{{*/
// ---------------------------------------------------------------------
/* This is not suitable for rebuilding the sourcelist file but it good for
   debugging. */
ostream &operator <<(ostream &O,pkgSourceList::Item &Itm)
{
   O << (int)Itm.Type() << ' ' << Itm.URI() << ' ' << Itm.Dist << ' ' << Itm.Section;
   return O;
}
									/*}}}*/
// SourceList::Repository::SetType - Sets the distribution type		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgSourceList::RepositoryItem::SetType(string S)
{
   if (S == "deb")
   {
      Type = Deb;
      return true;
   }

   if (S == "deb-src")
   {
      Type = DebSrc;
      return true;
   }
   
   //akk
   if (S == "rpm")
   {
      Type = Rpm;
      return true;
   }
   
   if (S == "rpm-src")
   {
      Type = RpmSrc;
      return true;
   }

   return false;
}
									/*}}}*/

bool pkgSourceList::RepositoryItem::SetVendor(vector<VendorItem*> vendors,
					      string S)
{
   vector<VendorItem*>::const_iterator iter;

   if (S.empty()) 
   {
       Vendor = NULL;
       return false;
   }
   
   for (iter = vendors.begin();
	iter != vendors.end(); 
	iter++)
   {
      Vendor = *iter;

      if (Vendor->VendorID == S) 
      {
	  return true;
      }
   }
    
   Vendor = NULL;

   return false;
}


// SourceList::RepositoryItem::SetURI - Set the URI				/*{{{*/
// ---------------------------------------------------------------------
/* For simplicity we strip the scheme off the uri */
bool pkgSourceList::RepositoryItem::SetURI(string S)
{
   if (S.empty() == true)
      return false;

   if (S.find(':') == string::npos)
      return false;

   S = SubstVar(S,"$(ARCH)",_config->Find("APT::Architecture"));
   
   // Make sure that the URN is / postfixed
   URI = S;
   if (URI[URI.size() - 1] != '/')
      URI += '/';
   
   return true;
}
									/*}}}*/
// SourceList::Item::SiteOnly - Strip off the path part of a URI	/*{{{*/
// ---------------------------------------------------------------------
/* */
string pkgSourceList::RepositoryItem::SiteOnly(string URI)
{
   ::URI U(URI);
   U.User = string();
   U.Password = string();
   U.Path = string();
   U.Port = 0;
   return U;
}
									/*}}}*/


string pkgSourceList::RepositoryItem::HashesURI()
{   
   string Res;
   switch (Type)
   {
    case Deb:
      break;
      
    case DebSrc:
      break;
      
    case Rpm:
      Res = URI + Dist + "/base/hashfile";
      break;
      
    case RpmSrc:
      Res = URI + Dist + "/base/hashfile";
      break;
   };
   return Res;   
}


string pkgSourceList::RepositoryItem::HashesInfo()
{
   string Res;
   switch (Type)
   {
    case Deb:
      break;
     
    case DebSrc:
      break;
      
    case Rpm:
    case RpmSrc:
      Res = SiteOnly(URI) + ' ';
      Res += Dist + "/";
      Res += "base/hashfile";
      break;
   };
   return Res;
}


bool pkgSourceList::RepositoryItem::UpdateHashes(string File)
{
  // Open the stream for reading
   FileFd F(File, FileFd::ReadOnly);
   if (_error->PendingError())
      return _error->Error(_("could not open hash index"),File.c_str());

   pkgTagFile Tags(F);
   pkgTagSection Section;

   if (!Tags.Step(Section))
       return false;
   
   string Files = Section.FindS("MD5SUM");
   if (Files.empty()) {
      return _error->Error(_("No MD5SUM data in hashfile"));
   }
   // Iterate over the entire list grabbing each triplet
   const char *C = Files.c_str();
   while (*C != 0)
   {   
      string Size;
      string Hash;
      string Path;
      
      // Parse each of the elements
      if (ParseQuoteWord(C,Hash) == false ||
	  ParseQuoteWord(C,Size) == false ||
	  ParseQuoteWord(C,Path) == false)
	 return _error->Error(_("Error parsing MD5 hash record"));
      
      // Parse the size and append the directory
      HashIndex[Path].size = atoi(Size.c_str());
      HashIndex[Path].md5_hash = Hash;
   }
   
   return true;
}



bool pkgSourceList::RepositoryItem::MD5HashForFile(string file,
						   string &hash,
						   unsigned int &size)
{
    
   if (Vendor == NULL) {
      // means authentication is disabled
      size = 0;
      hash = "";
      return true;
   }
   if (HashIndex.count(file) == 0) {
      return _error->Error(_("Repository entry in sources.list contains extra components that are not listed in the signed hash file: %s"),
			   file.c_str());
   }
   if (HashIndex[file].md5_hash.empty())
       return false;
   hash = HashIndex[file].md5_hash;
   size = HashIndex[file].size;
   return true;
}


// SourceList::Item::PackagesURI - Returns a URI to the packages file	/*{{{*/
// ---------------------------------------------------------------------
/* */
string pkgSourceList::Item::PackagesURI(bool PathOnly) const
{
   string Res;
   string Prefix;
   
   if (PathOnly)
       Prefix = "";
   else
       Prefix = URI();
   
   switch (Type())
   {
    case Deb:
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    Res = Prefix + Dist;
	 else 
	    Res = Prefix;
      }      
      else
	 Res = Prefix + "dists/" + Dist + '/' + Section +
	 "/binary-" + _config->Find("APT::Architecture") + '/';
      
      Res += "Packages";
      break;
      
    case DebSrc:
      if (Dist[Dist.size() - 1] == '/')
	 Res = Prefix + Dist;
      else
	 Res = Prefix + "dists/" + Dist + '/' + Section +
	 "/source/";
      
      Res += "Sources";
      break;
      
    case Rpm:
       if (Dist[Dist.size()-1] == '/')
	  Res = Prefix + (Dist != "/" ? Dist : "") + "pkglist";
       else
	   Res = Prefix + Dist + "/base/pkglist."+Section;
      break;
      
    case RpmSrc:
       if (Dist[Dist.size()-1] == '/')
	   Res = Prefix + (Dist != "/" ? Dist : "") + "srclist";
       else
	   Res = Prefix + Dist + "/base/srclist."+Section;
      break;
      
    default:
      cout << "SHIT!!!"<<endl;
   };
   return Res;
}
									/*}}}*/
// SourceList::Item::PackagesInfo - Shorter version of the URI		/*{{{*/
// ---------------------------------------------------------------------
/* This is a shorter version that is designed to be < 60 chars or so */
string pkgSourceList::Item::PackagesInfo() const
{
   string Res;

   switch (Type())
   {
    case Deb:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	     Res += Dist;
      }      
      else
	  Res += Dist + '/' + Section;
      
      Res += " Packages";
      break;
      
    case DebSrc:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
	  Res += Dist;
      else
	  Res += Dist + '/' + Section;
      
      Res += " Sources";
      break;
      
    case Rpm:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
	  Res += (Dist != "/" ? Dist : "") + "pkglist";
      else
	  Res += Dist + "/base/pkglist." + Section;
      break;
      
    case RpmSrc:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
	  Res += (Dist != "/" ? Dist : "") + "srclist";
      else
	  Res += Dist + "/base/srclist." + Section;
      break;
   };
   return Res;
}
/*}}}*/

// SourceList::Item::ReleaseURI - Returns a URI to the release file	/*{{{*/
// ---------------------------------------------------------------------
/* */
string pkgSourceList::Item::ReleaseURI(bool PathOnly) const
{
   string Res;
   string Prefix;
   
   if (PathOnly)
       Prefix = "";
   else
       Prefix = URI();
   
   switch (Type())
   {
      case Deb:
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    Res = Prefix + Dist;
	 else
	    Res = Prefix;
      }      
      else
	 Res = Prefix + "dists/" + Dist + '/' + Section +
	 "/binary-" + _config->Find("APT::Architecture") + '/';
      
      Res += "Release";
      break;
      
      case DebSrc:
      if (Dist[Dist.size() - 1] == '/')
	 Res = Prefix + Dist;
      else
	 Res = Prefix + "dists/" + Dist + '/' + Section +
	 "/source/";
      
      Res += "Release";
      break;
      
    case Rpm:
    case RpmSrc:
      if (Dist[Dist.size() - 1] == '/')
	  Res = Prefix + (Dist != "/" ? Dist : "") + "release";
      else 
	  Res = Prefix + Dist + "/base/release."+Section;
      break;
   };
   return Res;
}
									/*}}}*/
// SourceList::Item::ReleaseInfo - Shorter version of the URI		/*{{{*/
// ---------------------------------------------------------------------
/* This is a shorter version that is designed to be < 60 chars or so */
string pkgSourceList::Item::ReleaseInfo() const
{
   string Res;
   
   switch (Type())
   {
    case Deb:
    case DebSrc:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    Res += Dist;
      }      
      else
	 Res += Dist + '/' + Section;
      
      Res += " Release";
      break;
      
    case Rpm:
    case RpmSrc:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	     Res += Dist;
	 Res += " release";
      } else {
	 Res += Dist;

	 Res += " release."+Section;
      }
      break;
   };
   return Res;
}
									/*}}}*/

// SourceList::Item::ArchiveInfo - Shorter version of the archive spec	/*{{{*/
// ---------------------------------------------------------------------
/* This is a shorter version that is designed to be < 60 chars or so */
string pkgSourceList::Item::ArchiveInfo(pkgCache::VerIterator Ver) const
{
   string Res;
   
   switch (Type())
   {
      case DebSrc:
      case Deb:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    Res += Dist;
      }      
      else
	 Res += Dist + '/' + Section;
      
      Res += " ";
      Res += Ver.ParentPkg().Name();
      Res += " ";
      Res += Ver.VerStr();

      break;
      
    case Rpm:
    case RpmSrc:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    Res += Dist;
      }      
      else
	 Res += Dist + '/' + Section;
      
      Res += " ";
      Res += Ver.ParentPkg().Name();
      Res += " ";
      Res += Ver.VerStr();
      break;
   };
   return Res;
}
									/*}}}*/
// SourceList::Item::ArchiveURI - Returns a URI to the given archive	/*{{{*/
// ---------------------------------------------------------------------
/* */
string pkgSourceList::Item::ArchiveURI(string File) const
{
   string Res;
   switch (Type())
   {
    case Deb:
    case DebSrc:
      Res = URI() + File;
      break;
    case Rpm:
      if (Dist[Dist.size()-1] == '/')
	  Res = URI()+(Dist != "/" ? Dist : "")+File;
      else
	  Res = URI() + Dist + "/RPMS." + Section + "/" + File;
      break;
    case RpmSrc:
      if (Dist[Dist.size()-1] == '/')
	  Res = URI()+(Dist != "/" ? Dist : "")+File;
      else
	  Res = URI() + Dist + "/../" + File;
      break;
   };
   return Res;
}
									/*}}}*/
// SourceList::Item::SourceInfo	- Returns an info line for a source	/*{{{*/
// ---------------------------------------------------------------------
/* */
string pkgSourceList::Item::SourceInfo(string Pkg,string Ver,string Comp) const
{
   string Res;
   
   switch (Type())
   {
    case DebSrc:
    case Deb:
    case RpmSrc:
    case Rpm:
      Res += Repository->SiteOnly(URI()) + ' ';
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    Res += Dist;
      }
      else
	 Res += Dist + '/' + Section;
      
      Res += " ";
      Res += Pkg;
      Res += " ";
      Res += Ver;
      if (Comp.empty() == false)
	 Res += " (" + Comp + ")";
      break;
   };
   return Res;
}
