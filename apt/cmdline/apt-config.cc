// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: apt-config.cc,v 1.3 2001/08/01 21:57:05 kojima Exp $
/* ######################################################################
   
   APT Config - Program to manipulate APT configuration files
   
   This program will parse a config file and then do something with it.
   
   Commands:
     shell - Shell mode. After this a series of word pairs should occure.
             The first is the environment var to set and the second is
             the key to set it from. Use like: 
 eval `apt-config shell QMode apt::QMode`
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <apt-pkg/cmndline.h>
#include <apt-pkg/error.h>
#include <apt-pkg/init.h>
#include "config.h"

#include <i18n.h>

#include <iostream>
									/*}}}*/

// DoShell - Handle the shell command					/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoShell(CommandLine &CmdL)
{
   for (const char **I = CmdL.FileList + 1; *I != 0; I += 2)
   {
      if (I[1] == 0 || strlen(I[1]) == 0)
	 return _error->Error(_("Arguments not in pairs"));

      // Check if the caller has requested a directory path
      if (I[1][strlen(I[1])-1] == '/')
      {
	 char S[300];
	 strcpy(S,I[1]);
	 S[strlen(S)-1] = 0;
	 if (_config->Exists(S) == true)
	    cout << *I << "=\"" << _config->FindDir(S) << '"' << endl;
      }
      
      if (_config->Exists(I[1]) == true)
	 cout << *I << "=\"" << _config->Find(I[1]) << '"' << endl;
   }
   
   return true;
}
									/*}}}*/
// DoDump - Dump the configuration space				/*{{{*/
// ---------------------------------------------------------------------
/* */
bool DoDump(CommandLine &CmdL)
{
   _config->Dump();
   return true;
}
									/*}}}*/
// ShowHelp - Show the help screen					/*{{{*/
// ---------------------------------------------------------------------
/* */
int ShowHelp()
{
   cout << PACKAGE << ' ' << VERSION << " for " << COMMON_CPU <<
       " compiled on " << __DATE__ << "  " << __TIME__ << endl;
   if (_config->FindB("version") == true)
      return 100;
   
   cout << _("Usage: apt-config [options] command") << endl;
   cout << endl;
   cout << _("apt-config is a simple tool to read the APT config file") << endl;   
   cout << endl;
   cout << _("Commands:") << endl;
   cout << _("   shell - Shell mode") << endl;
   cout << _("   dump - Show the configuration") << endl;
   cout << endl;
   cout << _("Options:") << endl;
   cout << _("  -h   This help text.") << endl;
   cout << _("  -c=? Read this configuration file") << endl;
   cout << _("  -o=? Set an arbitary configuration option, eg -o dir::cache=/tmp") << endl;
   return 100;
}
									/*}}}*/

int main(int argc,const char *argv[])
{
   CommandLine::Args Args[] = {
      {'h',"help","help",0},
      {'v',"version","version",0},
      {'c',"config-file",0,CommandLine::ConfigFile},
      {'o',"option",0,CommandLine::ArbItem},
      {0,0,0,0}};
   CommandLine::Dispatch Cmds[] = {{"shell",&DoShell},
                                   {"dump",&DoDump},
                                   {0,0}};
    
   setlocale(LC_ALL, "");
   bindtextdomain(PACKAGE, LOCALEDIR);
   textdomain(PACKAGE);
   
   // Parse the command line and initialize the package library
   CommandLine CmdL(Args,_config);
   if (pkgInitialize(*_config) == false ||
       CmdL.Parse(argc,argv) == false)
   {
      _error->DumpErrors();
      return 100;
   }

   // See if the help should be shown
   if (_config->FindB("help") == true ||
       CmdL.FileSize() == 0)
      return ShowHelp();

   // Match the operation
   CmdL.DispatchArg(Cmds);
   
   // Print any errors or warnings found during parsing
   if (_error->empty() == false)
   {
      bool Errors = _error->PendingError();
      _error->DumpErrors();
      return Errors == true?100:0;
   }
   
   return 0;
}
