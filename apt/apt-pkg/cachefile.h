// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: cachefile.h,v 1.4 2001/07/12 21:47:32 kojima Exp $
/* ######################################################################
   
   CacheFile - Simple wrapper class for opening, generating and whatnot
   
   This class implements a simple 2 line mechanism to open various sorts
   of caches. It can operate as root, as not root, show progress and so on,
   it transparently handles everything necessary.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_CACHEFILE_H
#define PKGLIB_CACHEFILE_H

#ifdef __GNUG__
#pragma interface "apt-pkg/cachefile.h"
#endif 


#include <apt-pkg/depcache.h>
#include <apt-pkg/dpkginit.h>
#include <apt-pkg/rpminit.h>

class pkgCacheFile
{
   protected:
   
   MMap *Map;
   pkgDepCache *Cache;
#if 0//akk
   pkgDpkgLock *Lock;
#endif
   pkgRpmLock *RPM;
   
   public:
      
   // We look pretty much exactly like a pointer to a dep cache
   inline operator pkgDepCache &() {return *Cache;};
   inline operator pkgDepCache *() {return Cache;};
   inline pkgDepCache *operator ->() {return Cache;};
   inline pkgDepCache &operator *() {return *Cache;};
   inline pkgDepCache::StateCache &operator [](pkgCache::PkgIterator const &I) {return (*Cache)[I];};
   inline unsigned char &operator [](pkgCache::DepIterator const &I) {return (*Cache)[I];};

   // Release the dpkg status lock
   inline void ReleaseLock() {
#if 0//akk
      if (0)
	Lock->Close(); 
      else
#endif
	RPM->Close();
   };//akk
   
   bool Open(OpProgress &Progress,bool WithLock = true);
   
   pkgCacheFile();
   ~pkgCacheFile();
};

#endif
