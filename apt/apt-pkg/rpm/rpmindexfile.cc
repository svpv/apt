// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmindexfile.cc,v 1.4 2002/11/27 16:22:40 niemeyer Exp $
/* ######################################################################

   RPM Specific sources.list types and the three sorts of RPM
   index files.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmindexfile.h"
#endif

#include <config.h>

#ifdef HAVE_RPM

#include <apt-pkg/rpmindexfile.h>
#include <apt-pkg/rpmsrcrecords.h>
#include <apt-pkg/rpmlistparser.h>
#include <apt-pkg/rpmrecords.h>
#include <apt-pkg/rpmsystem.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmpackagedata.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/repository.h>

#include <apti18n.h>

#include <sys/stat.h>
									/*}}}*/
vector<pkgRepository *> RepList;

// rpmListIndex::Release* - Return the URI to the release file		/*{{{*/
// ---------------------------------------------------------------------
/* */
inline string rpmListIndex::ReleaseFile(string Type) const
{
   return URItoFileName(ReleaseURI(Type));
}

string rpmListIndex::ReleaseURI(string Type) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist + "/base/";

   Res += Type;
   
   if (rpmdata->HasIndexTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist;
      Dict["sect"] = "";
      Dict["type"] = Type;
      rpmdata->TranslateIndex(Res, Dict);
   }

   return Res;
}
									/*}}}*/
// rpmListIndex::ReleaseInfo - One liner describing the index URI	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmListIndex::ReleaseInfo(string Type) const 
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist;   
   Info += " ";
   Info += Type;
   return Info;
}
									/*}}}*/
// rpmListIndex::GetReleases - Fetch the index files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListIndex::GetReleases(pkgAcquire *Owner) const
{
   if (!Repository->Acquire)
      return true;
   Repository->Acquire = false;
   new pkgAcqIndexRel(Owner,Repository,ReleaseURI("release"),
		      ReleaseInfo("release"), "release", true);
   return true;
}
									/*}}}*/
// SrcListIndex::CreateHandler - Create a RPMHandler for this file	/*{{{*/
// ---------------------------------------------------------------------
RPMHandler *rpmSrcListIndex::CreateHandler() const
{
   return new RPMFileHandler(IndexFile("srclist"));
}
									/*}}}*/
// SrcListIndex::SourceInfo - Short 1 liner describing a source		/*{{{*/
// ---------------------------------------------------------------------
string rpmSrcListIndex::SourceInfo(pkgSrcRecords::Parser const &Record,
				   pkgSrcRecords::File const &File) const
{
   string Res;
   Res = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res += Dist;
   }      
   else
      Res += Dist + '/' + Section;
   
   Res += " ";
   Res += Record.Package();
   Res += " ";
   Res += Record.Version();
   if (File.Type.empty() == false)
      Res += " (" + File.Type + ")";
   return Res;
}
									/*}}}*/

// SrcListIndex::ArchiveURI - URI for the archive       	        /*{{{*/
// ---------------------------------------------------------------------
string rpmSrcListIndex::ArchiveURI(string File) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist;
   
   /* This should be fixed to include only the filename in
    * the parameter. Right now it includes the SRPMS.<sect> directory
    * information as well. It was written that way to make --flat work. */
   Res += "/" + File;

   if (rpmdata->HasSourceTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      string::size_type pos = File.rfind("/");
      if (pos != string::npos)
	 Dict["file"] = string(File, pos+1);
      else
	 Dict["file"] = File;
      
      rpmdata->TranslateSource(Res, Dict);
   }
	 
   return Res;
}
									/*}}}*/
// SrcListIndex::CreateSrcParser - Get a parser for the source files	/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgSrcRecords::Parser *rpmSrcListIndex::CreateSrcParser() const
{
   string SourcesURI;
   SourcesURI = IndexFile("srclist");
   return new rpmSrcRecordParser(_config->FindDir("Dir::State::lists") +
				 SourcesURI, this);
}
									/*}}}*/
// SrcListIndex::Describe - Give a descriptive path to the index	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmSrcListIndex::Describe(bool Short) const
{
   char S[300];
   if (Short == true)
      snprintf(S,sizeof(S),"%s",Info("pkglist").c_str());
   else
      snprintf(S,sizeof(S),"%s (%s)",Info("pkglist").c_str(),
         IndexFile("srclist").c_str());
   return S;
}
									/*}}}*/
// SrcListIndex::Info - One liner describing the index URI		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmSrcListIndex::Info(string Type) const
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist + '/' + Section;   
   Info += " ";
   Info += Type;
   return Info;
}
									/*}}}*/
// SrcListIndex::Index* - Return the URI to the index files		/*{{{*/
// ---------------------------------------------------------------------
/* */
inline string rpmSrcListIndex::IndexFile(string Type) const
{
   return URItoFileName(IndexURI(Type));
}

string rpmSrcListIndex::IndexURI(string Type) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist + "/base/";

   Res += Type+'.'+Section;

   if (rpmdata->HasIndexTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      Dict["type"] = Type;
      rpmdata->TranslateIndex(Res, Dict);
   }

   return Res;
}
									/*}}}*/
// SrcListIndex::GetIndexes - Fetch the index files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSrcListIndex::GetIndexes(pkgAcquire *Owner) const
{
   // Ignore indexes for repositories that could not be authenticated
   if (Repository->IsAuthenticated() == true && 
       Repository->HasRelease() == false)
      return true;
   new pkgAcqIndex(Owner,Repository,IndexURI("srclist"),Info("srclist"),
		   "srclist");
   return true;
}
									/*}}}*/
// SrcListIndex::Exists - Check if the index is available		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmSrcListIndex::Exists() const
{
   return FileExists(IndexFile("srclist"));
}
									/*}}}*/
// SrcListIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* */
unsigned long rpmSrcListIndex::Size() const
{
   struct stat S;
   if (stat(IndexFile("srclist").c_str(),&S) != 0)
      return 0;
   return S.st_size;
}
									/*}}}*/

// PkgListIndex::CreateHandler - Create a RPMHandler for this file	/*{{{*/
// ---------------------------------------------------------------------
RPMHandler *rpmPkgListIndex::CreateHandler() const
{
   return new RPMFileHandler(IndexFile("pkglist"));
}
									/*}}}*/
// PkgListIndex::ArchiveInfo - Short version of the archive url	        /*{{{*/
// ---------------------------------------------------------------------
/* This is a shorter version that is designed to be < 60 chars or so */
string rpmPkgListIndex::ArchiveInfo(pkgCache::VerIterator Ver) const
{
   string Res = ::URI::SiteOnly(URI) + ' ';
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
   return Res;
}
									/*}}}*/
// PkgListIndex::ArchiveURI - URI for the archive       	        /*{{{*/
// ---------------------------------------------------------------------
string rpmPkgListIndex::ArchiveURI(string File) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else
	 Res = URI;
   }
   else
      Res = URI + Dist;

   Res += "/RPMS."+Section + '/' + File;

   if (rpmdata->HasBinaryTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      Dict["file"] = File;
      rpmdata->TranslateBinary(Res, Dict);
   }
	 
   return Res;
}
									/*}}}*/
// PkgListIndex::Describe - Give a descriptive path to the index	/*{{{*/
// ---------------------------------------------------------------------
/* This should help the user find the index in the sources.list and
   in the filesystem for problem solving */
string rpmPkgListIndex::Describe(bool Short) const
{   
   char S[300];
   if (Short == true)
      snprintf(S,sizeof(S),"%s",Info("pkglist").c_str());
   else
      snprintf(S,sizeof(S),"%s (%s)",Info("pkglist").c_str(),
         IndexFile("pkglist").c_str());
   return S;
}
									/*}}}*/
// PkgListIndex::Info - One liner describing the index URI		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmPkgListIndex::Info(string Type) const 
{
   string Info = ::URI::SiteOnly(URI) + ' ';
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Info += Dist;
   }
   else
      Info += Dist + '/' + Section;   
   Info += " ";
   Info += Type;
   return Info;
}
									/*}}}*/
// PkgListIndex::Index* - Return the URI to the index files		/*{{{*/
// ---------------------------------------------------------------------
/* */
inline string rpmPkgListIndex::IndexFile(string Type) const
{
   return _config->FindDir("Dir::State::lists")+URItoFileName(IndexURI(Type));
}


string rpmPkgListIndex::IndexURI(string Type) const
{
   RPMPackageData *rpmdata = RPMPackageData::Singleton();
   string Res;
   if (Dist[Dist.size() - 1] == '/')
   {
      if (Dist != "/")
	 Res = URI + Dist;
      else 
	 Res = URI;
   }
   else
      Res = URI + Dist + "/base/";
   
   Res += Type + '.' + Section;

   if (rpmdata->HasIndexTranslation() == true)
   {
      map<string,string> Dict;
      Dict["uri"] = URI;
      Dict["dist"] = Dist; 
      Dict["sect"] = Section;
      Dict["type"] = Type;
      rpmdata->TranslateIndex(Res, Dict);
   }

   return Res;
}
									/*}}}*/
// PkgListIndex::GetIndexes - Fetch the index files			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::GetIndexes(pkgAcquire *Owner) const
{
   // Ignore indexes for repositories that could not be authenticated
   if (Repository->IsAuthenticated() == true && 
       Repository->HasRelease() == false)
      return true;
   new pkgAcqIndex(Owner,Repository,IndexURI("pkglist"),Info("pkglist"),
		   "pkglist");
   new pkgAcqIndexRel(Owner,Repository,IndexURI("release"),Info("release"),
		      "release");
   return true;
}
									/*}}}*/
// PkgListIndex::Exists - Check if the index is available		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::Exists() const
{
   return FileExists(IndexFile("pkglist"));
}
									/*}}}*/
// PkgListIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* This is really only used for progress reporting. */
unsigned long rpmPkgListIndex::Size() const
{
   struct stat S;
   if (stat(IndexFile("pkglist").c_str(),&S) != 0)
      return 0;
   return S.st_size;
}
									/*}}}*/
// PkgListIndex::Merge - Load the index file into a cache		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const
{
   string PackageFile = IndexFile("pkglist");
   RPMHandler *Handler = CreateHandler();

   Prog.SubProgress(0,Info("pkglist"));
   ::URI Tmp(URI);
   if (Gen.SelectFile(PackageFile,Tmp.Host,*this) == false)
   {
      delete Handler;
      return _error->Error(_("Problem with SelectFile %s"),PackageFile.c_str());
   }

   // Store the IMS information
   pkgCache::PkgFileIterator File = Gen.GetCurFile();
   struct stat St;
   if (stat(PackageFile.c_str(),&St) != 0) 
   {
      delete Handler;
      return _error->Errno("stat","Failed to stat");
   }
   File->Size = St.st_size;
   File->mtime = St.st_mtime;
   
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true) 
   {
      delete Handler;
      return _error->Error(_("Problem opening %s"),PackageFile.c_str());
   }
   
   if (Gen.MergeList(Parser) == false)
   {
      delete Handler;
      return _error->Error(_("Problem with MergeList %s"),PackageFile.c_str());
   }
   
   delete Handler;

   // Check the release file
   string RelFile = IndexFile("release");
   if (FileExists(RelFile) == true)
   {
      FileFd Rel(RelFile,FileFd::ReadOnly);
      if (_error->PendingError() == true)
	 return false;
      Parser.LoadReleaseInfo(File,Rel);
      Rel.Seek(0);
   }

   return true;
}
									/*}}}*/
// PkgListIndex::MergeFileProvides - Process file dependencies if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmPkgListIndex::MergeFileProvides(pkgCacheGenerator &Gen,
					OpProgress &Prog) const
{
   string PackageFile = IndexFile("pkglist");
   RPMHandler *Handler = CreateHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true) 
   {
      delete Handler;
      return _error->Error(_("Problem opening %s"),PackageFile.c_str());
   }
   Prog.SubProgress(0,Info("pkglist"));
   if (Gen.MergeFileProvides(Parser) == false)
      return _error->Error(_("Problem with MergeFileProvides %s"),
			   PackageFile.c_str());
   delete Handler;
   return true;
}
									/*}}}*/
// PkgListIndex::FindInCache - Find this index				/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCache::PkgFileIterator rpmPkgListIndex::FindInCache(pkgCache &Cache) const
{
   string FileName = IndexFile("pkglist");
   pkgCache::PkgFileIterator File = Cache.FileBegin();
   for (; File.end() == false; File++)
   {
      if (FileName != File.FileName())
	 continue;
      
      struct stat St;
      if (stat(File.FileName(),&St) != 0)
	 return pkgCache::PkgFileIterator(Cache);
      if ((unsigned)St.st_size != File->Size || St.st_mtime != File->mtime)
	 return pkgCache::PkgFileIterator(Cache);
      return File;
   }
   
   return File;
}
									/*}}}*/

// DatabaseIndex::rpmDatabaseIndex - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmDatabaseIndex::rpmDatabaseIndex()
{
}
									/*}}}*/
// DatabaseIndex::Size - Return the size of the index			/*{{{*/
// ---------------------------------------------------------------------
/* */
unsigned long rpmDatabaseIndex::Size() const
{
   return rpmSys.GetDBHandler()->Size();
}
									/*}}}*/
// DatabaseIndex::CreateHandler - Create a RPMHandler for this file	/*{{{*/
// ---------------------------------------------------------------------
RPMHandler *rpmDatabaseIndex::CreateHandler() const
{
   return rpmSys.GetDBHandler();
}
									/*}}}*/
// DatabaseIndex::Merge - Load the index file into a cache		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmDatabaseIndex::Merge(pkgCacheGenerator &Gen,OpProgress &Prog) const
{
   RPMDBHandler *Handler = rpmSys.GetDBHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true)
      return _error->Error(_("Problem opening RPM database"));
   
   Prog.SubProgress(0,"RPM Database");
   if (Gen.SelectFile(Handler->DataPath(false),string(),*this,pkgCache::Flag::NotSource) == false)
      return _error->Error(_("Problem with SelectFile RPM Database"));

   // Store the IMS information
   pkgCache::PkgFileIterator CFile = Gen.GetCurFile();
   struct stat St;
   if (stat(Handler->DataPath(false).c_str(),&St) != 0)
      return _error->Errno("fstat","Failed to stat");
   CFile->Size = St.st_size;
   CFile->mtime = St.st_mtime;
   
   if (Gen.MergeList(Parser) == false)
      return _error->Error(_("Problem with MergeList %s"),
			   Handler->DataPath(false).c_str());
   return true;
}
									/*}}}*/
// DatabaseIndex::MergeFileProvides - Process file dependencies if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmDatabaseIndex::MergeFileProvides(pkgCacheGenerator &Gen,
		   		  	 OpProgress &Prog) const
{
   RPMDBHandler *Handler = rpmSys.GetDBHandler();
   rpmListParser Parser(Handler);
   if (_error->PendingError() == true)
      return _error->Error(_("Problem opening RPM database"));
   Prog.SubProgress(0,"RPM Database");
   if (Gen.MergeFileProvides(Parser) == false)
      return _error->Error(_("Problem with MergeFileProvides %s"),
			   Handler->DataPath(false).c_str());
   return true;
}
									/*}}}*/
// DatabaseIndex::FindInCache - Find this index				/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCache::PkgFileIterator rpmDatabaseIndex::FindInCache(pkgCache &Cache) const
{
   pkgCache::PkgFileIterator File = Cache.FileBegin();
   for (; File.end() == false; File++)
   {
      if (rpmSys.GetDBHandler()->DataPath(false) != File.FileName())
	 continue;
      struct stat St;
      if (stat(File.FileName(),&St) != 0)
	 return pkgCache::PkgFileIterator(Cache);
      if ((unsigned)St.st_size != File->Size || St.st_mtime != File->mtime)
	 return pkgCache::PkgFileIterator(Cache);
      return File;
   }   
   return File;
}
									/*}}}*/

// Source List types for rpm						/*{{{*/

class rpmSLTypeGen : public pkgSourceList::Type
{
   public:

   rpmSLTypeGen()
   {
      Name = "rpm";
      Label = "Standard RPM source tree";
   }   

   pkgRepository *FindRepository(string URI,string Dist,
				 const pkgSourceList::Vendor *Vendor) const
   {
      for (vector<pkgRepository *>::const_iterator iter = RepList.begin();
	   iter != RepList.end(); iter++) 
      {
	 if ((*iter)->URI == URI && (*iter)->Dist == Dist) 
	 {	 
	    // The registered object has no vendor, but the new one does
	    // so, we make the previous one have the vendor too.
	    if ((*iter)->Vendor == NULL && Vendor != NULL)
	       (*iter)->Vendor = Vendor;
	    return *iter;
	 }
      }
      return NULL;
   }

   pkgRepository *GetRepository(string URI,string Dist,
				const pkgSourceList::Vendor *Vendor) const
   {
      pkgRepository *Rep = FindRepository(URI,Dist,Vendor);
      if (Rep != NULL)
	 return Rep;

      string BaseURI;
      if (Dist[Dist.size() - 1] == '/')
      {
	 if (Dist != "/")
	    BaseURI = URI + Dist;
	 else 
	    BaseURI = URI + '/';
      }
      else
	 BaseURI = URI + Dist + '/';

      Rep = new pkgRepository(URI,Dist,Vendor,BaseURI);
      RepList.push_back(Rep);
      return Rep;
   }
};


class rpmSLTypeRpm : public rpmSLTypeGen
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmPkgListIndex(URI,Dist,Section,Rep));
      return true;
   };

   rpmSLTypeRpm()
   {
      Name = "rpm";
      Label = "Standard RPM binary tree";
   }   
};

class rpmSLTypeSrpm : public rpmSLTypeGen
{
   public:

   bool CreateItem(vector<pkgIndexFile *> &List,
		   string URI, string Dist, string Section,
		   pkgSourceList::Vendor const *Vendor) const 
   {
      pkgRepository *Rep = GetRepository(URI,Dist,Vendor);
      List.push_back(new rpmSrcListIndex(URI,Dist,Section,Rep));
      return true;
   };  
   
   rpmSLTypeSrpm()
   {
      Name = "rpm-src";
      Label = "Standard RPM source tree";
   }   
};

rpmSLTypeRpm _apt_rpmType;
rpmSLTypeSrpm _apt_rpmSrcType;
									/*}}}*/
// Index File types for rpm						/*{{{*/
class rpmIFTypeSrc : public pkgIndexFile::Type
{
   public:
   
   rpmIFTypeSrc() {Label = "RPM Source Index";};
};
class rpmIFTypePkg : public pkgIndexFile::Type
{
   public:
   
   virtual pkgRecords::Parser *CreatePkgParser(pkgCache::PkgFileIterator File) const
   {
      return new rpmRecordParser(File.FileName(),*File.Cache());
   };
   rpmIFTypePkg() {Label = "RPM Package Index";};
};
class rpmIFTypeDatabase : public pkgIndexFile::Type
{
   public:
   
   virtual pkgRecords::Parser *CreatePkgParser(pkgCache::PkgFileIterator File) const
   {
      return new rpmRecordParser(File.FileName(),*File.Cache());
   };
   rpmIFTypeDatabase() {Label = "RPM Database";};
};
static rpmIFTypeSrc _apt_Src;
static rpmIFTypePkg _apt_Pkg;
static rpmIFTypeDatabase _apt_DB;

const pkgIndexFile::Type *rpmSrcListIndex::GetType() const
{
   return &_apt_Src;
}
const pkgIndexFile::Type *rpmPkgListIndex::GetType() const
{
   return &_apt_Pkg;
}
const pkgIndexFile::Type *rpmDatabaseIndex::GetType() const
{
   return &_apt_DB;
}

									/*}}}*/
#endif /* HAVE_RPM */

// vim:sts=3:sw=3
