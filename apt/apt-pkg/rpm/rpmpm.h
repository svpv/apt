// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmpm.h,v 1.6 2001/11/12 16:04:38 kojima Exp $
/* ######################################################################

   rpm Package Manager - Provide an interface to rpm
   
   ##################################################################### 
 */
									/*}}}*/
#ifndef PKGLIB_rpmPM_H
#define PKGLIB_rpmPM_H

#ifdef __GNUG__
#pragma interface "apt-pkg/rpmpm.h"
#endif

#include <apt-pkg/packagemanager.h>
#include <vector>
#include <slist>

    
class pkgRPMPM : public pkgPackageManager
{
   bool noninteractive;

   protected:
   
   struct Item
   {
      enum Ops {Install, Configure, Remove, Purge} Op;
      string File;
      PkgIterator Pkg;
      Item(Ops Op,PkgIterator Pkg,string File = "") : Op(Op), 
            File(File), Pkg(Pkg) {};
      Item() {};
      
   };
   
   enum Operation { OInstall, OUpgrade, ORemove, OCheckSignature };
   
   vector<Item> List;

   // Helpers
   bool RunScripts(const char *Cnf);
   bool RunScriptsWithPkgs(const char *Cnf);
   
   // The Actuall installation implementation
   virtual bool Install(PkgIterator Pkg,string File);
   virtual bool Configure(PkgIterator Pkg);
   virtual bool Remove(PkgIterator Pkg,bool Purge = false);
    
   bool ExecRPM(Operation operation, slist<char*> *files, bool nodeps);
   bool Process(slist<char*> *install,
		slist<char*> *upgrade,
		slist<char*> *uninstall);
   
   virtual bool Go();
   virtual void Reset();
   
   public:

   pkgRPMPM(pkgDepCache &Cache);
   virtual ~pkgRPMPM();
   
   void setNonInteractive() { noninteractive = true; };
};





#endif
