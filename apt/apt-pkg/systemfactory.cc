


#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/systemfactory.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/progress.h>
#include <assert.h>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <system.h>

#include <i18n.h>


SystemFactory *_system = NULL;



SystemFactory::SystemFactory()
{
    assert(_system == NULL);
    _system = this;
}


pkgPackageManager *SystemFactory::CreatePackageManager(pkgDepCache &Cache)
{
    return NULL;
}


// SrcCacheCheck - Check if the source package cache is uptodate	/*{{{*/
// ---------------------------------------------------------------------
/* The source cache is checked against the source list and the files 
 on disk, any difference results in a false. */
bool SystemFactory::sourceCacheCheck(pkgSourceList &List)
{
    if (_error->PendingError() == true) {
	return false;
    }
    
    string CacheFile = _config->FindFile("Dir::Cache::srcpkgcache");
    string ListDir = _config->FindDir("Dir::State::lists");
    
    // Count the number of missing files
    int Missing = 0;
    for (pkgSourceList::const_iterator I = List.begin(); I != List.end(); I++)
    {
	// Only cache our source types.
	if (!checkSourceType(I->Type()))
	{
	    Missing++;
	    continue;
	}
	
	string File = ListDir + URItoFileName(I->PackagesURI());
	struct stat Buf;
	if (stat(File.c_str(),&Buf) != 0)
	{
	    // Old format file name.. rename it
	    if (File[0] == '_' && stat(File.c_str()+1,&Buf) == 0)
	    {
		if (rename(File.c_str()+1,File.c_str()) != 0)
		    return _error->Errno("rename",_("Failed to rename %s to %s"),
					 File.c_str()+1,File.c_str());
		continue;
	    }	 
	    
	    _error->WarningE("stat",_("Couldn't stat source package list '%s' (%s)"),
			     I->PackagesInfo().c_str(),File.c_str());	 
	    Missing++;
	}      
    }
    
    // Open the source package cache
    if (FileExists(CacheFile) == false)
	return false;
    
    FileFd CacheF(CacheFile,FileFd::ReadOnly);
    if (_error->PendingError() == true)
    {
	_error->Discard();
	return false;
    }

    MMap Map(CacheF,MMap::Public | MMap::ReadOnly);
    if (_error->PendingError() == true || Map.Size() == 0)
    {
	_error->Discard();
	return false;
    }
    
    pkgCache Cache(Map);
    if (_error->PendingError() == true)
    {
	_error->Discard();
	return false;
    }
    
    // They are certianly out of sync
    if (Cache.Head().PackageFileCount != List.size() - Missing)
	return false;

    for (pkgCache::PkgFileIterator F(Cache); F.end() == false; F++)
    {
	// Search for a match in the source list
	bool Bad = true;
	for (pkgSourceList::const_iterator I = List.begin(); 
	     I != List.end(); I++)
	{
	    // Only cache deb source types.
	    if (!checkSourceType(I->Type()))
		continue;
	    
	    string File = ListDir + URItoFileName(I->PackagesURI());
	    if (F.FileName() == File)
	    {
		Bad = false;
		break;
	    }
	}
	
	// Check if the file matches what was cached
	Bad |= !F.IsOk();
	if (Bad == true)
	    return false;
    }
    
    return true;
}

/*}}}*/
// GenerateSrcCache - Write the source package lists to the map		/*{{{*/
// ---------------------------------------------------------------------
/* This puts the source package cache into the given generator. */
bool SystemFactory::generateSrcCache(pkgSourceList &List,OpProgress &Progress,
				     pkgCacheGenerator &Gen,
				     unsigned long &CurrentSize,unsigned long &TotalSize)
{
    string ListDir = _config->FindDir("Dir::State::lists");

    // Prepare the progress indicator
    TotalSize = 0;
    struct stat Buf;
    for (pkgSourceList::const_iterator I = List.begin(); I != List.end(); I++)
    {
	string File = ListDir + URItoFileName(I->PackagesURI());
	if (stat(File.c_str(),&Buf) != 0)
	    continue;
	TotalSize += Buf.st_size;
    }

    if (addStatusSize(TotalSize) == false)
	return false;
    
    // Generate the pkg source cache
    CurrentSize = 0;
    for (pkgSourceList::const_iterator I = List.begin(); I != List.end(); I++)
    {
	pkgCacheGenerator::ListParser *Parser;

	// Only cache relevant source types.
	if (!checkSourceType(I->Type()))
	    continue;

	string File = ListDir + URItoFileName(I->PackagesURI());
	
	if (FileExists(File) == false)
	    continue;

	FileFd Pkg(File,FileFd::ReadOnly);

	Parser = CreateListParser(Pkg);

	Progress.OverallProgress(CurrentSize,TotalSize,Pkg.Size(),_("Reading Package Lists"));

	if (_error->PendingError() == true) {
	    delete Parser;
	    return _error->Error(_("Problem opening %s"),File.c_str());
	}

	CurrentSize += Pkg.Size();

	Progress.SubProgress(0,I->PackagesInfo());
	if (Gen.SelectFile(File) == false) {
	    delete Parser;
	    return _error->Error(_("Problem with SelectFile %s"),File.c_str());
	}

	if (Gen.MergeList(*Parser) == false) {
	    delete Parser;
	    return _error->Error(_("Problem with MergeList %s"),File.c_str());
	}
	
	// Check the release file
	string RFile = ListDir + URItoFileName(I->ReleaseURI());
	if (FileExists(RFile) == true)
	{
	    FileFd Rel(RFile,FileFd::ReadOnly);
	    if (_error->PendingError() == true) {
		return false;
	    }
	    Parser->LoadReleaseInfo(Gen.GetCurFile(),Rel);
	}
    }
    
    return true;
}



// MakeStatusCache - Generates a cache that includes the status files	/*{{{*/
// ---------------------------------------------------------------------
/* This copies the package source cache and then merges the status and 
 xstatus files into it. */
bool SystemFactory::makeStatusCache(pkgSourceList &List,OpProgress &Progress)
{
    unsigned long MapSize = _config->FindI("APT::Cache-Limit",4*1024*1024);   
        
    string CacheFile = _config->FindFile("Dir::Cache::pkgcache");
    bool SrcOk = sourceCacheCheck(List);

    bool PkgOk = SrcOk && packageCacheCheck(CacheFile);
        
    // Rebuild the source and package caches   
    if (SrcOk == false)
    {
	if (!preProcess(List, Progress)) 
	    return false;
	
	Progress.OverallProgress(0,1,1,_("Reading Package Lists"));
	
	string SCacheFile = _config->FindFile("Dir::Cache::srcpkgcache");
	FileFd SCacheF(SCacheFile,FileFd::WriteEmpty);

	/* Open the pkgcache, we want a new inode here so we do no corrupt
	 existing mmaps */
	unlink(CacheFile.c_str());             
	FileFd CacheF(CacheFile,FileFd::WriteEmpty);
	DynamicMMap Map(CacheF,MMap::Public,MapSize);
	if (_error->PendingError() == true)
	    return false;

	pkgCacheGenerator Gen(Map,Progress);
	unsigned long CurrentSize = 0;
	unsigned long TotalSize = 0;
	
	if (generateSrcCache(List,Progress,Gen,CurrentSize,TotalSize) == false)
	    return false;

	// Write the src cache
	Gen.GetCache().HeaderP->Dirty = false;
	if (SCacheF.Write(Map.Data(),Map.Size()) == false)
	    return _error->Error(_("IO Error saving source cache"));
	Gen.GetCache().HeaderP->Dirty = true;

	// Merge in the source caches
	return mergeInstalledPackages(Progress,Gen,CurrentSize,TotalSize);
    }

    if (PkgOk == true)
    {
	Progress.OverallProgress(0,1,1,_("Reading Package Lists"));
	Progress.OverallProgress(1,1,1,_("Reading Package Lists"));      
	return true;
    }
    else 
    {
	if (!preProcess(List, Progress)) 
	    return false;
	Progress.OverallProgress(0,1,1,_("Reading Package Lists"));
    }

    // We use the source cache to generate the package cache
    string SCacheFile = _config->FindFile("Dir::Cache::srcpkgcache");
    FileFd SCacheF(SCacheFile,FileFd::ReadOnly);
    
    /* Open the pkgcache, we want a new inode here so we do no corrupt
     existing mmaps */
    unlink(CacheFile.c_str());             
    FileFd CacheF(CacheFile,FileFd::WriteEmpty);
    DynamicMMap Map(CacheF,MMap::Public,MapSize);
    if (_error->PendingError() == true)
	return false;
    
    // Preload the map with the source cache
    if (SCacheF.Read((unsigned char *)Map.Data() + Map.RawAllocate(SCacheF.Size()),
		     SCacheF.Size()) == false)
	return false;

    pkgCacheGenerator Gen(Map,Progress);
    
    // Compute the progress
    unsigned long TotalSize = 0;
    if (addStatusSize(TotalSize) == false)
	return false;

    unsigned long CurrentSize = 0;
    return mergeInstalledPackages(Progress,Gen,CurrentSize,TotalSize);
}
/*}}}*/
// MakeStatusCacheMem - Returns a map for the status cache		/*{{{*/
// ---------------------------------------------------------------------
/* This creates a map object for the status cache. If the process has write
 access to the caches then it is the same as MakeStatusCache, otherwise it
 creates a memory block and puts the cache in there. */
MMap *SystemFactory::makeStatusCacheMem(pkgSourceList &List,OpProgress &Progress)
{
    unsigned long MapSize = _config->FindI("APT::Cache-Limit",4*1024*1024);

    if (!preProcess(List, Progress)) {
	return 0;
    }

    /* If the cache file is writeable this is just a wrapper for
     MakeStatusCache */
    string CacheFile = _config->FindFile("Dir::Cache::pkgcache");
    bool Writeable = (access(CacheFile.c_str(),W_OK) == 0) ||
	(errno == ENOENT);
    
    if (Writeable == true)
    {
	if (makeStatusCache(List,Progress) == false)
	    return 0;
	
	// Open the cache file
	FileFd File(_config->FindFile("Dir::Cache::pkgcache"),FileFd::ReadOnly);
	if (_error->PendingError() == true)
	    return 0;
	
	MMap *Map = new MMap(File,MMap::Public | MMap::ReadOnly);
	if (_error->PendingError() == true)
	{
	    delete Map;
	    return 0;
	}
	return Map;
    }      
    
    // Mostly from MakeStatusCache..
    Progress.OverallProgress(0,1,1,_("Reading Package Lists"));
    
    bool SrcOk = sourceCacheCheck(List);
    bool PkgOk = SrcOk && packageCacheCheck(CacheFile);
    
    // Rebuild the source and package caches   
    if (SrcOk == false)
    {
	DynamicMMap *Map = new DynamicMMap(MMap::Public,MapSize);
	if (_error->PendingError() == true)
	{
	    delete Map;
	    return 0;
	}
	
	pkgCacheGenerator Gen(*Map,Progress);
	unsigned long CurrentSize = 0;
	unsigned long TotalSize = 0;
	if (generateSrcCache(List,Progress,Gen,CurrentSize,TotalSize) == false)
	{
	    delete Map;
	    return 0;
	}
	
	// Merge in the source caches
	if (mergeInstalledPackages(Progress,Gen,CurrentSize,TotalSize) == false)
	{
	    delete Map;
	    return 0;
	}
	
	return Map;
    }
    
    if (PkgOk == true)
    {
	Progress.OverallProgress(1,1,1,_("Reading Package Lists"));
	
	// Open the cache file
	FileFd File(_config->FindFile("Dir::Cache::pkgcache"),FileFd::ReadOnly);
	if (_error->PendingError() == true)
	    return 0;
	
	MMap *Map = new MMap(File,MMap::Public | MMap::ReadOnly);
	if (_error->PendingError() == true)
	{
	    delete Map;
	    return 0;
	}
	return Map;
    }
    
    // We use the source cache to generate the package cache
    string SCacheFile = _config->FindFile("Dir::Cache::srcpkgcache");
    FileFd SCacheF(SCacheFile,FileFd::ReadOnly);
    DynamicMMap *Map = new DynamicMMap(MMap::Public,MapSize);
    if (_error->PendingError() == true)
    {
	delete Map;
	return 0;
    }
    
    // Preload the map with the source cache
    if (SCacheF.Read((unsigned char *)Map->Data() + Map->RawAllocate(SCacheF.Size()),
		     SCacheF.Size()) == false)
    {
	delete Map;
	return 0;
    }
    
    pkgCacheGenerator Gen(*Map,Progress);
    
    // Compute the progress
    unsigned long TotalSize = 0;
    if (addStatusSize(TotalSize) == false)
    {
	delete Map;
	return 0;
    }
    
    unsigned long CurrentSize = 0;
    if (mergeInstalledPackages(Progress,Gen,CurrentSize,TotalSize) == false)
    {
	delete Map;
	return 0;
    }
    
    return Map;
}
/*}}}*/

