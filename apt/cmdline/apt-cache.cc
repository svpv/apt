// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: apt-cache.cc,v 1.20 2001/08/01 21:57:04 kojima Exp $

/* ######################################################################
   
   apt-cache - Manages the cache files
   
   apt-cache provides some functions fo manipulating the cache files.
   It uses the command line interface common to all the APT tools. The
   only really usefull function right now is dumpavail which is used
   by the dselect method. Everything else is meant as a debug aide.
   
   Returns 100 on failure, 0 on success.
   
   ##################################################################### 
 */

									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/error.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/init.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/cmndline.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/srcrecords.h>

#include <apt-pkg/rpminit.h> //akk

//akk#include <apt-pkg/debfactory.h>
#include <apt-pkg/rpmfactory.h>
#include <apt-pkg/rpmrecords.h>

#include <config.h>

#include <i18n.h>

#include <iostream.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <stdio.h>

#include <rpm/rpmlib.h>//akk

									/*}}}*/

pkgCache *GCache = 0;

// UnMet - Show unmet dependencies					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool UnMet(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   bool Important = _config->FindB("APT::Cache::Important",false);
   
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      for (pkgCache::VerIterator V = P.VersionList(); V.end() == false; V++)
      {
	 bool Header = false;
	 for (pkgCache::DepIterator D = V.DependsList(); D.end() == false;)
	 {
	    // Collect or groups
	    pkgCache::DepIterator Start;
	    pkgCache::DepIterator End;
	    D.GlobOr(Start,End);
	    
/*	    cout << "s: Check " << Start.TargetPkg().Name() << ',' <<
	       End.TargetPkg().Name() << endl;*/
	       
	    // Skip conflicts and replaces
	    if (End->Type != pkgCache::Dep::PreDepends &&
		End->Type != pkgCache::Dep::Depends && 
		End->Type != pkgCache::Dep::Suggests &&
		End->Type != pkgCache::Dep::Recommends)
	       continue;

	    // Important deps only
	    if (Important == true)
	       if (End->Type != pkgCache::Dep::PreDepends &&
		   End->Type != pkgCache::Dep::Depends)
		  continue;
	    
	    // Verify the or group
	    bool OK = false;
	    pkgCache::DepIterator RealStart = Start;
	    do
	    {
	       // See if this dep is Ok
	       pkgCache::Version **VList = Start.AllTargets();
	       if (*VList != 0)
	       {
		  OK = true;
		  delete [] VList;
		  break;
	       }
	       delete [] VList;
	       
	       if (Start == End)
		  break;
	       Start++;
	    }
	    while (1);

	    // The group is OK
	    if (OK == true)
	       continue;
	    
	    // Oops, it failed..
	    if (Header == false)
		  cout << _("Package ") << P.Name() << _(" version ") <<
	       V.VerStr() << _(" has an unmet dep:") << endl;
	    Header = true;
	    
	    // Print out the dep type
	    cout << " " << End.DepType() << ": ";

	    // Show the group
	    Start = RealStart;
	    do
	    {
	       cout << Start.TargetPkg().Name();
	       if (Start.TargetVer() != 0)
		  cout << " (" << Start.CompType() << " " << Start.TargetVer() <<
		  ")";
	       if (Start == End)
		  break;
	       cout << " | ";
	       Start++;
	    }
	    while (1);
	    
	    cout << endl;
	 }	 
      }
   }   
   return true;
}
									/*}}}*/
// DumpPackage - Show a dump of a package record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DumpPackage(CommandLine &CmdL)
{   
   pkgCache &Cache = *GCache;
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }

      cout << _("Package: ") << Pkg.Name() << endl;
      cout << _("Versions: ");
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); Cur.end() != true; Cur++)
      {
	 cout << Cur.VerStr();
	 for (pkgCache::VerFileIterator Vf = Cur.FileList(); Vf.end() == false; Vf++)
	    cout << "(" << Vf.File().FileName() << ")";
	 cout << ',';
      }
      
      cout << endl;
      
      cout << _("Reverse Depends: ") << endl;
      for (pkgCache::DepIterator D = Pkg.RevDependsList(); D.end() != true; D++)
	 cout << "  " << D.ParentPkg().Name() << ',' << D.TargetPkg().Name() << endl;

      cout << _("Dependencies: ") << endl;
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); Cur.end() != true; Cur++)
      {
	 cout << Cur.VerStr() << " - ";
	 for (pkgCache::DepIterator Dep = Cur.DependsList(); Dep.end() != true; Dep++)
	    cout << Dep.TargetPkg().Name() << " (" << (int)Dep->CompareOp << " " << Dep.TargetVer() << ") ";
	 cout << endl;
      }      

      cout << _("Provides: ") << endl;
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); Cur.end() != true; Cur++)
      {
	 cout << Cur.VerStr() << " - ";
	 for (pkgCache::PrvIterator Prv = Cur.ProvidesList(); Prv.end() != true; Prv++)
	    cout << Prv.ParentPkg().Name() << " ";
	 cout << endl;
      }
      cout << _("Reverse Provides: ") << endl;
      for (pkgCache::PrvIterator Prv = Pkg.ProvidesList(); Prv.end() != true; Prv++)
	 cout << Prv.OwnerPkg().Name() << " " << Prv.OwnerVer().VerStr() << endl;
   }

   return true;
}
									/*}}}*/
// Stats - Dump some nice statistics					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Stats(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;
   cout << _("Total Package Names : ") << Cache.Head().PackageCount << " (" <<
      SizeToStr(Cache.Head().PackageCount*Cache.Head().PackageSz) << ')' << endl;
   pkgCache::PkgIterator I = Cache.PkgBegin();
   
   int Normal = 0;
   int Virtual = 0;
   int NVirt = 0;
   int DVirt = 0;
   int Missing = 0;
   for (;I.end() != true; I++)
   {
      if (I->VersionList != 0 && I->ProvidesList == 0)
      {
	 Normal++;
	 continue;
      }

      if (I->VersionList != 0 && I->ProvidesList != 0)
      {
	 NVirt++;
	 continue;
      }
      
      if (I->VersionList == 0 && I->ProvidesList != 0)
      {
	 // Only 1 provides
	 if (I.ProvidesList()->NextProvides == 0)
	 {
	    DVirt++;
	 }
	 else
	    Virtual++;
	 continue;
      }
      if (I->VersionList == 0 && I->ProvidesList == 0)
      {
	 Missing++;
	 continue;
      }
   }
   cout << _("  Normal Packages: ") << Normal << endl;
   cout << _("  Pure Virtual Packages: ") << Virtual << endl;
   cout << _("  Single Virtual Packages: ") << DVirt << endl;
   cout << _("  Mixed Virtual Packages: ") << NVirt << endl;
   cout << _("  Missing: ") << Missing << endl;
   
   cout << _("Total Distinct Versions: ") << Cache.Head().VersionCount << " (" <<
      SizeToStr(Cache.Head().VersionCount*Cache.Head().VersionSz) << ')' << endl;
   cout << _("Total Dependencies: ") << Cache.Head().DependsCount << " (" << 
      SizeToStr(Cache.Head().DependsCount*Cache.Head().DependencySz) << ')' << endl;
   
   cout << _("Total Ver/File relations: ") << Cache.Head().VerFileCount << " (" <<
      SizeToStr(Cache.Head().VerFileCount*Cache.Head().VerFileSz) << ')' << endl;
   cout << _("Total Provides Mappings: ") << Cache.Head().ProvidesCount << " (" <<
      SizeToStr(Cache.Head().ProvidesCount*Cache.Head().ProvidesSz) << ')' << endl;
   
   // String list stats
   unsigned long Size = 0;
   unsigned long Count = 0;
   for (pkgCache::StringItem *I = Cache.StringItemP + Cache.Head().StringList;
        I!= Cache.StringItemP; I = Cache.StringItemP + I->NextItem)
   {
      Count++;
      Size += strlen(Cache.StrP + I->String);
   }
   cout << _("Total Globbed Strings: ") << Count << " (" << SizeToStr(Size) << ')' << endl;
      
   unsigned long Slack = 0;
   for (int I = 0; I != 7; I++)
      Slack += Cache.Head().Pools[I].ItemSize*Cache.Head().Pools[I].Count;
   cout << _("Total Slack space: ") << SizeToStr(Slack) << endl;
   
   unsigned long Total = 0;
   Total = Slack + Size + Cache.Head().DependsCount*Cache.Head().DependencySz + 
           Cache.Head().VersionCount*Cache.Head().VersionSz +
           Cache.Head().PackageCount*Cache.Head().PackageSz + 
           Cache.Head().VerFileCount*Cache.Head().VerFileSz +
           Cache.Head().ProvidesCount*Cache.Head().ProvidesSz;
   cout << _("Total Space Accounted for: ") << SizeToStr(Total) << endl;
   
   return true;
}
									/*}}}*/
// Check - Check some things about the cache				/*{{{*/
// ---------------------------------------------------------------------
/* Debug aide mostly */
bool Check(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;
   pkgCache::PkgIterator Pkg = Cache.PkgBegin();
   for (;Pkg.end() != true; Pkg++)
   {
      if (Pkg.Section() == 0 && Pkg->VersionList != 0)
	 cout << _("Bad section ") << Pkg.Name() << endl;
      
      for (pkgCache::VerIterator Cur = Pkg.VersionList(); 
	   Cur.end() != true; Cur++)
      {
	 if (Cur->Priority < 1 || Cur->Priority > 5)
	    cout << _("Bad prio ") << Pkg.Name() << ',' << Cur.VerStr() << " == " << (int)Cur->Priority << endl;
      }
   }
   return true;
}
									/*}}}*/
// Dump - show everything						/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Dump(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;
   for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
   {
      cout << _("Package: ") << P.Name() << endl;
      for (pkgCache::VerIterator V = P.VersionList(); V.end() == false; V++)
      {
	 cout << _(" Version: ") << V.VerStr() << endl;
	 cout << _("     File: ") << V.FileList().File().FileName() << endl;
	 for (pkgCache::DepIterator D = V.DependsList(); D.end() == false; D++)
	    cout << _("  Depends: ") << D.TargetPkg().Name() << ' ' << D.TargetVer() << endl;
      }      
   }

   for (pkgCache::PkgFileIterator F(Cache); F.end() == false; F++)
   {
      cout << _("File: ") << F.FileName() << endl;
      cout << _(" Size: ") << F->Size << endl;
      cout << _(" ID: ") << F->ID << endl;
      cout << _(" Flags: ") << F->Flags << endl;
      cout << _(" Time: ") << TimeRFC1123(F->mtime) << endl;
      cout << _(" Archive: ") << F.Archive() << endl;
      cout << _(" Component: ") << F.Component() << endl;
      cout << _(" Version: ") << F.Version() << endl;
      cout << _(" Origin: ") << F.Origin() << endl;
      cout << _(" Label: ") << F.Label() << endl;
      cout << _(" Architecture: ") << F.Architecture() << endl;
   }

   return true;
}
									/*}}}*/
// DumpAvail - Print out the available list				/*{{{*/
// ---------------------------------------------------------------------
/* This is needed to make dpkg --merge happy */
bool DumpAvail(CommandLine &Cmd)
{
   pkgCache &Cache = *GCache;
   unsigned char *Buffer = new unsigned char[Cache.HeaderP->MaxVerFileSize];

   for (pkgCache::PkgFileIterator I = Cache.FileBegin(); I.end() == false; I++)
   {
      if ((I->Flags & pkgCache::Flag::NotSource) != 0)
	 continue;
      
      if (I.IsOk() == false)
      {
	 delete [] Buffer;
	 return _error->Error(_("Package file %s is out of sync."),I.FileName());
      }
      
      FileFd PkgF(I.FileName(),FileFd::ReadOnly);
      if (_error->PendingError() == true)
      {
	 delete [] Buffer;
	 return false;
      }

      /* Write all of the records from this package file, we search the entire
         structure to find them */
      for (pkgCache::PkgIterator P = Cache.PkgBegin(); P.end() == false; P++)
      {
	 // Find the proper version to use. We should probably use the DepCache.
	 pkgCache::VerIterator V = Cache.GetCandidateVer(P,false);

	 if (V.end() == true || V.FileList().File() != I)
	    continue;
	 
	 // Read the record and then write it out again.
	 if (PkgF.Seek(V.FileList()->Offset) == false ||
	     PkgF.Read(Buffer,V.FileList()->Size) == false ||
	     write(STDOUT_FILENO,Buffer,V.FileList()->Size) != V.FileList()->Size)
	 {
	    delete [] Buffer;
	    return false;
	 }	 
      }
   }
   
   return true;
}
									/*}}}*/
// Depends - Print out a dependency tree				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool Depends(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      
      pkgCache::VerIterator Ver = Pkg.VersionList();
      if (Ver.end() == true)
      {
	 cout << '<' << Pkg.Name() << '>' << endl;
	 continue;
      }

      cout << Pkg.Name() << endl;
      
      for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
      {
	 if ((D->CompareOp & pkgCache::Dep::Or) == pkgCache::Dep::Or)
	    cout << " |";
	 else
	    cout << "  ";
	 
	 // Show the package
	 pkgCache::PkgIterator Trg = D.TargetPkg();
	 if (Trg->VersionList == 0)
	    cout << D.DepType() << ": <" << Trg.Name() << ">" << endl;
	 else
	    cout << D.DepType() << ": " << Trg.Name() << endl;
	    
	 // Display all solutions
	 pkgCache::Version **List = D.AllTargets();
	 for (pkgCache::Version **I = List; *I != 0; I++)
	 {
	    pkgCache::VerIterator V(Cache,*I);
	    if (V != Cache.VerP + V.ParentPkg()->VersionList || 
		V->ParentPkg == D->Package)
	       continue;
	    cout << "    " << V.ParentPkg().Name() << endl;
	 }
	 delete [] List;
      }
   }   
   
   return true;
}
									/*}}}*/
// Dotty - Generate a graph for Dotty					/*{{{*/
// ---------------------------------------------------------------------
/* Dotty is the graphvis program for generating graphs. It is a fairly
   simple queuing algorithm that just writes dependencies and nodes. 
   http://www.research.att.com/sw/tools/graphviz/ */
bool Dotty(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   bool GivenOnly = _config->FindB("APT::Cache::GivenOnly",false);
   
   /* Normal packages are boxes
      Pure Provides are triangles
      Mixed are diamonds
      Hexagons are missing packages*/
   const char *Shapes[] = {"hexagon","triangle","box","diamond"};
   
   /* Initialize the list of packages to show.
      1 = To Show
      2 = To Show no recurse
      3 = Emitted no recurse
      4 = Emitted
      0 = None */
   enum States {None=0, ToShow, ToShowNR, DoneNR, Done};
   enum TheFlags {ForceNR=(1<<0)};
   unsigned char *Show = new unsigned char[Cache.Head().PackageCount];
   unsigned char *Flags = new unsigned char[Cache.Head().PackageCount];
   unsigned char *ShapeMap = new unsigned char[Cache.Head().PackageCount];
   
   // Show everything if no arguments given
   if (CmdL.FileList[1] == 0)
      for (unsigned long I = 0; I != Cache.Head().PackageCount; I++)
	 Show[I] = ToShow;
   else
      for (unsigned long I = 0; I != Cache.Head().PackageCount; I++)
	 Show[I] = None;
   memset(Flags,0,sizeof(*Flags)*Cache.Head().PackageCount);
   
   // Map the shapes
   for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
   {   
      if (Pkg->VersionList == 0)
      {
	 // Missing
	 if (Pkg->ProvidesList == 0)
	    ShapeMap[Pkg->ID] = 0;
	 else
	    ShapeMap[Pkg->ID] = 1;
      }
      else
      {
	 // Normal
	 if (Pkg->ProvidesList == 0)
	    ShapeMap[Pkg->ID] = 2;
	 else
	    ShapeMap[Pkg->ID] = 3;
      }
   }
   
   // Load the list of packages from the command line into the show list
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      // Process per-package flags
      string P = *I;
      bool Force = false;
      if (P.length() > 3)
      {
	 if (P.end()[-1] == '^')
	 {
	    Force = true;
	    P.erase(P.end()-1);
	 }
	 
	 if (P.end()[-1] == ',')
	    P.erase(P.end()-1);
      }
      
      // Locate the package
      pkgCache::PkgIterator Pkg = Cache.FindPkg(P);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      Show[Pkg->ID] = ToShow;
      
      if (Force == true)
	 Flags[Pkg->ID] |= ForceNR;
   }
   
   // Little header
   printf("digraph packages {\n");
   printf("concentrate=true;\n");
   printf("size=\"30,40\";\n");
   
   bool Act = true;
   while (Act == true)
   {
      Act = false;
      for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
      {
	 // See we need to show this package
	 if (Show[Pkg->ID] == None || Show[Pkg->ID] >= DoneNR)
	    continue;
	 
	 // Colour as done
	 if (Show[Pkg->ID] == ToShowNR || (Flags[Pkg->ID] & ForceNR) == ForceNR)
	 {
	    // Pure Provides and missing packages have no deps!
	    if (ShapeMap[Pkg->ID] == 0 || ShapeMap[Pkg->ID] == 1)
	       Show[Pkg->ID] = Done;
	    else
	       Show[Pkg->ID] = DoneNR;
	 }	 
	 else
	    Show[Pkg->ID] = Done;
	 Act = true;

	 // No deps to map out
	 if (Pkg->VersionList == 0 || Show[Pkg->ID] == DoneNR)
	    continue;
	 
	 pkgCache::VerIterator Ver = Pkg.VersionList();
	 for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
	 {
	    // See if anything can meet this dep
	    // Walk along the actual package providing versions
	    bool Hit = false;
	    pkgCache::PkgIterator DPkg = D.TargetPkg();
	    for (pkgCache::VerIterator I = DPkg.VersionList();
		      I.end() == false && Hit == false; I++)
	    {
	       if (_system->checkDep(D.TargetVer(),I.VerStr(),D->CompareOp) == true)
		  Hit = true;
	    }
	    
	    // Follow all provides
	    for (pkgCache::PrvIterator I = DPkg.ProvidesList(); 
		      I.end() == false && Hit == false; I++)
	    {
	       if (_system->checkDep(D.TargetVer(),I.ProvideVersion(),D->CompareOp) == false)
		  Hit = true;
	    }
	    
	    // Only graph critical deps	    
	    if (D.IsCritical() == true)
	    {
	       printf("\"%s\" -> \"%s\"",Pkg.Name(),D.TargetPkg().Name());
	       
	       // Colour the node for recursion
	       if (Show[D.TargetPkg()->ID] <= DoneNR)
	       {
		  /* If a conflicts does not meet anything in the database
		     then show the relation but do not recurse */
		  if (Hit == false && D->Type == pkgCache::Dep::Conflicts)
		  {
		     if (Show[D.TargetPkg()->ID] == None && 
			 Show[D.TargetPkg()->ID] != ToShow)
			Show[D.TargetPkg()->ID] = ToShowNR;
		  }		  
		  else
		  {
		     if (GivenOnly == true && Show[D.TargetPkg()->ID] != ToShow)
			Show[D.TargetPkg()->ID] = ToShowNR;
		     else
			Show[D.TargetPkg()->ID] = ToShow;
		  }
	       }
	       
	       // Edge colour
	       switch(D->Type)
	       {
		  case pkgCache::Dep::Conflicts:
		  printf("[color=springgreen];\n");
		  break;
		  
		  case pkgCache::Dep::PreDepends:
		  printf("[color=blue];\n");
		  break;
		  
		  default:
		  printf(";\n");
		  break;
	       }	       
	    }	    
	 }
      }
   }   
   
   /* Draw the box colours after the fact since we can not tell what colour
      they should be until everything is finished drawing */
   for (pkgCache::PkgIterator Pkg = Cache.PkgBegin(); Pkg.end() == false; Pkg++)
   {
      if (Show[Pkg->ID] < DoneNR)
	 continue;
      
      // Orange box for early recursion stoppage
      if (Show[Pkg->ID] == DoneNR)
	 printf("\"%s\" [color=orange,shape=%s];\n",Pkg.Name(),
		Shapes[ShapeMap[Pkg->ID]]);
      else
	 printf("\"%s\" [shape=%s];\n",Pkg.Name(),
		Shapes[ShapeMap[Pkg->ID]]);
   }
   
   printf("}\n");
   return true;
}
									/*}}}*/
// DoAdd - Perform an adding operation					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoAdd(CommandLine &CmdL)
{
   // Make sure there is at least one argument
   if (CmdL.FileSize() <= 1)
      return _error->Error(_("You must give at least one file name"));
   
   // Open the cache
   FileFd CacheF(_config->FindFile("Dir::Cache::pkgcache"),FileFd::WriteAny);
   if (_error->PendingError() == true)
      return false;
   
   DynamicMMap Map(CacheF,MMap::Public);
   if (_error->PendingError() == true)
      return false;

   OpTextProgress Progress(*_config);
   pkgCacheGenerator Gen(Map,Progress);
   if (_error->PendingError() == true)
      return false;

   unsigned long Length = CmdL.FileSize() - 1;
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      Progress.OverallProgress(I - CmdL.FileList,Length,1,_("Generating cache"));
      Progress.SubProgress(Length);

      // Do the merge
      FileFd TagF(*I,FileFd::ReadOnly);
      pkgCacheGenerator::ListParser *Parser = _system->CreateListParser(TagF);
       
      if (_error->PendingError() == true)
	 return _error->Error(_("Problem opening %s"),*I);
      
      if (Gen.SelectFile(*I) == false)
	 return _error->Error(_("Problem with SelectFile"));

      if (Gen.MergeList(*Parser, 0) == false)
	 return _error->Error(_("Problem with MergeList"));
   }

   Progress.Done();
   GCache = &Gen.GetCache();
   Stats(CmdL);
   
   return true;
}
									/*}}}*/
// DisplayRecord - Displays the complete record for the package		/*{{{*/
// ---------------------------------------------------------------------
/* This displays the package record from the proper package index file. 
   It is not used by DumpAvail for performance reasons. */
#if 1
#define CRPMTAG_FILENAME 1000000
#define CRPMTAG_FILESIZE 1000001
#define CRPMTAG_ESSENTIAL 1000002
#define CRPMTAG_MD5      1000005




void pstrarray(FILE *f, char **str, int count)
{
   if (!count)
     return;

   fprintf(f, "%s", *str++);
   count--;
   while (count--) {
      fprintf(f, ", %s", *str++);
   }
   fprintf(f, "\n");
}

void pstr(FILE *f, char **data, int count, int type) 
{
   if (type == RPM_STRING_TYPE) {
      fprintf(f, "%s\n", (char*)data);
   } else if (type == RPM_STRING_ARRAY_TYPE) {
      pstrarray(f, data, count);
   } else {
      puts("Oh shit!");
      abort();
   }
}


void pitem(FILE *f, char *file, char *version, int flags)
{
   fputs(file, f);
   if (*version) {
      int c = 0;
      fputs(" (", f);
      /*
       * For some reason, debian ppl decided that:
       * > and < are the same as >=, <=
       * >> and << is the same as >, <
       */
      if (flags & RPMSENSE_LESS) {
	 fputc('<', f);
	 c = '<';
      }
      if (flags & RPMSENSE_GREATER) {
	 fputc('>', f);
	 c = '>';
      }
      if (flags & RPMSENSE_EQUAL) {
	 fputc('=', f);
      } else {
	 if (c)
	   fputc(c, f);
      }
      fprintf(f, " %s)", version);
   }
}


void ptuplearray(FILE *f, char **str1, char **str2, int *flags, int count)
{
   int first = 1;
   if (!count)
     return;

   while (count--) {
       if (!first)
	   fputs(", ", f);
       first = 0;
       pitem(f, *str1, *str2, *flags);
       str1++; str2++; flags++;
   }
   fputc('\n', f);
}

void pdepend(FILE *f, char **data1, char **data2, int *flags, int count, 
	    int type)
{
   if (type == RPM_STRING_TYPE) {
      pitem(f, (char*)data1, (char*)data2, *flags);
      fprintf(f, "\n");
   } else if (type == RPM_STRING_ARRAY_TYPE) {
      ptuplearray(f, data1, data2, flags, count);
   } else {
      puts(_("Oh shit!"));
      abort();
   }
}


void dumpDescription(FILE *f, char *descr)
{
   int nl;
   
   nl = 1;
   while (*descr) {
      if (nl) {
	 fputc(' ', f);
	 nl = 0;
      }
      if (*descr=='\n') {
	 nl = 1;
      }
      fputc(*descr++, f);
   }
   fputc('\n', f);
}


void showHeader(FILE *f, Header hdr)
{
   int type, type2, type3, count;
   char *str;
   char **strv;
   char **strv2;
   int num;
   int_32 *numv;

   headerGetEntry(hdr, RPMTAG_NAME, &type, (void **)&str, &count);
   fprintf(f, _("Package: %s\n"), str);

   headerGetEntry(hdr, RPMTAG_GROUP, &type, (void **)&str, &count);
   fprintf(f, _("Section: %s\n"), str);

   headerGetEntry(hdr, RPMTAG_SIZE, &type, (void **)&numv, &count);
   fprintf(f, _("Installed Size: %i\n"), numv[0] / 1000);

   // Another kludge, --claudio
   str = NULL;
   headerGetEntry(hdr, RPMTAG_PACKAGER, &type, (void **)&str, &count);
   if (!str)
     headerGetEntry(hdr, RPMTAG_VENDOR, &type, (void **)&str, &count);
   fprintf(f, _("Maintainer: %s\n"), str);
   
   headerGetEntry(hdr, RPMTAG_VERSION, &type, (void **)&str, &count);
   if (headerGetEntry(hdr, RPMTAG_EPOCH, &type, (void **)&numv, &count)==1)
       fprintf(f, _("Version: %i:%s"), numv[0], str);
   else
       fprintf(f, _("Version: %s"), str);

   headerGetEntry(hdr, RPMTAG_RELEASE, &type, (void **)&str, &count);
   fprintf(f, "-%s\n", str);

//   headerGetEntry(hdr, RPMTAG_DISTRIBUTION, &type, (void **)&str, &count);
//   fprintf(f, "Distribution: %s\n", str);

   headerGetEntry(hdr, RPMTAG_REQUIRENAME, &type, (void **)&strv, &count);
   headerGetEntry(hdr, RPMTAG_REQUIREVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(hdr, RPMTAG_REQUIREFLAGS, &type3, (void **)&numv, &count);
   if (count > 0) {
      char **dn, **dv;
      int *df;
      int i, j;

      dn = (char**)malloc(sizeof(char*)*count);
      dv = (char**)malloc(sizeof(char*)*count);
      df = (int*)malloc(sizeof(int)*count);
      
      if (!dn || !dv || !df) {
	 puts("could not malloc");
	 exit(1);
      }
      
      for (j = i = 0; i < count; i++) {
	 if ((numv[i] & RPMSENSE_PREREQ) && *strv[i]!='/') {//XXX
	    dn[j] = strv[i];
	    dv[j] = strv2[i];
	    df[j] = numv[i];
	    j++;
	 }
      }
      if (j > 0) {
	 fprintf(f, _("Pre-Depends: "));
	 pdepend(f, dn, dv, df, j, type);
      }
      
      for (j = i = 0; i < count; i++) {
	 if (!(numv[i] & RPMSENSE_PREREQ) && *strv[i]!='/') {//XXX
	    dn[j] = strv[i];
	    dv[j] = strv2[i];
	    df[j] = numv[i];
	    j++;
	 }
      }
      if (j > 0) {
	 fprintf(f, _("Depends: "));
	 pdepend(f, dn, dv, df, j, type);
      }

      free(dn);
      free(dv);
      free(df);
   }
   
   headerGetEntry(hdr, RPMTAG_CONFLICTNAME, &type, (void **)&strv, &count);
   headerGetEntry(hdr, RPMTAG_CONFLICTVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(hdr, RPMTAG_CONFLICTFLAGS, &type3, (void **)&numv, &count);
   if (count > 0) {
      fprintf(f, _("Conflicts: "));
      pdepend(f, strv, strv2, numv, count, type);
   }
   
   headerGetEntry(hdr, RPMTAG_PROVIDENAME, &type, (void **)&strv, &count);
   if (count > 0) {
      fprintf(f, _("Provides: "));
      pstr(f, strv, count, type);
   }

   headerGetEntry(hdr, RPMTAG_OBSOLETENAME, &type, (void **)&strv, &count);
   if (count > 0) {
      fprintf(f, _("Obsoletes: "));
      pstr(f, strv, count, type);
   }

   headerGetEntry(hdr, RPMTAG_ARCH, &type, (void **)&str, &count);
   fprintf(f, _("Architecture: %s\n"), str);
   
   headerGetEntry(hdr, CRPMTAG_FILESIZE, &type, (void **)&num, &count);
   fprintf(f, _("Size: %d\n"), num);

   if (headerGetEntry(hdr, CRPMTAG_MD5, &type, (void **)&str, &count)==1)
      printf(_("MD5sum: %s\n"), str);

   headerGetEntry(hdr, CRPMTAG_FILENAME, &type, (void **)&str, &count);
   fprintf(f, _("Filename: %s\n"), str);

   headerGetEntry(hdr, RPMTAG_SUMMARY, &type, (void **)&str, &count);
   fprintf(f, _("Description: %s\n"), str);
   
   headerGetEntry(hdr, RPMTAG_DESCRIPTION, &type, (void **)&str, &count);
   dumpDescription(f, str);

   fputc('\n', f);
}

bool DisplayRecord(pkgCache &Cache, pkgCache::VerIterator V)
{
   // Find an appropriate file
   pkgCache::VerFileIterator Vf = V.FileList();
   for (; Vf.end() == false; Vf++)
      if ((Vf.File()->Flags & pkgCache::Flag::NotSource) == 0)
	 break;
   if (Vf.end() == true)
      Vf = V.FileList();
      
   // Check and load the package list file
   pkgCache::PkgFileIterator I = Vf.File();
   if (I.IsOk() == false)
      return _error->Error(_("Package file %s is out of sync."),I.FileName());

   pkgRecords::Parser *Parser = _system->CreateRecordParser(I.FileName(), Cache);
   if (_error->PendingError() == true)
      return false;
   
   // Read the record and then write it out again.
   if (Parser->Jump(V.FileList()) == false)
      return false;

   Header h = ((rpmRecordParser*)Parser)->GetRecord();

   if (!h) {
      return false;
   }
   
   showHeader(stdout, h);
   

   return true;
}
#else
bool DisplayRecord(pkgCachepkgCache::VerIterator V)
{
   // Find an appropriate file
   pkgCache::VerFileIterator Vf = V.FileList();
   for (; Vf.end() == false; Vf++)
      if ((Vf.File()->Flags & pkgCache::Flag::NotSource) == 0)
	 break;
   if (Vf.end() == true)
      Vf = V.FileList();
      
   // Check and load the package list file
   pkgCache::PkgFileIterator I = Vf.File();
   if (I.IsOk() == false)
      return _error->Error(_("Package file %s is out of sync."),I.FileName());
   
   FileFd PkgF(I.FileName(),FileFd::ReadOnly);
   if (_error->PendingError() == true)
      return false;
   
   // Read the record and then write it out again.
   unsigned char *Buffer = new unsigned char[GCache->HeaderP->MaxVerFileSize];
   if (PkgF.Seek(V.FileList()->Offset) == false ||
       PkgF.Read(Buffer,V.FileList()->Size) == false ||
       write(STDOUT_FILENO,Buffer,V.FileList()->Size) != V.FileList()->Size)
   {
      delete [] Buffer;
      return false;
   }
   
   delete [] Buffer;

   return true;
}
#endif
									/*}}}*/
// Search - Perform a search						/*{{{*/
// ---------------------------------------------------------------------
/* This searches the package names and pacakge descriptions for a pattern */
bool Search(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   bool ShowFull = _config->FindB("APT::Cache::ShowFull",false);
   bool NamesOnly = _config->FindB("APT::Cache::NamesOnly",false);
   
   // Make sure there is at least one argument
   if (CmdL.FileSize() != 2)
      return _error->Error(_("You must give exactly one pattern"));

   // Compile the regex pattern
   regex_t Pattern;
   if (regcomp(&Pattern,CmdL.FileList[1],REG_EXTENDED | REG_ICASE | 
	       REG_NOSUB) != 0)
      return _error->Error(_("Regex compilation error"));

   // Create the text record parser
   pkgRecords Recs(Cache);
   if (_error->PendingError() == true)
      return false;
   
   // Search package names
   pkgCache::PkgIterator I = Cache.PkgBegin();
   for (;I.end() != true; I++)
   {
      // We search against the install version as that makes the most sense..
      pkgCache::VerIterator V = Cache.GetCandidateVer(I);
      if (V.end() == true)
	 continue;

      pkgRecords::Parser &P = Recs.Lookup(V.FileList());

      if (regexec(&Pattern,I.Name(),0,0,0) == 0 ||
	  (NamesOnly == false && 
	   regexec(&Pattern,P.LongDesc().c_str(),0,0,0) == 0))
      {
	 if (ShowFull == true)
	    DisplayRecord(Cache, V);
	 else
	    cout << I.Name() << " - " << P.ShortDesc() << endl;
      }
   }

   regfree(&Pattern);

   return true;
}
									/*}}}*/
// ShowPackage - Dump the package record to the screen			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowPackage(CommandLine &CmdL)
{   
   pkgCache &Cache = *GCache;
   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      pkgCache::PkgIterator Pkg = Cache.FindPkg(*I);
      if (Pkg.end() == true)
      {
	 _error->Warning(_("Unable to locate package %s"),*I);
	 continue;
      }
      
      // Find the proper version to use. We should probably use the DepCache.
      if (_config->FindB("APT::Cache::AllVersions","true") == true)
      {
	 pkgCache::VerIterator V;
	 for (V = Pkg.VersionList(); V.end() == false; V++)
	 {
	    if (DisplayRecord(Cache, V) == false)
	       return false;
	 }
      }
      else
      {
	 pkgCache::VerIterator V = Cache.GetCandidateVer(Pkg);
	 if (V.end() == true || V.FileList().end() == true)
	    continue;
	 if (DisplayRecord(Cache, V) == false)
	    return false;
      }      
   }
   return true;
}
									/*}}}*/
// ShowPkgNames - Show package names					/*{{{*/
// ---------------------------------------------------------------------
/* This does a prefix match on the first argument */
bool ShowPkgNames(CommandLine &CmdL)
{
   pkgCache &Cache = *GCache;
   pkgCache::PkgIterator I = Cache.PkgBegin();
   bool All = _config->FindB("APT::Cache::AllNames","false");
   
   if (CmdL.FileList[1] != 0)
   {
      for (;I.end() != true; I++)
      {
	 if (All == false && I->VersionList == 0)
	    continue;
	 
	 if (strncmp(I.Name(),CmdL.FileList[1],strlen(CmdL.FileList[1])) == 0)
	    cout << I.Name() << endl;
      }

      return true;
   }
   
   // Show all pkgs
   for (;I.end() != true; I++)
   {
      if (All == false && I->VersionList == 0)
	 continue;
      cout << I.Name() << endl;
   }
   
   return true;
}
									/*}}}*/
// ShowSrcPackage - Show source package records				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowSrcPackage(CommandLine &CmdL)
{
   pkgSourceList List;
   List.ReadMainList();
   
   // Create the text record parsers
   pkgSrcRecords SrcRecs(List);
   if (_error->PendingError() == true)
      return false;

   for (const char **I = CmdL.FileList + 1; *I != 0; I++)
   {
      SrcRecs.Restart();
      
      pkgSrcRecords::Parser *Parse;
      while ((Parse = SrcRecs.Find(*I,false)) != 0)
	 cout << Parse->AsStr();
   }      
   return true;
}
									/*}}}*/
// GenCaches - Call the main cache generator				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool GenCaches(CommandLine &Cmd)
{
   OpTextProgress Progress(*_config);
   
   pkgSourceList List;
   List.ReadMainList();
   return _system->makeStatusCache(List,Progress);
}
									/*}}}*/
// ShowHelp - Show a help screen					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ShowHelp(CommandLine &Cmd)
{
   cout << PACKAGE << ' ' << VERSION << " for " << COMMON_CPU <<
       " compiled on " << __DATE__ << "  " << __TIME__ << endl;
   if (_config->FindB("version") == true)
      return 100;
   
   cout << _("Usage: apt-cache [options] command") << endl;
   cout << _("       apt-cache [options] add file1 [file1 ...]") << endl;
   cout << _("       apt-cache [options] showpkg pkg1 [pkg2 ...]") << endl;
   cout << endl;
   cout << _("apt-cache is a low-level tool used to manipulate APT's binary") << endl;
   cout << _("cache files stored in ") << _config->FindFile("Dir::Cache") << endl;
   cout << _("It is not meant for ordinary use only as a debug aide.") << endl;
   cout << endl;
   cout << _("Commands:") << endl;
   cout << _("   add - Add an package file to the source cache") << endl;
   cout << _("   gencaches - Build both the package and source cache") << endl;
   cout << _("   showpkg - Show some general information for a single package") << endl;
   cout << _("   stats - Show some basic statistics") << endl;
   cout << _("   dump - Show the entire file in a terse form") << endl;
   cout << _("   dumpavail - Print an available file to stdout") << endl;
   cout << _("   unmet - Show unmet dependencies") << endl;
   cout << _("   check - Check the cache a bit") << endl;
   cout << _("   search - Search the package list for a regex pattern") << endl;
   cout << _("   show - Show a readable record for the package") << endl;
   cout << _("   depends - Show raw dependency information for a package") << endl;
   cout << _("   pkgnames - List the names of all packages") << endl;
   cout << _("   dotty - Generate package graphs for GraphVis") << endl;
   cout << endl;
   cout << _("Options:") << endl;
   cout << _("  -h   This help text.") << endl;
   cout << _("  -p=? The package cache. [") << _config->FindFile("Dir::Cache::pkgcache") << ']' << endl;
   cout << _("  -s=? The source cache. [") << _config->FindFile("Dir::Cache::srcpkgcache") << ']' << endl;
   cout << _("  -q   Disable progress indicator.") << endl;
   cout << _("  -i   Show only important deps for the unmet command.") << endl;
   cout << _("  -c=? Read this configuration file") << endl;
   cout << _("  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp") << endl;
   cout << _("See the apt-cache(8) and apt.conf(5) manual pages for more information.") << endl;
   return 100;
}
									/*}}}*/
// CacheInitialize - Initialize things for apt-cache			/*{{{*/
// ---------------------------------------------------------------------
/* */
void CacheInitialize()
{
   _config->Set("quiet",0);
   _config->Set("help",false);
}
									/*}}}*/

int main(int argc,const char *argv[])
{
   CommandLine::Args Args[] = {
      {'h',"help","help",0},
      {'v',"version","version",0},
      {'p',"pkg-cache","Dir::Cache::pkgcache",CommandLine::HasArg},
      {'s',"src-cache","Dir::Cache::srcpkgcache",CommandLine::HasArg},
      {'q',"quiet","quiet",CommandLine::IntLevel},
      {'i',"important","APT::Cache::Important",0},
      {'f',"full","APT::Cache::ShowFull",0},
      {'g',"no-generate","APT::Cache::NoGenerate",0},
      {'a',"all-versions","APT::Cache::AllVersions",0},
      {0,"names-only","APT::Cache::NamesOnly",0},
      {0,"all-names","APT::Cache::AllNames",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};
   CommandLine::Dispatch CmdsA[] = {{"help",&ShowHelp},
                                    {"add",&DoAdd},
                                    {"gencaches",&GenCaches},
                                    {"showsrc",&ShowSrcPackage},
                                    {0,0}};
   CommandLine::Dispatch CmdsB[] = {{"showpkg",&DumpPackage},
                                    {"stats",&Stats},
                                    {"dump",&Dump},
                                    {"dumpavail",&DumpAvail},
                                    {"unmet",&UnMet},
                                    {"check",&Check},
                                    {"search",&Search},
                                    {"depends",&Depends},
                                    {"dotty",&Dotty},
                                    {"show",&ShowPackage},
                                    {"pkgnames",&ShowPkgNames},
                                    {0,0}};
    
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);

   if (1) {
       RPMFactory *factory = new RPMFactory;
       
       pkgRpmLock *rpm = new pkgRpmLock(false);
       void *shutup_gcc = NULL;
       shutup_gcc = rpm;
       shutup_gcc = factory;
   } 
#if 0//akk
   else {
       DebianFactory *factory = new DebianFactory;
       void *shutup_gcc = NULL;
       shutup_gcc = factory;
   }
#endif
   CacheInitialize();
   
   // Parse the command line and initialize the package library
   CommandLine CmdL(Args,_config);
   if (pkgInitialize(*_config) == false ||
       CmdL.Parse(argc,argv) == false)
   {
      _error->DumpErrors();
      return 100;
   }

   // See if the help should be shown
   if (_config->FindB("help") == true ||
       CmdL.FileSize() == 0)
      return ShowHelp(CmdL);

   // Deal with stdout not being a tty
   if (ttyname(STDOUT_FILENO) == 0 && _config->FindI("quiet",0) < 1)
      _config->Set("quiet","1");

   if (CmdL.DispatchArg(CmdsA,false) == false && _error->PendingError() == false)
   { 
      MMap *Map;
      if (_config->FindB("APT::Cache::NoGenerate",false) == true)
      {
	 Map = new MMap(*new FileFd(_config->FindFile("Dir::Cache::pkgcache"),
				    FileFd::ReadOnly),MMap::Public|MMap::ReadOnly);
      }
      else
      {
	 // Open the cache file
	 pkgSourceList List;
	 List.ReadMainList();

	 // Generate it and map it
	 OpProgress Prog;
	 Map = _system->makeStatusCacheMem(List,Prog);
      }
      
      if (_error->PendingError() == false)
      {
	 pkgCache Cache(*Map);   
	 GCache = &Cache;
	 if (_error->PendingError() == false)
	    CmdL.DispatchArg(CmdsB);
      }
      delete Map;
   }
   
   // Print any errors or warnings found during parsing
   if (_error->empty() == false)
   {
      bool Errors = _error->PendingError();
      _error->DumpErrors();
      return Errors == true?100:0;
   }
          
   return 0;
}
