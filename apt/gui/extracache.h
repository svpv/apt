// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: extracache.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   ExtraCache - Extension data for the cache
   
   Two name sorted lists of packages are maintained. One is sorted purely
   by name and the other is sorted by section and then by name. This
   class also provides Originally pkgDepCache used to be in here, but it 
   has migrated into pkglib.

   ##################################################################### */
									/*}}}*/
#ifndef EXTRACACHE_H
#define EXTRACACHE_H

#include <apt-pkg/depcache.h>

class Policy;
class ExtraCache : public pkgDepCache
{
   // Helper functions
   static ExtraCache *SortCache;
   static int NameComp(const void *a,const void *b);
   static int SecNameComp(const void *a,const void *b);
   bool Init();
   
   protected:

   // Policy implementation for DepCache
   virtual VerIterator GetCandidateVer(PkgIterator Pkg);
   virtual bool IsImportantDep(DepIterator Dep);
   
   public:
      
   // Name sorted lists of packages
   Package **SortedPkgs;
   Package **SecSortedPkgs;

   Policy &CurPolicy;
   inline bool PromoteAutoKeep() {return true;};
   
   ExtraCache(Policy &Pol,MMap &Map,OpProgress &Prog);
   virtual ~ExtraCache();
};

class ExtraCacheF
{
   FileFd *File;
   MMap *Map;
   ExtraCache *Cache;
   public:
   
   ExtraCache &operator *() {return *Cache;};
   ExtraCache &operator ->() {return *Cache;};
   
   ExtraCacheF(Policy &Pol,OpProgress &Prog);
   ~ExtraCacheF();
};

#endif
