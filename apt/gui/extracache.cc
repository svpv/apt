// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: extracache.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   ExtraCache - Extension data for the cache
   
   This class stores the cache data and a set of extension structures for
   monitoring the current state of all the packages.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include "extracache.h"
#include "policy.h"

#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
									/*}}}*/

ExtraCache *ExtraCache::SortCache = 0;

// ExtraCache::ExtraCache - Constructors				/*{{{*/
// ---------------------------------------------------------------------
/* */
ExtraCache::ExtraCache(Policy &Pol,MMap &Map,OpProgress &Prog) : 
             pkgDepCache(Map,Prog), SortedPkgs(0), SecSortedPkgs(0), CurPolicy(Pol)
{
   if (_error->PendingError() == false)
      Init();
}
									/*}}}*/
// ExtraCache::~ExtraCache - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
ExtraCache::~ExtraCache()
{
   delete [] SortedPkgs;
   delete [] SecSortedPkgs;
}
									/*}}}*/
// ExtraCache::NameComp - QSort compare by name				/*{{{*/
// ---------------------------------------------------------------------
/* */
int ExtraCache::NameComp(const void *a,const void *b)
{
   const Package &A = **(Package **)a;
   const Package &B = **(Package **)b;

   return strcmp(SortCache->StrP + A.Name,SortCache->StrP + B.Name);
}
									/*}}}*/
// ExtraCache::SecNameComp - Compare by name and then by section	/*{{{*/
// ---------------------------------------------------------------------
/* */
int ExtraCache::SecNameComp(const void *a,const void *b)
{
   const Package &A = **(Package **)a;
   const Package &B = **(Package **)b;

   // Sort by section first
   if (A.Section == B.Section)
      return strcmp(SortCache->StrP + A.Name,SortCache->StrP + B.Name);
   return strcmp(SortCache->StrP + A.Section,SortCache->StrP + B.Section);
}
									/*}}}*/
// ExtraCache::Init - Generate the initial extra structures.		/*{{{*/
// ---------------------------------------------------------------------
/* This allocats the extension buffers and then sorts the packages two
   ways. */
bool ExtraCache::Init()
{
   // Grab some memory including space for the trailing null
   delete [] SortedPkgs;
   delete [] SecSortedPkgs;
   SortedPkgs = new Package *[Head().PackageCount + 1];
   SecSortedPkgs = new Package *[Head().PackageCount + 1];
   
   // Initialize both lists
   int J = 0;
   for (PkgIterator I = PkgBegin(); I.end() != true; I++, J++)
      SortedPkgs[J] = SecSortedPkgs[J] = I;
   SortedPkgs[J] = SecSortedPkgs[J] = 0;

   // Sort them
   SortCache = this;
   qsort(SortedPkgs,J,sizeof(*SortedPkgs),NameComp);
   qsort(SecSortedPkgs,J,sizeof(*SecSortedPkgs),SecNameComp);

   return true;
} 
									/*}}}*/
// ExtraCache::GetCandidateVer - Returns the Candidate install version	/*{{{*/
// ---------------------------------------------------------------------
/* */
ExtraCache::VerIterator ExtraCache::GetCandidateVer(PkgIterator Pkg)
{
   return CurPolicy.GetCandidateVer(Pkg);
};
									/*}}}*/
// ExtraCache::IsImportantDep - True if the dep is important		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool ExtraCache::IsImportantDep(DepIterator Dep)
{
   return CurPolicy.IsImportantDep(Dep);
};
									/*}}}*/

// ExtraCacheF::ExtraCacheF - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* This creats objects to load the cache data into memory in an nice 
   single package. */
ExtraCacheF::ExtraCacheF(Policy &Pol,OpProgress &Prog) : 
              File(0), Map(0), Cache(0)
{
   string FName = _config->FindFile("Dir::Cache::pkgcache");
   File = new FileFd(FName,FileFd::ReadOnly);
   if (_error->PendingError() == true)
      return; 
   
   Map = new MMap(*File,MMap::Public | MMap::ReadOnly);
   if (_error->PendingError() == true)
      return; 
   
   Cache = new ExtraCache(Pol,*Map,Prog);
   if (_error->PendingError() == true)
      return;    
}
									/*}}}*/
// ExtraCacheF::~ExtraCacheF - Destructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
ExtraCacheF::~ExtraCacheF()
{
   delete Cache;
   delete Map;
   delete File;
}
									/*}}}*/
