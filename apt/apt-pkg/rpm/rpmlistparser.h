// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmlistparser.h,v 1.15 2001/11/30 20:34:13 kojima Exp $
/* ######################################################################
   
   Debian Package List Parser - This implements the abstract parser 
   interface for rpmian package files
   
   ##################################################################### 
 */
									/*}}}*/
// Header section: pkglib
#ifndef PKGLIB_RPMLISTPARSER_H
#define PKGLIB_RPMLISTPARSER_H

#include <apt-pkg/pkgcachegen.h>
#include <rpm/rpmlib.h>
#include <map>
#include <slist>
#include <vector>
#include <regex.h>

class rpmListParser : public pkgCacheGenerator::ListParser
{
   Header header;
   FileFd *file;
   unsigned long offset;
   
   vector<string> Essentials;
   vector<string> Importants;
   
   map<string,int> *filedeps;
   map<string,string> *multiarchs;
   
   map<string,long> *DupPackages;
   slist<regex_t*> *AllowedDupPackages;
   bool duplicated;
   
   slist<regex_t*> *HoldPackages;   
   
   bool parsing_hdlist;
   
   char *Arch;
   char *BaseArch;
   
   // Parser Helper
   struct WordList
   {
      char *Str;
      unsigned char Val;
   };
   
   bool GetConfig();
   
   bool IsMultiArch(string package);
   
   unsigned long UniqFindTagWrite(int Tag);
   bool ParseStatus(pkgCache::PkgIterator Pkg,pkgCache::VerIterator Ver);
   bool ParseDepends(pkgCache::VerIterator Ver,
		     char **namel, char **verl, int_32 *flagl,
		     int count, unsigned int Type);
   bool ParseDepends(pkgCache::VerIterator Ver, unsigned int Type);
   bool ParseProvides(pkgCache::VerIterator Ver);
   
   bool ProcessFileProvides(pkgCache::VerIterator Ver);
   
   Header NextHeader();

   bool ShouldBeIgnored(string pkg);

 public:

   // These all operate against the current header
   virtual string Package();
   virtual string Version();
   virtual string Architecture();
   virtual bool NewVersion(pkgCache::VerIterator Ver);
   virtual unsigned short VersionHash();
   virtual bool UsePackage(pkgCache::PkgIterator Pkg,
			   pkgCache::VerIterator Ver);
   virtual unsigned long Offset() {return offset;};
   virtual unsigned long Size();
   
   virtual bool Step();
   
   bool LoadReleaseInfo(pkgCache::PkgFileIterator FileI,FileFd &File);
   
   rpmListParser(FileFd &File, map<string,int> *fdeps, map<string,string> *marchs);
   rpmListParser(map<string,int> *fdeps, map<string,string> *marchs);
   ~rpmListParser();
};

#endif
