// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: policy.cc,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Policy - Contains a number of configurable algorithms designed to 
            effect the way things work.

   The policy class allows user control of some important policy choices
   and algorithms. Other classes use this class to determine how to
   make choices. 
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include "policy.h"
#include "extracache.h"
									/*}}}*/

Policy *Policy::Cur = new Policy;

// Policy::Policy - Constructor						/*{{{*/
// ---------------------------------------------------------------------
/* */
Policy::Policy()
{
   Displayable.InProfile = false;
   Displayable.Installed = true;
   Displayable.ToUpgrade = false;
   Displayable.ToDowngrade = false;
   Displayable.HeldUpDn = false;
   Displayable.New = false;
   Displayable.Obsolete = false;
   Displayable.NotInstalled = false;
   Displayable.Broken = false;
   Displayable.All = false;

   Columns.CurVersion = false;
   Columns.InstVersion = true;
   Columns.InstAsWell = true;

   ImportantDeps.Suggests = true;
   ImportantDeps.Recommends = false;

   ShowSections = true;
}
									/*}}}*/
// Policy::GetCandidateVersion - Determine the candidate install ver	/*{{{*/
// ---------------------------------------------------------------------
/* This decides what version to offer for installation based on 
   user preferences. It is used to generate the extra cache which will
   cache the result of this function. */
pkgCache::VerIterator Policy::GetCandidateVer(pkgCache::PkgIterator Pkg)
{
   // Try to use an explicit target
   if (Pkg->TargetVer == 0)
      return Pkg.VersionList();
   else
      return Pkg.TargetVer();
}
									/*}}}*/
// Policy::ShouldDisplay - Determine if a package meets the policy	/*{{{*/
// ---------------------------------------------------------------------
/* We just run through all the items on the displayable list.. The 
   extra cache is used to speed this up. */
bool Policy::ShouldDisplay(ExtraCache &Cache,pkgCache::PkgIterator Pkg)
{
   /* Packages without versions are not shown ever, these are probably
      virtual package place holders or some other evilness */
   if (Pkg->VersionList == 0)
      return false;

   // All Packages
   if (Displayable.All == true)
      return true;
   
   // Packages with an installed version
   if (Displayable.Installed == true && Pkg->CurrentVer != 0)
      return true;
  
   // Packages not installed
   if (Displayable.NotInstalled == true && Pkg->CurrentVer == 0)
      return true;

   // Already Cached info about the package
   ExtraCache::StateCache &State = Cache[Pkg];
   
   // Held upgrade/downgrade
   if (Displayable.HeldUpDn == true && Pkg->CurrentVer != 0 && 
       State.Held() == true)
      return true;

   // Broken
   if (Displayable.Broken == true &&
       (State.DepState & ExtraCache::DepInstMin) == 0)
      return true;
          
   // Target version is newer
   if (Displayable.ToUpgrade == true && State.Upgrade() == true)
      return true;

   // Cant downgrade a not installed package
   if (Pkg->CurrentVer == 0)
      return false;

   // Target version is older or delete is selected
   if (Displayable.ToDowngrade == true && (State.Downgrade() == true || 
					   State.Delete() == true))
      return true;

   return false;
}
									/*}}}*/
// Policy::IsImportantDep - Returns true if the dependency is important	/*{{{*/
// ---------------------------------------------------------------------
/* This is used to control what dependencies are important. An important
   dependency is one that should be automatically installed but is not
   critical for the package. The other type of dependency is a critical
   dependency that must be present for the package to be installable. */
bool Policy::IsImportantDep(pkgCache::DepIterator Dep)
{
   // Basic important dependencies
   if (Dep.IsCritical() == true)
      return true;
   
   // User control for suggests being important
   if (ImportantDeps.Suggests == true && Dep->Type == pkgCache::Dep::Suggests)
      return true;
   
   // User control for Recommends being important
   if (ImportantDeps.Recommends == true && Dep->Type == pkgCache::Dep::Recommends)
      return true;

   return false;
}
									/*}}}*/
