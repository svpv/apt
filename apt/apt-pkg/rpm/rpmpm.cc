// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################

   RPM Package Manager - Provide an interface to rpm
   
   ##################################################################### 
 */
									/*}}}*/
// Includes								/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmpm.h"
#endif

#include <apt-pkg/rpmpm.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

#include <string.h>

#include <i18n.h>

#include <rpm/rpmlib.h>
									/*}}}*/
// RPMPM::pkgRPMPM - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMPM::pkgRPMPM(pkgDepCache &Cache) : pkgPackageManager(Cache)
{
    noninteractive = false;
}
									/*}}}*/
// RPMPM::pkgRPMPM - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRPMPM::~pkgRPMPM()
{
}
									/*}}}*/
// RPMPM::Install - Install a package					/*{{{*/
// ---------------------------------------------------------------------
/* Add an install operation to the sequence list */
bool pkgRPMPM::Install(PkgIterator Pkg,string File)
{
   if (File.empty() == true || Pkg.end() == true)
      return _error->Error(_("Internal Error, No file name for %s"),Pkg.Name());

   List.push_back(Item(Item::Install,Pkg,File));
   return true;
}
									/*}}}*/
// RPMPM::Configure - Configure a package				/*{{{*/
// ---------------------------------------------------------------------
/* Add a configure operation to the sequence list */
bool pkgRPMPM::Configure(PkgIterator Pkg)
{
   if (Pkg.end() == true) {
      return false;
   }
   
//   List.push_back(Item(Item::Configure,Pkg));
   return true;
}
									/*}}}*/
// RPMPM::Remove - Remove a package					/*{{{*/
// ---------------------------------------------------------------------
/* Add a remove operation to the sequence list */
bool pkgRPMPM::Remove(PkgIterator Pkg,bool Purge)
{
   if (Pkg.end() == true)
      return false;
   
//   if (Purge == true)
//      List.push_back(Item(Item::Purge,Pkg));
//   else
      List.push_back(Item(Item::Remove,Pkg));
   return true;
}
									/*}}}*/
// RPMPM::RunScripts - Run a set of scripts				/*{{{*/
// ---------------------------------------------------------------------
/* This looks for a list of script sto run from the configuration file,
   each one is run with system from a forked child. */
bool pkgRPMPM::RunScripts(const char *Cnf)
{
   Configuration::Item const *Opts = _config->Tree(Cnf);
   if (Opts == 0 || Opts->Child == 0)
      return true;
   Opts = Opts->Child;

   // Fork for running the system calls
   pid_t Child = ExecFork();
   
   // This is the child
   if (Child == 0)
   {
      if (chdir("/tmp/") != 0)
	 _exit(100);
	 
      unsigned int Count = 1;
      for (; Opts != 0; Opts = Opts->Next, Count++)
      {
	 if (Opts->Value.empty() == true)
	    continue;
	 
	 if (system(Opts->Value.c_str()) != 0)
	    _exit(100+Count);
      }
      _exit(0);
   }      

   // Wait for the child
   int Status = 0;
   while (waitpid(Child,&Status,0) != Child)
   {
      if (errno == EINTR)
	 continue;
      return _error->Errno("waitpid",_("Couldn't wait() for subprocess"));
   }

   // Restore sig int/quit
   signal(SIGQUIT,SIG_DFL);
   signal(SIGINT,SIG_DFL);   

   // Check for an error code.
   if (WIFEXITED(Status) == 0 || WEXITSTATUS(Status) != 0)
   {
      unsigned int Count = WEXITSTATUS(Status);
      if (Count > 100)
      {
	 Count -= 100;
	 for (; Opts != 0 && Count != 1; Opts = Opts->Next, Count--);
	 _error->Error(_("Problem executing scripts %s '%s'"),Cnf,Opts->Value.c_str());
      }
      
      return _error->Error(_("Sub-process returned an error code"));
   }
   
   return true;
}

                                                                        /*}}}*/
// RPMPM::RunScriptsWithPkgs - Run scripts with package names on stdin /*{{{*/
// ---------------------------------------------------------------------
/* This looks for a list of scripts to run from the configuration file
   each one is run and is fed on standard input a list of all .deb files
   that are due to be installed. */
bool pkgRPMPM::RunScriptsWithPkgs(const char *Cnf)
{
   Configuration::Item const *Opts = _config->Tree(Cnf);
   if (Opts == 0 || Opts->Child == 0)
      return true;
   Opts = Opts->Child;
   
   unsigned int Count = 1;
   for (; Opts != 0; Opts = Opts->Next, Count++)
   {
      if (Opts->Value.empty() == true)
         continue;
		
      // Create the pipes
      int Pipes[2];
      if (pipe(Pipes) != 0)
	 return _error->Errno("pipe",_("Failed to create IPC pipe to subprocess"));
      SetCloseExec(Pipes[0],true);
      SetCloseExec(Pipes[1],true);
      
      // Purified Fork for running the script
      pid_t Process = ExecFork();      
      if (Process == 0)
      {
	 // Setup the FDs
	 dup2(Pipes[0],STDIN_FILENO);
	 SetCloseExec(STDOUT_FILENO,false);
	 SetCloseExec(STDIN_FILENO,false);      
	 SetCloseExec(STDERR_FILENO,false);

	 const char *Args[4];
	 Args[0] = "/bin/sh";
	 Args[1] = "-c";
	 Args[2] = Opts->Value.c_str();
	 Args[3] = 0;
	 execv(Args[0],(char **)Args);
	 _exit(100);
      }
      close(Pipes[0]);
      FileFd Fd(Pipes[1]);

      // Feed it the filenames.
      for (vector<Item>::iterator I = List.begin(); I != List.end(); I++)
      {
	 // Only deal with packages to be installed from .rpm
	 if (I->Op != Item::Install)
	    continue;

	 // No errors here..
	 if (I->File[0] != '/')
	    continue;
	 
	 /* Feed the filename of each package that is pending install
	    into the pipe. */
	 if (Fd.Write(I->File.begin(),I->File.length()) == false || 
	     Fd.Write("\n",1) == false)
	 {
	    kill(Process,SIGINT);	    
	    Fd.Close();   
	    ExecWait(Process,Opts->Value.c_str(),true);
	    return _error->Error(_("Failure running script %s"),Opts->Value.c_str());
	 }
      }
      Fd.Close();
      
      // Clean up the sub process
      if (ExecWait(Process,Opts->Value.c_str()) == false)
	 return _error->Error(_("Failure running script %s"),Opts->Value.c_str());
   }

   return true;
}

									/*}}}*/

bool pkgRPMPM::ExecRPM(Operation operation, slist<char*> *files, bool nodeps)
{
   // Generate the argument list
   const char *Args[10000];      
   unsigned int n = 0;
   string options;

   Args[n++] = _config->Find("Dir::Bin::rpm","rpm").c_str();
      
   // Stick in any custom rpm options
   Configuration::Item const *Opts = _config->Tree("RPM::Options");
   if (Opts != 0)
   {
      Opts = Opts->Child;
      for (; Opts != 0; Opts = Opts->Next)
      {
	 if (Opts->Value.empty() == true)
	     continue;
	 Args[n++] = Opts->Value.c_str();
      }	 
   }

   switch (operation) {
    case OInstall:
    case OUpgrade:
      Opts = _config->Tree("RPM::UpgradeOptions");
      break;

    case ORemove:
      Opts = _config->Tree("RPM::RemoveOptions");
      break;

    case OCheckSignature:
      break;
   }
   
   if (Opts != 0)
   {
      Opts = Opts->Child;
      for (; Opts != 0; Opts = Opts->Next)
      {
	 if (Opts->Value.empty() == true)
	     continue;
	 Args[n++] = Opts->Value.c_str();
      }	 
   }

   string rootdir = _config->Find("RPM::RootDir", "");
   if (!rootdir.empty()) {
       Args[n++] = "-r";
       Args[n++] = rootdir.c_str();
   }
   

   switch (operation) {
    case OInstall:
      options = "-i";
	    
      Args[n++] = "-i";
      
      Args[n++] = "--replacepkgs";

      if (noninteractive)
	 Args[n++] = "--percent";
      else
	 Args[n++] = "-h";
      
      if (_config->FindB("RPM::Force", false) == true) 
	  Args[n++] = "--force";
      
      break;
      
    case OUpgrade:
      options = "-U";
      Args[n++] = "-Uv";
      
      Args[n++] = "--replacepkgs";

      if (noninteractive)
	 Args[n++] = "--percent";
      else
	 Args[n++] = "-h";
      
      if (_config->FindB("RPM::Force", false) == true)
	  Args[n++] = "--force";
      break;

    case ORemove:
      options = "-e";
      Args[n++] = "-e";      
      break;

    case OCheckSignature:
      options = "-K";
      Args[n++] = "-Kv";
      break;
   }
    
   if (nodeps)
       Args[n++] = "--nodeps";

   
   for (slist<char*>::iterator i = files->begin();
	i != files->end() && n < sizeof(Args);
	i++)
   {
      Args[n++] = *i;
   }
   Args[n++] = 0;

   
   // spit out command line for debugging
   if (_config->FindB("Debug::pkgRPMPM",false) == true)
   {
      for (unsigned int k = 0; k < n; k++)
	  clog << Args[k] << ' ';
      clog << endl;
      return true;
   }

   cout << _("Executing RPM (")<<options<<")..." << endl;

   cout << flush;
   clog << flush;
   cerr << flush;

   /* Mask off sig int/quit. We do this because dpkg also does when 
    it forks scripts. What happens is that when you hit ctrl-c it sends
    it to all processes in the group. Since dpkg ignores the signal 
    it doesn't die but we do! So we must also ignore it */
   //akk ??
   signal(SIGQUIT,SIG_IGN);
   signal(SIGINT,SIG_IGN);

   // Fork rpm
   pid_t Child = ExecFork();
            
   // This is the child
   if (Child == 0)
   {
      if (chdir(_config->FindDir("RPM::Run-Directory","/").c_str()) != 0)
	  _exit(100);
	 
      if (_config->FindB("RPM::FlushSTDIN",true) == true)
      {
	 int Flags,dummy;
	 if ((Flags = fcntl(STDIN_FILENO,F_GETFL,dummy)) < 0)
	     _exit(100);
	 
	 // Discard everything in stdin before forking
	 if (fcntl(STDIN_FILENO,F_SETFL,Flags | O_NONBLOCK) < 0)
	     _exit(100);
	 
	 while (read(STDIN_FILENO,&dummy,1) == 1);
	 
	 if (fcntl(STDIN_FILENO,F_SETFL,Flags & (~(long)O_NONBLOCK)) < 0)
	     _exit(100);
      }

      execvp(Args[0],(char **)Args);
      cerr << _("Could not exec() ") << Args[0] << endl;
      _exit(100);
   }      
   
   // Wait for rpm
   int Status = 0;
   while (waitpid(Child,&Status,0) != Child)
   {
      if (errno == EINTR)
	  continue;
      return _error->Errno("waitpid",_("Couldn't wait for subprocess"));
   }

   // Restore sig int/quit
   signal(SIGQUIT,SIG_DFL);
   signal(SIGINT,SIG_DFL);
       
   // Check for an error code.
   if (WIFEXITED(Status) == 0 || WEXITSTATUS(Status) != 0)
   {
      if (WIFSIGNALED(Status) != 0)
	  return _error->Error(_("Sub-process %s terminated by signal (%i)") ,Args[0], WTERMSIG(Status) );
      
      if (WIFEXITED(Status) != 0)
	  return _error->Error(_("Sub-process %s returned an error code (%u)"),Args[0],
			       WEXITSTATUS(Status));
      
      return _error->Error(_("Sub-process %s exited unexpectedly"),Args[0]);
   }
    
   return true;
}


bool pkgRPMPM::Process(slist<char*> *install, 
		       slist<char*> *upgrade,
		       slist<char*> *uninstall)
{
   if (_config->FindB("RPM::Check-Signatures", false) == true) 
   {
      cout << "Verifying signatures for individual packages..." << endl;
      cout << "Package operations will not be performed." << endl;
      
      if (!install->empty())
	  ExecRPM(OCheckSignature, install, false);
      
      if (!upgrade->empty())
	  ExecRPM(OCheckSignature, upgrade, false);
      
      if (_error->PendingError() == true) 
	  return _error->Error("Errors found while verifying individual package signatures.");
      else
	  return true;
   }

   if (!uninstall->empty())
       ExecRPM(ORemove, uninstall, true);

   if (!install->empty())
       ExecRPM(OInstall, install, true);
   
   if (!upgrade->empty())
       ExecRPM(OUpgrade, upgrade, false);
 
   return true;
}


// RPMPM::Go - Run the sequence					/*{{{*/
// ---------------------------------------------------------------------
/* This globs the operations and calls rpm */
bool pkgRPMPM::Go()
{
   bool RPMUpgrade = false;
    
   if (RunScripts("RPM::Pre-Invoke") == false)
      return false;

   if (RunScriptsWithPkgs("RPM::Pre-Install-Pkgs") == false)
      return false;
   
   slist<char*> *install = new slist<char*>;
   slist<char*> *upgrade = new slist<char*>;
   slist<char*> *uninstall = new slist<char*>;
   
   for (vector<Item>::iterator I = List.begin(); I != List.end(); I++)
   {
      switch (I->Op)
      {
       case Item::Purge:
       case Item::Remove:
	 if (strchr(I->Pkg.Name(), '#'))
	 {
	    char *ptr = strdup(I->Pkg.Name());
	    char *p1 = strchr(ptr, '#');
	    char *p2 = strrchr(ptr, '#');
	    if (p1 == p2) 
	    {
		*p1 = '-';
	       uninstall->push_front(ptr); // akk: leak
	    }
	    else 
	    {
	       p2 = strrchr(I->Pkg.Name(), '#');
	       strcpy(p1, p2);
	       *p1 = '-';
	       uninstall->push_front(ptr);
	    }
	 }
	 else 
	 {
	    uninstall->push_front((char*)I->Pkg.Name());
	 }
	 break;

       case Item::Configure:
//	 cout << __FUNCTION__ << ": ignoring request for configure\n";
	 break;

       case Item::Install:
	 // eeek!! yuck!!! bleh!!! blame rpm	 
	 if (strcmp(I->Pkg.Name(), "rpm") == 0
		|| strcmp(I->Pkg.Name(), "librpm") == 0)
	     RPMUpgrade = true;
	 
	 if (strchr(I->Pkg.Name(), '#'))
	 {
	    install->push_front((char*)I->File.c_str());
	 }
	 else
	 {
	    upgrade->push_front((char*)I->File.c_str());
	 }
	 break;
	  
       default:
	 cout << __FUNCTION__ << "UNKNOWN OPERATION!!!!\n";
	 break;
      }
   }

   if (RPMUpgrade == true && _config->FindB("RPM::AutoRebuildDB", true) == true)
   {
      int res;
      
      cerr << (_("Rebuilding RPM database (this may take a few minutes)...")) << endl;
            
      res = rpmdbRebuild(_config->FindDir("RPM::RootDir", "/").c_str());
      if (res != 0)
	 return _error->Error(_("could not rebuild RPM database for upgrade of RPM"));
   }
   
   bool result = Process(install, upgrade, uninstall);
   
   delete install;
   delete upgrade;
   delete uninstall;

   if (!result) {
       RunScripts("RPM::Post-Invoke");
       return false;
   }
    
   if (RunScripts("RPM::Post-Invoke") == false)
      return false;

   if (RunScriptsWithPkgs("RPM::Post-Invoke-Pkgs") == false)
      return false;

   return true;
}
									/*}}}*/
// pkgRPMPM::Reset - Dump the contents of the command list		/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgRPMPM::Reset() 
{
   List.erase(List.begin(),List.end());
}
									/*}}}*/
