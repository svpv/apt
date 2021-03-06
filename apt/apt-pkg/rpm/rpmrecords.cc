// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
/* ######################################################################
   
   RPM Package Records - Parser for RPM package records
     
   ##################################################################### 
 */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmrecords.h"
#endif

#include <config.h>

#ifdef HAVE_RPM

#include <assert.h>

#include <apt-pkg/rpmrecords.h>
#include <apt-pkg/error.h>
#include <apt-pkg/rpmhandler.h>
#include <apt-pkg/rpmsystem.h>

#include <apti18n.h>

#include <cstring>

// RecordParser::rpmRecordParser - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmRecordParser::rpmRecordParser(string File, pkgCache &Cache)
   : Handler(0), HeaderP(0), Buffer(0), BufSize(0), BufUsed(0)
{
   if (File == RPMDBHandler::DataPath(false)) {
      IsDatabase = true;
      Handler = rpmSys.GetDBHandler();
   } else {
      IsDatabase = false;
      struct stat Buf;
      if (stat(File.c_str(),&Buf) == 0 && S_ISDIR(Buf.st_mode))
	 Handler = new RPMDirHandler(File);
      else if (flExtension(File) == "rpm")
	 Handler = new RPMSingleFileHandler(File);
      else
	 Handler = new RPMFileHandler(File);
   }
}
									/*}}}*/
// RecordParser::~rpmRecordParser - Destructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmRecordParser::~rpmRecordParser()
{
   // Can't use Handler->IsDatabase here, since the RPMDBHandler
   // could already have been destroyed.
   if (IsDatabase == false)
      delete Handler;
   free(Buffer);
}
									/*}}}*/
// RecordParser::Jump - Jump to a specific record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmRecordParser::Jump(pkgCache::VerFileIterator const &Ver)
{
   Handler->Jump(Ver->Offset);
   HeaderP = Handler->GetHeader();
   return (HeaderP != NULL);
}
									/*}}}*/
// RecordParser::FileName - Return the archive filename on the site	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::FileName()
{
   string Dir = Handler->Directory();
   if (Dir.empty() == true)
      return Handler->FileName();
   return flCombine(Dir, Handler->FileName());
}
									/*}}}*/
// RecordParser::Name - Return the package name				/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::Name()
{
   char *str;
   rpm_tagtype_t type;
   rpm_count_t count;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_NAME,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}
									/*}}}*/
// RecordParser::MD5Hash - Return the archive hash			/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::MD5Hash()
{
   return Handler->MD5Sum();
}
									/*}}}*/
// RecordParser::Maintainer - Return the maintainer email		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::Maintainer()
{
   char *str;
   rpm_tagtype_t type;
   rpm_count_t count;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_PACKAGER,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}
									/*}}}*/
// RecordParser::ShortDesc - Return a 1 line description		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::ShortDesc()
{
   char *str;
   rpm_tagtype_t type;
   rpm_count_t count;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_SUMMARY,
			   &type, (void**)&str, &count);
   if (rc != 1)
      return string();
   else
      return string(str,0,string(str).find('\n'));
}
									/*}}}*/
// RecordParser::LongDesc - Return a longer description			/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::LongDesc()
{
   char *str, *ret, *x, *y;
   rpm_tagtype_t type;
   rpm_count_t count;
   int len;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_DESCRIPTION,
			   &type, (void**)&str, &count);
   if (rc != 1)
      return "";

   // Count size plus number of newlines
   for (x = str, len = 0; *x; x++, len++)
      if (*x == '\n')
	 len++;
   
   ret = (char*)malloc(len+1);
   if (ret == NULL)
      return "out of mem";

   // Copy string, inserting a space after each newline
   for (x = str, y = ret; *x; x++, y++)
   {
      *y = *x;
      if (*x == '\n')
	 *++y = ' ';
   }
   *y = 0;

   // Remove spaces and newlines from end of string
   for (y--; y > ret && (*y == ' ' || *y == '\n'); y--)
      *y = 0;
   
   string Ret = string(ret);
   free(ret);
   
   return Ret;
}
									/*}}}*/
// RecordParser::Changelog - Return package changelog if any		/*{{{*/
// -----------------------------------------------
string rpmRecordParser::Changelog()
{
   char *str;
   string rval("");

   str = headerSprintf(HeaderP,
         "[* %{CHANGELOGTIME:day} %{CHANGELOGNAME}\n%{CHANGELOGTEXT}\n\n]",
         rpmTagTable, rpmHeaderFormats, NULL);

   if (str && *str) {
	  rval = (const char *)str;
   }
   if (str)
      free(str);

   return rval;
}

									/*}}}*/
// RecordParser::SourcePkg - Return the source package name if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::SourcePkg()
{
   // This must be the *package* name, not the *file* name. We have no
   // current way to extract it safely from the file name.
   char *str;
   rpm_tagtype_t type;
   rpm_count_t count;
   assert(HeaderP != NULL);
   int rc = headerGetEntry(HeaderP, RPMTAG_SOURCERPM,
			   &type, (void**)&str, &count);
   return string(rc?str:"");
}
									/*}}}*/

void rpmRecordParser::BufCat(const char *text)
{
   if (text != NULL)
      BufCat(text, text+strlen(text));
}

void rpmRecordParser::BufCat(const char *begin, const char *end)
{
   unsigned len = end - begin;
    
   while (BufUsed + len + 1 >= BufSize)
   {
      size_t new_size = BufSize + 512;
      char *new_buf = (char*)realloc(Buffer, new_size);
      if (new_buf == NULL)
      {
	 _error->Errno("realloc", _("Could not allocate buffer for record text"));
	 return;
      }
      Buffer = new_buf;
      BufSize = new_size;
   }

   memcpy(Buffer+BufUsed, begin, len);
   BufUsed += len;
   Buffer[BufUsed] = '\0';
}

void rpmRecordParser::BufCatTag(const char *tag, const char *value)
{
   BufCat(tag);
   BufCat(value);
}

void rpmRecordParser::BufCatDep(const char *pkg,
			        const char *version,
				int flags)
{
   char buf[16];
   char *ptr = buf;

   BufCat(pkg);
   if (*version) 
   {
      int c = 0;
      *ptr++ = ' ';
      *ptr++ = '(';
      if (flags & RPMSENSE_LESS)
      {
	 *ptr++ = '<';
	 c = '<';
      }
      if (flags & RPMSENSE_GREATER) 
      {
	 *ptr++ = '>';
	 c = '>';
      }
      if (flags & RPMSENSE_EQUAL) 
      {
	 *ptr++ = '=';
      }/* else {
	 if (c)
	   fputc(c, f);
      }*/
      *ptr++ = ' ';
      *ptr = '\0';

      BufCat(buf);
      BufCat(version);
      BufCat(")");
   }
}

void rpmRecordParser::BufCatDescr(const char *descr)
{
   const char *begin = descr;
   const char *p = descr;

   while (*p)
   {
      if (*p=='\n')
      {
	 BufCat(" ");
	 BufCat(begin, p+1);
	 begin = p+1;
      }
      p++;
   }
   if (*begin) {
      BufCat(" ");
      BufCat(begin, p);
      BufCat("\n");
   }
}


// RecordParser::GetRec - The record in raw text, in std Debian format	/*{{{*/
// ---------------------------------------------------------------------
void rpmRecordParser::GetRec(const char *&Start,const char *&Stop) 
{
   // FIXME: This method is leaking memory from headerGetEntry().
   rpm_tagtype_t type, type2, type3;
   rpm_count_t count;
   char *str;
   char **strv;
   char **strv2;
   int_32 *numv;
   char buf[32];

   BufUsed = 0;

   assert(HeaderP != NULL);
   
   headerGetEntry(HeaderP, RPMTAG_NAME, &type, (void **)&str, &count);
   BufCatTag("Package: ", str);

   headerGetEntry(HeaderP, RPMTAG_GROUP, &type, (void **)&str, &count);
   BufCatTag("\nSection: ", str);

   headerGetEntry(HeaderP, RPMTAG_SIZE, &type, (void **)&numv, &count);
   snprintf(buf, sizeof(buf), "%d", numv[0]);
   BufCatTag("\nInstalled Size: ", buf);

   str = NULL;
   headerGetEntry(HeaderP, RPMTAG_PACKAGER, &type, (void **)&str, &count);
   if (!str)
       headerGetEntry(HeaderP, RPMTAG_VENDOR, &type, (void **)&str, &count);
   BufCatTag("\nMaintainer: ", str);
   
   BufCat("\nVersion: ");
   headerGetEntry(HeaderP, RPMTAG_VERSION, &type, (void **)&str, &count);
   if (headerGetEntry(HeaderP, RPMTAG_EPOCH, &type, (void **)&numv, &count)==1)
       snprintf(buf, sizeof(buf), "%i:%s-", numv[0], str);
   else
       snprintf(buf, sizeof(buf), "%s-", str);
   BufCat(buf);
   headerGetEntry(HeaderP, RPMTAG_RELEASE, &type, (void **)&str, &count);
   BufCat(str);

//   headerGetEntry(HeaderP, RPMTAG_DISTRIBUTION, &type, (void **)&str, &count);
//   fprintf(f, "Distribution: %s\n", str);

   headerGetEntry(HeaderP, RPMTAG_REQUIRENAME, &type, (void **)&strv, &count);
   assert(type == RPM_STRING_ARRAY_TYPE || count == 0);

   headerGetEntry(HeaderP, RPMTAG_REQUIREVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(HeaderP, RPMTAG_REQUIREFLAGS, &type3, (void **)&numv, &count);
   
   if (count > 0)
   {
      int i, j;

      for (j = i = 0; i < count; i++) 
      {
	 if ((numv[i] & RPMSENSE_PREREQ))
	 {
	    if (j == 0) 
		BufCat("\nPre-Depends: ");
	    else
		BufCat(", ");
	    BufCatDep(strv[i], strv2[i], numv[i]);
	    j++;
	 }
      }

      for (j = 0, i = 0; i < count; i++) 
      {
	 if (!(numv[i] & RPMSENSE_PREREQ)) 
	 {
	    if (j == 0)
		BufCat("\nDepends: ");
	    else
		BufCat(", ");
	    BufCatDep(strv[i], strv2[i], numv[i]);
	    j++;
	 }
      }
   }
   
   headerGetEntry(HeaderP, RPMTAG_CONFLICTNAME, &type, (void **)&strv, &count);
   assert(type == RPM_STRING_ARRAY_TYPE || count == 0);

   headerGetEntry(HeaderP, RPMTAG_CONFLICTVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(HeaderP, RPMTAG_CONFLICTFLAGS, &type3, (void **)&numv, &count);
   
   if (count > 0) 
   {
      BufCat("\nConflicts: ");
      for (int i = 0; i < count; i++) 
      {
	 if (i > 0)
	     BufCat(", ");
	 BufCatDep(strv[i], strv2[i], numv[i]);
      }
   }

   headerGetEntry(HeaderP, RPMTAG_PROVIDENAME, &type, (void **)&strv, &count);
   assert(type == RPM_STRING_ARRAY_TYPE || count == 0);

   headerGetEntry(HeaderP, RPMTAG_PROVIDEVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(HeaderP, RPMTAG_PROVIDEFLAGS, &type3, (void **)&numv, &count);
   
   if (count > 0) 
   {
      BufCat("\nProvides: ");
      for (int i = 0; i < count; i++) 
      {
	 if (i > 0)
	     BufCat(", ");
	 BufCatDep(strv[i], strv2[i], numv[i]);
      }
   }

   headerGetEntry(HeaderP, RPMTAG_OBSOLETENAME, &type, (void **)&strv, &count);
   assert(type == RPM_STRING_ARRAY_TYPE || count == 0);

   headerGetEntry(HeaderP, RPMTAG_OBSOLETEVERSION, &type2, (void **)&strv2, &count);
   headerGetEntry(HeaderP, RPMTAG_OBSOLETEFLAGS, &type3, (void **)&numv, &count);
   if (count > 0) {
      BufCat("\nObsoletes: ");
      for (int i = 0; i < count; i++) 
      {
	 if (i > 0)
	     BufCat(", ");
	 BufCatDep(strv[i], strv2[i], numv[i]);
      }
   }

   headerGetEntry(HeaderP, RPMTAG_ARCH, &type, (void **)&str, &count);
   BufCatTag("\nArchitecture: ", str);
   
   snprintf(buf, sizeof(buf), "%lu", Handler->FileSize());
   BufCatTag("\nSize: ", buf);

   BufCatTag("\nMD5Sum: ", Handler->MD5Sum().c_str());

   BufCatTag("\nFilename: ", Handler->FileName().c_str());

   headerGetEntry(HeaderP, RPMTAG_SUMMARY, &type, (void **)&str, &count);
   BufCatTag("\nDescription: ", str);
   BufCat("\n");
   headerGetEntry(HeaderP, RPMTAG_DESCRIPTION, &type, (void **)&str, &count);
   BufCatDescr(str);

   str = headerSprintf(HeaderP,
         "[* %{CHANGELOGTIME:day} %{CHANGELOGNAME}\n%{CHANGELOGTEXT}\n]",
         rpmTagTable, rpmHeaderFormats, NULL);
   if (str && *str) {
      BufCat("Changelog:\n");
      BufCatDescr(str);
   }
   if (str)
      free(str);

   BufCat("\n");
   
   Start = Buffer;
   Stop = Buffer + BufUsed;
}
									/*}}}*/

bool rpmRecordParser::HasFile(const char *File)
{
   bool ret = false;
   const char *FileName;
   int count = 0;

   if (*File == '\0')
      return false;

   rpmtd fileNames = rpmtdNew();

   if (headerGet(HeaderP, RPMTAG_OLDFILENAMES, fileNames, HEADERGET_EXT) != 1 &&
       headerGet(HeaderP, RPMTAG_FILENAMES, fileNames, HEADERGET_EXT) != 1) {
      rpmtdFree(fileNames);
      return ret;
   }
   while ((FileName = rpmtdNextString(fileNames)) != NULL) {
      if (strcmp(File, FileName) == 0) {
         ret = true;
         break;
      }
   }
   rpmtdFreeData(fileNames);
   rpmtdFree(fileNames);
   return ret;
}

#endif /* HAVE_RPM */

// vim:sts=3:sw=3
