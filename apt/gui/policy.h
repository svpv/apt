// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: policy.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Policy - Contains a number of configurable algorithms designed to 
            effect the way things work.
   
   Upgrade Policy - This controls the selection of packages for 
      'Inst-Version'.
   List Policy - Controls wheather a package is currently displayed
       in the tree.
   Column Policy - Controls wheather a column is displayed in the tree

   Terms:
      Candidate Version - Version which is offered for install to the user.
                          This shows up as the Install-Version on the tree.
      Install Version - Version that is actually going to be installed
                        This is the canidate version, current version or 
                        none (remove).
      Current Version - Version that is installed in the system right now.
      Displayable - Meets the users profile/filter requirements
   
   This may end up being derived from a pkgPolicy class so alogrithms 
   in pkglib can use a similar scheme..
   
   ##################################################################### */
									/*}}}*/
#ifndef POLICY_H
#define POLICY_H

#include <apt-pkg/pkgcache.h>

class ExtraCache;
class Policy
{
   public:

   // Which packages to display
   struct
   {
      bool InProfile;
      bool Installed;
      bool ToUpgrade;
      bool ToDowngrade;
      bool HeldUpDn;
      bool New;
      bool Obsolete;
      bool NotInstalled;
      bool Broken;
      bool All;
   } Displayable;

   // Which columns to display
   struct ColumnsT
   {
      bool CurVersion;
      bool InstVersion;
      bool InstAsWell;
   } Columns;

   struct
   {
      bool Suggests;
      bool Recommends;
   } ImportantDeps;
   
   bool ShowSections;
   
   // Single global instance of the policy class
   static Policy *Cur;
   
   // Find the Install Candidate Version of a package
   pkgCache::VerIterator GetCandidateVer(pkgCache::PkgIterator Pkg);

   // See if the Pkg meets the display policy
   bool ShouldDisplay(ExtraCache &Cache,pkgCache::PkgIterator Pkg);
   bool IsImportantDep(pkgCache::DepIterator Dep);
      
   Policy();
};

#endif
