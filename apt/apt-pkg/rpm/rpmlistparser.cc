// Description
// $Id: rpmlistparser.cc,v 1.60 2001/12/11 20:50:12 kojima Exp $
// 
/* ######################################################################
 * Package Cache Generator - Generator for the cache structure.
 * This builds the cache structure from the abstract package list parser. 
 * 
 ##################################################################### 
 */

#include <config.h>

// Include Files
#include <apt-pkg/rpmlistparser.h>
#include <apt-pkg/rpminit.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/crc-16.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/rpmpackagedata.h>
#include <apt-pkg/rpmfactory.h>

#include <rpm/rpmlib.h>

#include <i18n.h>

#define DUPPACK


// ListParser::rpmListParser - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmListParser::rpmListParser(FileFd &File, map<string,int> *fdeps,
			     map<string,string> *marchs)
{
   file = &File;
   header = NULL;
   filedeps = fdeps;
   multiarchs = marchs;
   
   parsing_hdlist = true;
    
   DupPackages = NULL;
   
   GetConfig();
}

rpmListParser::rpmListParser(map<string,int> *fdeps, map<string,string> *marchs)
{   
   file = 0;
   header = NULL;
   filedeps = fdeps;
   multiarchs = marchs;
   
   parsing_hdlist = false;

   DupPackages = new map<string,long>();

   GetConfig();
}

rpmListParser::~rpmListParser()
{
#ifndef HAVE_RPM4
   if (header)
       headerFree(header);
#endif
   delete AllowedDupPackages;
   if (DupPackages)
       delete DupPackages;
}
/*}}}*/


bool rpmListParser::GetConfig()
{
   string str;
   char *begin, *end;
   char *p;
   
#ifdef DUPPACK
   AllowedDupPackages = new slist<regex_t*>;
   
   const Configuration::Item *Top = _config->Tree("RPM::AllowedDupPkgs");
   
   if (Top) {
      for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
      {
	 regex_t *ptrn = new regex_t;
	 
	 if (regcomp(ptrn,Top->Value.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0)
	 {
	    _error->Warning(_("Bad regular expression '%s' in option RPM::AllowedDupPkgs."),
			    Top->Value.c_str());
	    delete ptrn;
	 }
	 else
	     AllowedDupPackages->push_front(ptrn);
      }
   } else {
      str = _config->Find("RPM::AllowedDupPackages");
      if (!str.empty()) {
	 return _error->Error(_("Option RPM::AllowedDupPackages was replaced with RPM::AllowedDupPkgs, which is a list of regular expressions (apt-config dump for an example). Please update."));
      }
   }
#else
   AllowedDupPackages = NULL;//akk
#endif
   
   HoldPackages = new slist<regex_t*>;
   
   Top = _config->Tree("RPM::HoldPkgs");
   
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
   {
      regex_t *ptrn = new regex_t;
      
      if (regcomp(ptrn,Top->Value.c_str(),REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0)
      {
	 _error->Warning(_("Bad regular expression '%s' in option RPM::HoldPackages."),
			 Top->Value.c_str());
	 delete ptrn;
      }
      else
	  HoldPackages->push_front(ptrn);
   }

      
   str = _config->Find("APT::architecture");
   
   BaseArch = strdup(str.c_str());
   
   return true;
}

// ListParser::UniqFindTagWrite - Find the tag and write a unq string	/*{{{*/
// ---------------------------------------------------------------------
/* */
unsigned long rpmListParser::UniqFindTagWrite(int Tag)
{
   char *str;
   int type;
   int count;
   void *data;
   
   if (headerGetEntry(header, Tag, &type, &data, &count) != 1) {
      return 0;
   }
   if (type == RPM_STRING_TYPE) {
      str = (char*)data;
   } else {
      cout << _("oh shit, not handled for ")<<Package()<<_(" Tag:")<<Tag<<endl;
      abort();
   }
   
   return WriteUniqString(str, strlen(str));
}
                                                                        /*}}}*/
// ListParser::Package - Return the package name			/*{{{*/
// ---------------------------------------------------------------------
/* This is to return the name of the package this section describes */
string rpmListParser::Package()
{
   char *str;
   int type, count;
   
   duplicated = false;
   
   
   if (headerGetEntry(header, RPMTAG_NAME, &type, (void**)&str, &count) != 1) 
   {
      _error->Error(_("Corrupt pkglist: no RPMTAG_NAME in header entry"));
      return string();
   } 
   else 
   {
      bool dupok = false;
      string name = string(str);

      if (AllowedDupPackages != NULL)
      {
	 for (slist<regex_t*>::iterator iter = AllowedDupPackages->begin();
	      iter != AllowedDupPackages->end();
	      iter++)
	 {
	    if (regexec(*iter,str,0,0,0) == 0)
	    {
	       dupok = true;
	       break;
	    }
	 }
      }

#ifdef DUPPACK
      /*
       * If this package can have multiple versions installed at
       * the same time, then we make it so that the name of the
       * package is NAME+"#"+VERSION and also adds a provides
       * with the original name and version, to satisfy the 
       * dependencies.
       */
      if (dupok)
      {
	 string bla = name+"#"+Version();
	 duplicated = true;
	 return bla;
      } else if (DupPackages) {
	 string res = string(str);

	 if (DupPackages->find(res) != DupPackages->end()
	     && (*DupPackages)[res] != offset) {
	    _error->Error(_("There are two or more versions of the package '%s' installed in your "
			  "system, which is a situation APT can't handle cleanly at the moment.\n"
			  "Please do one of the following:\n"
			  "1) Remove the older packages, leaving only one version installed; or\n"
			  "2) If you do want to have multiple versions of that package, add the "
			  "package names to the RPM::AllowedDupPkgs option.\n"),
			  str);
	    (*DupPackages)[res] = offset;
	    return string();
	 }
	 (*DupPackages)[res] = offset;
      }
#endif
      return name;
   }
}
                                                                        /*}}}*/
// ListParser::Arch - Return the architecture string			/*{{{*/
// ---------------------------------------------------------------------
string rpmListParser::Architecture()
{
    int type, count;
    char *arch;
    int res;

    res = headerGetEntry(header, RPMTAG_ARCH, &type, (void **)&arch, &count);

    assert(res);
    
    return string(arch);
}
                                                                        /*}}}*/
// ListParser::Version - Return the version string			/*{{{*/
// ---------------------------------------------------------------------
/* This is to return the string describing the version in RPM form,
 version-release. If this returns the blank string then the
 entry is assumed to only describe package properties */
string rpmListParser::Version()
{
   char *ver, *rel;
   int_32 *ser;
   bool has_serial = false;
   int type, count;
   string str;
   
   if (headerGetEntry(header, RPMTAG_EPOCH, &type, (void **)&ser, &count)==1
       && count > 0) 
   {
      has_serial = true;
   }
   
   headerGetEntry(header, RPMTAG_VERSION, &type, (void **)&ver, &count);
   
   headerGetEntry(header, RPMTAG_RELEASE, &type, (void **)&rel, &count);
   
   if (has_serial) 
   {
      char buf[32];
      snprintf(buf, sizeof(buf), "%i", *ser);
      
      str = string(buf)+":"+string(ver)+"-"+string(rel);
   }
   else 
   {
       str = string(ver)+"-"+string(rel);
   }
   
   return str;
}
/*}}}*/
// ListParser::NewVersion - Fill in the version structure		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListParser::NewVersion(pkgCache::VerIterator Ver)
{
   int count, type;
   int_32 *num;
   
   // Parse the section
   Ver->Section = UniqFindTagWrite(RPMTAG_GROUP);
   Ver->Arch = UniqFindTagWrite(RPMTAG_ARCH);

   // Archive Size
   headerGetEntry(header, CRPMTAG_FILESIZE, &type, (void**)&num, &count);
   if (count > 0)
       Ver->Size = (unsigned)num[0];
   else
       Ver->Size = 1;
   
   // Unpacked Size (in kbytes)
   headerGetEntry(header, RPMTAG_SIZE, &type, (void**)&num, &count);
   Ver->InstalledSize = (unsigned)num[0];

//   Ver->InstalledSize /= 1024;
      
   if (ParseDepends(Ver,pkgCache::Dep::Depends) == false)
       return false;
    /*
     * We consider both Requires and PreRequires as simple Depends,
     * because that differentiation is only used for package ordering.
     * Since we delegate package ordering to rpm, it won't be usefull
     * in APT anyway.
   if (ParseDepends(Ver,pkgCache::Dep::PreDepends) == false)
       return false;
     */
   if (ParseDepends(Ver,pkgCache::Dep::Conflicts) == false)
       return false;
   if (ParseDepends(Ver,pkgCache::Dep::Obsoletes) == false)
       return false;
   if (ProcessFileProvides(Ver) == false)
       return false;

   if (ParseProvides(Ver) == false)
       return false;

   return true;
}
/*}}}*/
// ListParser::UsePackage - Update a package structure			/*{{{*/
// ---------------------------------------------------------------------
/* This is called to update the package with any new information
 that might be found in the section */
bool rpmListParser::UsePackage(pkgCache::PkgIterator Pkg,
			       pkgCache::VerIterator Ver)
{
   RPMPackageData *rpmdata;
   
   rpmdata = RPMPackageData::Singleton();
   if (_error->PendingError())
	return false;
   if (Pkg->Section == 0)
	Pkg->Section = UniqFindTagWrite(RPMTAG_GROUP);

   Ver->Priority = rpmdata->PackagePriority(string(Pkg.Name()));

   if (Ver->Priority == pkgCache::State::Important)
       Pkg->Flags |= pkgCache::Flag::Essential;

   if (Ver->Priority == pkgCache::State::Required ||
       Ver->Priority == pkgCache::State::Important)
       Pkg->Flags |= pkgCache::Flag::Important;
    
   if (ParseStatus(Pkg,Ver) == false)
       return false;
   
   return true;
}
/*}}}*/
// ListParser::VersionHash - Compute a unique hash for this version	/*{{{*/
// ---------------------------------------------------------------------
/* */

static int compare(const void *a, const void *b)
{   
   return strcmp(*(char**)a, *(char**)b);
}


unsigned short rpmListParser::VersionHash()
{
   int Sections[] ={
      RPMTAG_REQUIRENAME,
      RPMTAG_OBSOLETENAME,
      RPMTAG_CONFLICTNAME,
	  0
	  //                            "Pre-Depends",
	  //                            "Conflicts",
   };
   unsigned long Result = INIT_FCS;
   char S[300];
   char *I;
   
   for (const int *sec = Sections; *sec != 0; sec++)
   {
      char *Start;
      char *End;
      int type, count;
      int res;
      char **strings;
      
      res = headerGetEntry(header, *sec, &type, (void **)&strings, &count);
      if (res != 1 || count == 0) {
	 continue;
      }
      
      qsort(strings, count, sizeof(char*), compare);

      switch (type) {
       case RPM_STRING_ARRAY_TYPE:
	 while (count-- > 0) {
	    Start = strings[count];
	    
	    End = Start+strlen(Start);
	    
	    if (End - Start >= (signed)sizeof(S))
		continue;
	    
	    /* Strip out any spaces from the text */
	    I = S;
	    for (; Start != End; Start++) {
	       if (isspace(*Start) == 0)
		   *I++ = *Start;
	    }
	    
	    Result = AddCRC16(Result,S,I - S);
	    
//	    free(strings);
	 }
	 break;
	 
       case RPM_STRING_TYPE:
	 Start = (char*)strings;
	 
	 End = Start+strlen(Start);
	 
	 if (End - Start >= (signed)sizeof(S))
	     continue;
	 
	 /* Strip out any spaces from the text */
	 I = S;
	 for (; Start != End; Start++) {
	    if (isspace(*Start) == 0)
		*I++ = *Start;
	 }
	 Result = AddCRC16(Result,S,I - S);

	 break;
      }
   }
   
   return Result;
}
/*}}}*/
// ListParser::ParseStatus - Parse the status field			/*{{{*/
// ---------------------------------------------------------------------
bool rpmListParser::ParseStatus(pkgCache::PkgIterator Pkg,
				pkgCache::VerIterator Ver)
{   
   if (parsing_hdlist) { // this means we're parsing a hdlist, so it's not installed
      return true;
   }
   
   // if we're reading from the rpmdb, then it's installed

   bool hold = false;
   
   if (HoldPackages != NULL)
   {
      const char *str = Package().c_str();

      for (slist<regex_t*>::iterator iter = HoldPackages->begin();
	   iter != HoldPackages->end();
	   iter++)
      {
	 if (regexec(*iter,str,0,0,0) == 0)
	 {
	    hold = true;
	    break;
	 }
      }
   }
   
   if (hold) {
       Pkg->SelectedState = pkgCache::State::Hold;
   } else {
       Pkg->SelectedState = pkgCache::State::Install;
   }
    Pkg->InstState = pkgCache::State::Ok;    
    
       
   Pkg->CurrentState = pkgCache::State::Installed;
   
   /* A Status line marks the package as indicating the current
    version as well. Only if it is actually installed.. Otherwise
    the interesting dpkg handling of the status file creates bogus 
    entries. */
   if (!(Pkg->CurrentState == pkgCache::State::NotInstalled ||
	 Pkg->CurrentState == pkgCache::State::ConfigFiles))
   {
      if (Ver.end() == true)
	  _error->Warning(_("Encountered status field in a non-version description"));
      else
	  Pkg->CurrentVer = Ver.Index();
   }
   
   return true;
}


bool rpmListParser::ParseDepends(pkgCache::VerIterator Ver,
				 char **namel, char **verl, int_32 *flagl,
				 int count, unsigned int Type)
{
   int i;
   unsigned int Op = 0;
   
   for (i = 0; i < count; i++) {
      
      /*
      if (Type == pkgCache::Dep::Depends) {
	 if (flagl[i] & RPMSENSE_PREREQ)
	     continue;
      } else if (Type == pkgCache::Dep::PreDepends) {
	 if (!(flagl[i] & RPMSENSE_PREREQ))
	     continue;
      }
       */
      
      /*
       * <kluge>strip-off requires for rpmlib features..</kluge>
       */
      if (strncmp(namel[i], "rpmlib", 6) == 0) {
	 if (_config->FindB("RPM::IgnoreRpmlibDeps", false) == false ||
	     rpmCheckRpmlibProvides(namel[i], 
				    verl ? verl[i] : NULL,
				    flagl[i])) {
	    continue;
	 }
      }
      
      if (verl) {
	 if (!*verl[i]) {
	    Op = pkgCache::Dep::NoOp;
	 } else {
	    if (flagl[i] & RPMSENSE_LESS) {
	       if (flagl[i] & RPMSENSE_EQUAL) {
		  Op = pkgCache::Dep::LessEq;
	       } else {
		  Op = pkgCache::Dep::Less;	
	       }
	    } else if (flagl[i] & RPMSENSE_GREATER) {
	       if (flagl[i] & RPMSENSE_EQUAL) {
		  Op = pkgCache::Dep::GreaterEq;
	       } else {
		  Op = pkgCache::Dep::Greater;
	       }
	    } else if (flagl[i] & RPMSENSE_EQUAL) {
	       Op = pkgCache::Dep::Equals;
	    }
	 }
	 	 
	 if (NewDepends(Ver,string(namel[i]),string(verl[i]),Op,Type) == false)
	     return false;
      } else {
	 if (NewDepends(Ver,string(namel[i]),string(),pkgCache::Dep::NoOp,
			Type) == false)
	     return false;
      }
   }
   return true;
}
/*}}}*/


// ListParser::ParseDepends - Parse a dependency list			/*{{{*/
// ---------------------------------------------------------------------
/* This is the higher level depends parser. It takes a tag and generates
 a complete depends tree for the given version. */
bool rpmListParser::ParseDepends(pkgCache::VerIterator Ver,
				 unsigned int Type)
{
   char **namel = NULL;
   char **verl = NULL;
   int *flagl = NULL;
   int res, type, count;
   
   switch (Type) {
    case pkgCache::Dep::Depends:
    case pkgCache::Dep::PreDepends:
      res = headerGetEntry(header, RPMTAG_REQUIRENAME, &type, 
			   (void **)&namel, &count);
      if (res != 1)
	  return true;
      res = headerGetEntry(header, RPMTAG_REQUIREVERSION, &type, 
			   (void **)&verl, &count);
      res = headerGetEntry(header, RPMTAG_REQUIREFLAGS, &type,
			   (void **)&flagl, &count);
      break;
      
    case pkgCache::Dep::Obsoletes:
      res = headerGetEntry(header, RPMTAG_OBSOLETENAME, &type,
			   (void **)&namel, &count);
      if (res != 1)
	  return true;
      res = headerGetEntry(header, RPMTAG_OBSOLETEVERSION, &type,
			   (void **)&verl, &count);
      res = headerGetEntry(header, RPMTAG_OBSOLETEFLAGS, &type,
			   (void **)&flagl, &count);      
      break;

    case pkgCache::Dep::Conflicts:
      res = headerGetEntry(header, RPMTAG_CONFLICTNAME, &type, 
			   (void **)&namel, &count);
      if (res != 1)
	  return true;
      res = headerGetEntry(header, RPMTAG_CONFLICTVERSION, &type, 
			   (void **)&verl, &count);
      res = headerGetEntry(header, RPMTAG_CONFLICTFLAGS, &type,
			   (void **)&flagl, &count);
      break;
            
    default:
      cout << "not implemented!!!\n";
      abort();
   }
   
   ParseDepends(Ver, namel, verl, flagl, count, Type);
   
//   free(namel);
//   if (verl) free(verl);
   
   return true;
}
/*}}}*/



bool rpmListParser::ProcessFileProvides(pkgCache::VerIterator Ver)
{
   const char **names = NULL;    
   int count = 0;
   
   rpmBuildFileList(header, &names, &count);
   
#if 1
   while (count--) {
      /* if some pkg depends on the file, add it to the provides list */
      if (filedeps->find(string(names[count])) != filedeps->end()) {
	 if (!NewProvides(Ver, string(names[count]), string())) {
//	     free(names);
	     return false;
	 }
      }
   }
#else
   for (map<string,int>::iterator i = filedeps->begin(); 
	i!=filedeps->end(); 
	i++) {
      const char *file = (*i).first.c_str();
      
      if (bsearch(file, names, count, sizeof(char*), 
		  (int (*)(const void *, const void *))strcmp)) {
	 if (!NewProvides(Ver, string(file), string()))
	     return false;
      }
   }
#endif

//   free(names);

   return true;
}

// ListParser::ParseProvides - Parse the provides list			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListParser::ParseProvides(pkgCache::VerIterator Ver)
{
   int type, count;
   char **namel = NULL;
   char **verl = NULL;
   int res;
   bool ok = true;

#ifdef DUPPACK
   if (duplicated) {
      char *name;
      headerGetEntry(header, RPMTAG_NAME, &type, (void **)&name, &count);
      NewProvides(Ver, string(name), Version());
   }
#endif

   res = headerGetEntry(header, RPMTAG_PROVIDENAME, &type,
			(void **)&namel, &count);
   if (res != 1)
       return true;
   /*
    res = headerGetEntry(header, RPMTAG_PROVIDEFLAGS, &type,
    (void **)&flagl, &count);
    if (res != 1)
    return true;
    */
   res = headerGetEntry(header, RPMTAG_PROVIDEVERSION, &type, 
			(void **)&verl, NULL);
   if (res != 1)
	verl = NULL;

   for (int i = 0; i < count; i++) {
      if (verl && *verl[i]) {
	 if (NewProvides(Ver,string(namel[i]),string(verl[i])) == false) {
	    ok = false;
	    break;
	 }
      } else {
	 if (NewProvides(Ver,string(namel[i]),string()) == false) {
	    ok = false;
	    break;
	 }
      }
   }
    
   return ok;
}
/*}}}*/

Header rpmListParser::NextHeader()
{
   Header h;
   
   if (parsing_hdlist) {
      FD_t fdt = fdDup(file->Fd());
      offset = file->Tell();       
      h = headerRead(fdt, HEADER_MAGIC_YES);
      fdClose(fdt);
   } else {
      unsigned bla1, bla2;
      h = pkgRpmLock::SharedRPM()->NextHeader();
      pkgRpmLock::SharedRPM()->Offset(bla1, bla2);
      offset = bla2;
   }
   
   return h;
}


bool rpmListParser::ShouldBeIgnored(string pkg)
{
   // check whether this package must be ignored
   // usually because user installed it with --nodeps
   // use with moderation

   const Configuration::Item *Top = _config->Tree("RPM::IgnorePkgs");
   
   for (Top = (Top == 0?0:Top->Child); Top != 0; Top = Top->Next)
   {
      if (Top->Value == pkg)
	  return true;
   }
   
   return false;
}

// ListParser::Step - Move to the next section in the file		/*{{{*/
// ---------------------------------------------------------------------
/* This has to be carefull to only process the correct architecture */
bool rpmListParser::Step()
{
   Header tmp;
   
   while ((tmp = NextHeader()) != NULL)
   {
      /* See if this is the correct Architecture, if it isn't then we
       drop the whole section. A missing arch tag can't happen to us */
      int type;
      string arch;
      int count;
      bool res;
      bool archOk = false;
     
      if (header)
	  headerFree(header);
      header = tmp;

      arch = Architecture();

      string pkg = Package();
      if (!duplicated) {
	  pkg = pkg+'#'+Version();
      }
       
      if (ShouldBeIgnored(string(pkg.c_str(),pkg.find('#'))) == true)
	  continue;
      
      if (multiarchs->find(pkg) != multiarchs->end()) {
	 if (arch == (*multiarchs)[pkg]) {
	     archOk = true;
	 }
      } else {
	 if (rpmMachineScore(RPM_MACHTABLE_INSTARCH, arch.c_str()) > 0)
	     archOk = true;
	 else
	     archOk = false;
      }
      
      if (!parsing_hdlist || archOk) {
	  return true;
      }
   }

   if (header)
       headerFree(header);
   header = NULL;
   
   return false;
}
/*}}}*/
// ListParser::LoadReleaseInfo - Load the release information		/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmListParser::LoadReleaseInfo(pkgCache::PkgFileIterator FileI,
				    FileFd &File)
{
   pkgTagFile Tags(File);
   pkgTagSection Section;
   if (!Tags.Step(Section))
       return false;
   
   const char *Start;
   const char *Stop;
   if (Section.Find("Archive",Start,Stop))
       FileI->Archive = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Component",Start,Stop))
       FileI->Component = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Version",Start,Stop))
       FileI->Version = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Origin",Start,Stop))
       FileI->Origin = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Label",Start,Stop))
       FileI->Label = WriteUniqString(Start,Stop - Start);
   if (Section.Find("Architecture",Start,Stop))
       FileI->Architecture = WriteUniqString(Start,Stop - Start);
   
   if (Section.FindFlag("NotAutomatic",FileI->Flags,
			pkgCache::Flag::NotAutomatic) == false)
       _error->Warning("Bad NotAutomatic flag");
   
   return !_error->PendingError();
}
/*}}}*/


unsigned long rpmListParser::Size() 
{
   uint_32 *size;
   int type, count;
      
   if (headerGetEntry(header, RPMTAG_SIZE, &type, (void **)&size, &count)!=1)
       return 1;
      
   return size[0]/1024;
}
