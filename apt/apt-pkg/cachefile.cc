// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: cachefile.cc,v 1.8 2001/07/12 21:47:32 kojima Exp $
/* ######################################################################
   
   CacheFile - Simple wrapper class for opening, generating and whatnot
   
   This class implements a simple 2 line mechanism to open various sorts
   of caches. It can operate as root, as not root, show progress and so on,
   it transparently handles everything necessary.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/cachefile.h"
#endif


#include <apt-pkg/cachefile.h>
#include <apt-pkg/error.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/systemfactory.h>

#include <i18n.h>

									/*}}}*/

// CacheFile::CacheFile - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCacheFile::pkgCacheFile() : Map(0), Cache(0), 
#if 0//akk
Lock(0),
#endif
RPM(0)
{
}
									/*}}}*/
// CacheFile::~CacheFile - Destructor						/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgCacheFile::~pkgCacheFile()
{
   delete Cache;
   delete Map;
#if 0//akk
   if (Lock)
	delete Lock;
#endif
   if (RPM)
	delete RPM;
}   
									/*}}}*/
// CacheFile::Open - Open the cache files, creating if necessary	/*{{{*/
// ---------------------------------------------------------------------
/* */
bool pkgCacheFile::Open(OpProgress &Progress,bool WithLock)
{
   if (WithLock == true) 
    {
#if 0 //akk
       if (0)//akk
       {
	  Lock = new pkgDpkgLock;
       } else 
       {
	  
       }
#endif
   }
   if (1) {
      RPM = new pkgRpmLock(WithLock);
   }
   
   if (_error->PendingError() == true)
      return false;
   
   // Read the source list
   pkgSourceList List;
   if (List.ReadMainList() == false)
      return _error->Error(_("The list of sources could not be read."));
   
   /* Build all of the caches, using the cache files if we are locking 
      (ie as root) */
   if (WithLock == true)
   {
      _system->makeStatusCache(List, Progress);

      Progress.Done();
      if (_error->PendingError() == true)
	 return _error->Error(_("The package lists or status file could not be parsed or opened."));
      if (_error->empty() == false)
	 _error->Warning(_("You may want to run apt-get update to correct these missing files"));
      
      // Open the cache file
      FileFd File(_config->FindFile("Dir::Cache::pkgcache"),FileFd::ReadOnly);
      if (_error->PendingError() == true)
	 return false;
      
      Map = new MMap(File,MMap::Public | MMap::ReadOnly);
      if (_error->PendingError() == true)
	 return false;
   }
   else
   {
      Map = _system->makeStatusCacheMem(List,Progress);
      Progress.Done();
      if (Map == 0)
	 return false;
   }
   
   // Create the dependency cache
   Cache = new pkgDepCache(*Map,Progress);
   Progress.Done();
   if (_error->PendingError() == true)
      return false;
   
   return true;
}
									/*}}}*/
