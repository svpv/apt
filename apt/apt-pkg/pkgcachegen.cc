// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgcachegen.cc,v 1.24 2001/12/11 20:50:12 kojima Exp $
/* ######################################################################
   
   Package Cache Generator - Generator for the cache structure.
   
   This builds the cache structure from the abstract package list parser. 
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/pkgcachegen.h"
#endif

#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/error.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/strutl.h>

#include <apt-pkg/systemfactory.h>

#include <i18n.h>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <system.h>
									/*}}}*/
// CacheGenerator::pkgCacheGenerator - Constructor			/*{{{*/
// ---------------------------------------------------------------------
/* We set the diry flag and make sure that is written to the disk */
pkgCacheGenerator::pkgCacheGenerator(DynamicMMap &Map,OpProgress &Prog) : Map(Map), Cache(Map), Progress(&Prog)
{
    CurrentFile = 0;
    
    if (_error->PendingError() == true)
	return;
    
    if (Map.Size() == 0)
    {
	Map.RawAllocate(sizeof(pkgCache::Header));
	*Cache.HeaderP = pkgCache::Header();
    }
    Cache.HeaderP->Dirty = true;
    Map.Sync(0,sizeof(pkgCache::Header));
    Map.UsePools(*Cache.HeaderP->Pools,sizeof(Cache.HeaderP->Pools)/sizeof(Cache.HeaderP->Pools[0]));
    memset(UniqHash,0,sizeof(UniqHash));
}
/*}}}*/
// CacheGenerator::~pkgCacheGenerator - Destructor 			/*{{{*/
// ---------------------------------------------------------------------
/* We sync the data then unset the dirty flag in two steps so as to
 advoid a problem during a crash */
pkgCacheGenerator::~pkgCacheGenerator()
{
    if (_error->PendingError() == true)
	return;
    if (Map.Sync() == false)
	return;
    
    Cache.HeaderP->Dirty = false;
    Map.Sync(0,sizeof(pkgCache::Header));
}
                                                                        /*}}}*/
// CacheGenerator::MergeList - Merge the package list			/*{{{*/
// ---------------------------------------------------------------------
/* This provides the generation of the entries in the cache. Each loop
 goes through a single package record from the underlying parse engine. */
bool pkgCacheGenerator::MergeList(ListParser &List,
				  pkgCache::VerIterator *OutVer)
{
    List.Owner = this;
    
    unsigned int Counter = 0;
    while (List.Step() == true)
    {
	// Get a pointer to the package structure
	string PackageName = List.Package();

	if (PackageName.empty() == true)
	    return false;
	
	pkgCache::PkgIterator Pkg;
	if (NewPackage(Pkg,PackageName) == false)
	    return _error->Error(_("Error occured while processing %s (NewPackage)"),PackageName.c_str());
	Counter++;
	if (Counter % 100 == 0 && Progress != 0)
	    Progress->Progress(List.Offset());
	
	/* Get a pointer to the version structure. We know the list is sorted
         so we use that fact in the search. Insertion of new versions is
	 done with correct sorting */
	string Version = List.Version();
	if (Version.empty() == true)
	{
	    if (List.UsePackage(Pkg,pkgCache::VerIterator(Cache)) == false)
		return _error->Error(_("Error occured while processing %s (UsePackage1)"),PackageName.c_str());
	    continue;
	}
	
	pkgCache::VerIterator Ver = Pkg.VersionList();
	map_ptrloc *Last = &Pkg->VersionList;
	int Res = 1;
	for (; Ver.end() == false; Last = &Ver->NextVer, Ver++)
	{
	    Res = _system->versionCompare(Version.begin(),Version.end(),Ver.VerStr(),
				    Ver.VerStr() + strlen(Ver.VerStr()));
	    if (Res >= 0) {
		break;
	    }
	}
	
	/* We already have a version for this item, record that we saw it */
	unsigned long Hash = List.VersionHash();
	bool ToEnd = true;

	if (Res == 0 && Ver->Hash == Hash)
	{
	    if (List.UsePackage(Pkg,Ver) == false)
		return _error->Error(_("Error occured while processing %s (UsePackage2)"),PackageName.c_str());
	    
	    if (NewFileVer(Ver,List) == false)
		return _error->Error(_("Error occured while processing %s (NewFileVer1)"),PackageName.c_str());
	    
	    // Read only a single record and return
	    if (OutVer != 0)
	    {
		*OutVer = Ver;
		return true;
	    }
	    continue;
	} else if (Res == 0) {
	   cout << _("WARNING: '") << PackageName << _("' has 2 packages with same version but different dependencies. ");
	   cout << _("That usually means a packaging bug.") << endl;
	    
	   if (Pkg.CurrentVer() == Ver) { 
	        // if this is the currently installed version, then
		// keep it as the 1st in the same version set
		ToEnd = false;
	   } else {
	        // if this one is not the currently installed version,
		// just ignore it. and hope that this package doesn't
		// have an extra dependency that would fix some
		// situation... which would be severe a packaging bug anyway
//	        cout << "Skipping package"<<endl;
//	        continue;
//	        apparently have some problems yet, got a case where
//	        an isntalled pkg was undetected
	   }
	}
	// Skip to the end of the same version set
	if (Res == 0 && ToEnd == true)
	{
	    for (; Ver.end() == false; Last = &Ver->NextVer, Ver++)
	    {
		Res = _system->versionCompare(Version.begin(),Version.end(),Ver.VerStr(),
					Ver.VerStr() + strlen(Ver.VerStr()));
		if (Res != 0)
		    break;
	    }
	}
	
	// Add a new version
	*Last = NewVersion(Ver,Version,*Last);
	Ver->ParentPkg = Pkg.Index();
	Ver->Hash = Hash;
	if (List.NewVersion(Ver) == false)
	    return _error->Error(_("Error occured while processing %s (NewVersion1)"),PackageName.c_str());
	
	if (List.UsePackage(Pkg,Ver) == false)
	    return _error->Error(_("Error occured while processing %s (UsePackage3)"),PackageName.c_str());
	
	if (NewFileVer(Ver,List) == false)
	    return _error->Error(_("Error occured while processing %s (NewVersion2)"),PackageName.c_str());
	
	// Read only a single record and return
	if (OutVer != 0)
	{
	    *OutVer = Ver;
	    return true;
	}      
    }
    
    return true;
}
                                                                        /*}}}*/
// CacheGenerator::NewPackage - Add a new package			/*{{{*/
// ---------------------------------------------------------------------
/* This creates a new package structure and adds it to the hash table */
bool pkgCacheGenerator::NewPackage(pkgCache::PkgIterator &Pkg,string Name)
{
    Pkg = Cache.FindPkg(Name);
    if (Pkg.end() == false)
	return true;

    // Get a structure
    unsigned long Package = Map.Allocate(sizeof(pkgCache::Package));
    if (Package == 0)
	return false;
    
    Pkg = pkgCache::PkgIterator(Cache,Cache.PkgP + Package);
    
    // Insert it into the hash table
    unsigned long Hash = Cache.Hash(Name);
    Pkg->NextPackage = Cache.HeaderP->HashTable[Hash];
    Cache.HeaderP->HashTable[Hash] = Package;
    
    // Set the name and the ID
    Pkg->Name = Map.WriteString(Name);
    if (Pkg->Name == 0)
	return false;
    Pkg->ID = Cache.HeaderP->PackageCount++;
    
    return true;
}
/*}}}*/
// CacheGenerator::NewFileVer - Create a new File<->Version association	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgCacheGenerator::NewFileVer(pkgCache::VerIterator &Ver,
				   ListParser &List)
{
    if (CurrentFile == 0)
	return true;
    
    // Get a structure
    unsigned long VerFile = Map.Allocate(sizeof(pkgCache::VerFile));
    if (VerFile == 0)
	return false;
    
    pkgCache::VerFileIterator VF(Cache,Cache.VerFileP + VerFile);
    VF->File = CurrentFile - Cache.PkgFileP;
    
    // Link it to the end of the list
    map_ptrloc *Last = &Ver->FileList;
    for (pkgCache::VerFileIterator V = Ver.FileList(); V.end() == false; V++)
	Last = &V->NextFile;
    VF->NextFile = *Last;
    *Last = VF.Index();
    
    VF->Offset = List.Offset();
    VF->Size = List.Size();
    if (Cache.HeaderP->MaxVerFileSize < VF->Size)
	Cache.HeaderP->MaxVerFileSize = VF->Size;
    Cache.HeaderP->VerFileCount++;
    
    return true;
}
/*}}}*/
// CacheGenerator::NewVersion - Create a new Version 			/*{{{*/
// ---------------------------------------------------------------------
/* This puts a version structure in the linked list */
unsigned long pkgCacheGenerator::NewVersion(pkgCache::VerIterator &Ver,
					    string VerStr,
					    unsigned long Next)
{
    // Get a structure
    unsigned long Version = Map.Allocate(sizeof(pkgCache::Version));
    if (Version == 0)
	return 0;

    // Fill it in
    Ver = pkgCache::VerIterator(Cache,Cache.VerP + Version);
    Ver->NextVer = Next;
    Ver->ID = Cache.HeaderP->VersionCount++;
    Ver->VerStr = Map.WriteString(VerStr);
    if (Ver->VerStr == 0)
	return 0;
    
    return Version;
}
/*}}}*/
// ListParser::NewDepends - Create a dependency element			/*{{{*/
 // ---------------------------------------------------------------------
/* This creates a dependency element in the tree. It is linked to the
 version and to the package that it is pointing to. */
bool pkgCacheGenerator::ListParser::NewDepends(pkgCache::VerIterator Ver,
					       string PackageName,
					       string Version,
					       unsigned int Op,
					       unsigned int Type)
{
    pkgCache &Cache = Owner->Cache;

    // Get a structure
    unsigned long Dependency = Owner->Map.Allocate(sizeof(pkgCache::Dependency));
    if (Dependency == 0)
	return false;
        
    // Fill it in
    pkgCache::DepIterator Dep(Cache,Cache.DepP + Dependency);
    Dep->ParentVer = Ver.Index();
    Dep->Type = Type;
    Dep->CompareOp = Op;
    Dep->ID = Cache.HeaderP->DependsCount++;
    
    // Locate the target package
    pkgCache::PkgIterator Pkg;
    if (Owner->NewPackage(Pkg,PackageName) == false)
	return false;
    
    // Probe the reverse dependency list for a version string that matches
    if (Version.empty() == false)
    {
	/*      for (pkgCache::DepIterator I = Pkg.RevDependsList(); I.end() == false; I++, Hit++)
	 if (I->Version != 0 && I.TargetVer() == Version)
	 Dep->Version = I->Version;*/
	if (Dep->Version == 0)
	    if ((Dep->Version = WriteString(Version)) == 0)
		return false;
    }
    
    // Link it to the package
    Dep->Package = Pkg.Index();
    Dep->NextRevDepends = Pkg->RevDepends;
    Pkg->RevDepends = Dep.Index();
    
    /* Link it to the version (at the end of the list)
     Caching the old end point speeds up generation substantially */
    if (OldDepVer != Ver)
    {
	OldDepLast = &Ver->DependsList;
	for (pkgCache::DepIterator D = Ver.DependsList(); D.end() == false; D++)
	    OldDepLast = &D->NextDepends;
	OldDepVer = Ver;
    }
    
        
    Dep->NextDepends = *OldDepLast;
    *OldDepLast = Dep.Index();
    OldDepLast = &Dep->NextDepends;
    
    return true;
}
/*}}}*/
// ListParser::NewProvides - Create a Provides element			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgCacheGenerator::ListParser::NewProvides(pkgCache::VerIterator Ver,
					        string PackageName,
						string Version)
{
    pkgCache &Cache = Owner->Cache;
    
    // We do not add self referencing provides
    if (Ver.ParentPkg().Name() == PackageName)
	return true;
    
    // Get a structure
    unsigned long Provides = Owner->Map.Allocate(sizeof(pkgCache::Provides));
    if (Provides == 0)
	return false;
    Cache.HeaderP->ProvidesCount++;
    
    // Fill it in
    pkgCache::PrvIterator Prv(Cache,Cache.ProvideP + Provides,Cache.PkgP);
    Prv->Version = Ver.Index();
    Prv->NextPkgProv = Ver->ProvidesList;
    Ver->ProvidesList = Prv.Index();
    
    if (Version.empty() == false)
    {
	if (Prv->ProvideVersion == 0)
	    if ((Prv->ProvideVersion = WriteString(Version)) == 0)
		return false;
    }
    
    // Locate the target package
    pkgCache::PkgIterator Pkg;
    if (Owner->NewPackage(Pkg,PackageName) == false)
	return false;
    
    // Link it to the package
    Prv->ParentPkg = Pkg.Index();
    Prv->NextProvides = Pkg->ProvidesList;
    Pkg->ProvidesList = Prv.Index();

    return true;
}
/*}}}*/
// CacheGenerator::SelectFile - Select the current file being parsed	/*{{{*/
// ---------------------------------------------------------------------
/* This is used to select which file is to be associated with all newly
 added versions. */
bool pkgCacheGenerator::SelectFile(string File,unsigned long Flags)
{
    struct stat Buf;
    if (stat(File.c_str(),&Buf) == -1)
	return _error->Errno("stat","Couldn't stat %s",File.c_str());
    
    // Get some space for the structure
    CurrentFile = Cache.PkgFileP + Map.Allocate(sizeof(*CurrentFile));
    if (CurrentFile == Cache.PkgFileP)
	return false;
    
    // Fill it in
    CurrentFile->FileName = Map.WriteString(File);
    CurrentFile->Size = Buf.st_size;
    CurrentFile->mtime = Buf.st_mtime;
    CurrentFile->NextFile = Cache.HeaderP->FileList;
    CurrentFile->Flags = Flags;
    CurrentFile->ID = Cache.HeaderP->PackageFileCount;
    PkgFileName = File;
    Cache.HeaderP->FileList = CurrentFile - Cache.PkgFileP;
    Cache.HeaderP->PackageFileCount++;
    
    if (CurrentFile->FileName == 0)
	return false;
    
    if (Progress != 0)
	Progress->SubProgress(Buf.st_size);
    return true;
}
/*}}}*/
// CacheGenerator::WriteUniqueString - Insert a unique string		/*{{{*/
// ---------------------------------------------------------------------
/* This is used to create handles to strings. Given the same text it
 always returns the same number */
unsigned long pkgCacheGenerator::WriteUniqString(const char *S,
						 unsigned int Size)
{
    /* We use a very small transient hash table here, this speeds up generation
     by a fair amount on slower machines */
    pkgCache::StringItem *&Bucket = UniqHash[(S[0]*5 + S[1]) % _count(UniqHash)];
    if (Bucket != 0 && 
	stringcmp(S,S+Size,Cache.StrP + Bucket->String) == 0)
	return Bucket->String;
    
    // Search for an insertion point
    pkgCache::StringItem *I = Cache.StringItemP + Cache.HeaderP->StringList;
    int Res = 1;
    map_ptrloc *Last = &Cache.HeaderP->StringList;
    for (; I != Cache.StringItemP; Last = &I->NextItem, 
	 I = Cache.StringItemP + I->NextItem)
    {
	Res = stringcmp(S,S+Size,Cache.StrP + I->String);
	if (Res >= 0)
	    break;
    }
    
    // Match
    if (Res == 0)
    {
	Bucket = I;
	return I->String;
    }
    
    // Get a structure
    unsigned long Item = Map.Allocate(sizeof(pkgCache::StringItem));
    if (Item == 0)
	return 0;
    
    // Fill in the structure
    pkgCache::StringItem *ItemP = Cache.StringItemP + Item;
    ItemP->NextItem = I - Cache.StringItemP;
    *Last = Item;
    ItemP->String = Map.WriteString(S,Size);
    if (ItemP->String == 0)
	return 0;
    
    Bucket = ItemP;
    return ItemP->String;
}
