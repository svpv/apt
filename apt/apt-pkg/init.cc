// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: init.cc,v 1.16 2001/08/01 21:35:12 kojima Exp $
/* ######################################################################

   Init - Initialize the package library
   
   ##################################################################### */
									/*}}}*/
// Include files							/*{{{*/
#include <apt-pkg/init.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/error.h>
#include <config.h>
									/*}}}*/


// pkgInitialize - Initialize the configuration class			/*{{{*/
// ---------------------------------------------------------------------
/* Directories are specified in such a way that the FindDir function will
   understand them. That is, if they don't start with a / then their parent
   is prepended, this allows a fair degree of flexability. */
bool pkgInitialize(Configuration &Cnf)
{
   // General APT things
   Cnf.Set("APT::Architecture",COMMON_CPU);

   // State
   Cnf.Set("Dir::State","/var/state/apt/");
   Cnf.Set("Dir::State::lists","lists/");
   
   /* These really should be jammed into a generic 'Local Database' engine
      which is yet to be determined. The functions in pkgcachegen should
      be the only users of these */
   Cnf.Set("Dir::State::xstatus","xstatus");
   Cnf.Set("Dir::State::userstatus","status.user");
   if (0) {//akk
      Cnf.Set("Dir::State::status","/var/lib/dpkg/status");
   } else {
      Cnf.Set("Acquire::cdrom::mount", "/mnt/cdrom");
      Cnf.Set("RPM::AllowedDupPkgs::","^kernel$");
      Cnf.Set("RPM::AllowedDupPkgs::", "kernel-smp");
      Cnf.Set("RPM::AllowedDupPkgs::", "kernel-enterprise");

      Cnf.Set("RPM::HoldPkgs::", "kernel-source");
      Cnf.Set("RPM::HoldPkgs::", "kernel-headers");

      Cnf.Set("Dir::State::status","/var/lib/rpm/status");
   }
   Cnf.Set("Dir::State::cdroms","cdroms.list");
   
   // Cache
   Cnf.Set("Dir::Cache","/var/cache/apt/");
   Cnf.Set("Dir::Cache::archives","archives/");
   Cnf.Set("Dir::Cache::srcpkgcache","srcpkgcache.bin");
   Cnf.Set("Dir::Cache::pkgcache","pkgcache.bin");
   
   // Configuration
   Cnf.Set("Dir::Etc","/etc/apt/");
   Cnf.Set("Dir::Etc::sourcelist","sources.list");
   Cnf.Set("Dir::Etc::vendorlist","vendors.list");
   Cnf.Set("Dir::Etc::main","apt.conf");
   Cnf.Set("Dir::Bin::gpg","/usr/bin/gpg");
   Cnf.Set("Dir::Bin::methods","/usr/lib/apt/methods");
   if (0) {//akk
      Cnf.Set("Dir::Bin::dpkg","/usr/bin/dpkg");
      Cnf.Set("Acquire::ComprExtension", ".gz");
   } else {
      Cnf.Set("Dir::Etc::RpmPriorities", "rpmpriorities");
      Cnf.Set("Dir::bin::gzip","/usr/bin/bzip2");
      Cnf.Set("Dir::Bin::rpm","/bin/rpm");
      Cnf.Set("Acquire::ComprExtension", ".bz2");
   }
   
   // Read the main config file
   string FName = Cnf.FindFile("Dir::Etc::main");
   bool Res = true;
   if (FileExists(FName) == true)
      Res &= ReadConfigFile(Cnf,FName);
   
   // Read an alternate config file
   const char *Cfg = getenv("APT_CONFIG");
   if (Cfg != 0 && FileExists(Cfg) == true)
      Res &= ReadConfigFile(Cnf,Cfg);
   
   if (Res == false)
      return false;
   
   if (Cnf.FindB("Debug::pkgInitialize",false) == true)
      Cnf.Dump();
      
   return true;
}
									/*}}}*/
