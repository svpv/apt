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

#include <apt-pkg/rpmrecords.h>
#include <apt-pkg/error.h>
#include <apt-pkg/rpminit.h>

#include <rpm/rpmmacro.h>

// RecordParser::rpmRecordParser - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
rpmRecordParser::rpmRecordParser(string File, pkgCache &Cache)
{
    string RPMFile = "packages.rpm";
#ifdef HAVE_RPM4 
    if (rpmExpandNumeric("%{_dbapi}") == 3)
    	RPMFile = "Packages";
#endif

    if (File.find(RPMFile) != string::npos) {
	// read from the rpm db
	Fd = NULL;
	pkgRpmLock::SharedRPM()->Rewind();
    } else {
	Fd = new FileFd(File, FileFd::ReadOnly);
    }
    Offset = 0;
    header = NULL;
}
									/*}}}*/
rpmRecordParser::~rpmRecordParser()
{
    if (Fd) {
	delete Fd;
    }
    if (header)
	headerFree(header);    
}
// RecordParser::Jump - Jump to a specific record			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool rpmRecordParser::Jump(pkgCache::VerFileIterator const &Ver)
{
    if (header)
	headerFree(header);

    if (Fd) {
	Offset = Ver->Offset;
	Fd->Seek(Offset);
	FD_t fdt = fdDup(Fd->Fd());
	header = headerRead(fdt, HEADER_MAGIC_YES);
	fdClose(fdt);
    } else {
	Offset = Ver->Offset;
	header = pkgRpmLock::SharedRPM()->GetRecord(Offset);
    }
    
    if (header != NULL)
	return true;
    else
	return false;
}
									/*}}}*/
// RecordParser::FileName - Return the archive filename on the site	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::FileName()
{
   char *str;
   int_32 count, type;
    
   headerGetEntry(header, CRPMTAG_FILENAME, &type, (void**)&str, &count);
    
   string tmp = string(str);
   
   return tmp;
}
									/*}}}*/
// RecordParser::MD5Hash - Return the archive hash			/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::MD5Hash()
{
   char *str;
   int_32 count, type;
    
   if (headerGetEntry(header, CRPMTAG_MD5, &type, (void**)&str, &count)!=1) {
      return string();
   } else {
      return string(str);
   }
}
									/*}}}*/
// RecordParser::Maintainer - Return the maintainer email		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::Maintainer()
{
   char *str = NULL;
   int_32 count, type;

   headerGetEntry(header, RPMTAG_PACKAGER, &type, (void**)&str, &count);
    
   if (!str)
	return "";
    
   string tmp = string(str);
   return tmp;
}
									/*}}}*/
// RecordParser::ShortDesc - Return a 1 line description		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::ShortDesc()
{
    string Res;
    char *str = NULL;
    int_32 count, type;
	
    headerGetEntry(header, RPMTAG_SUMMARY, &type, (void**)&str, &count);
    
    if (!str)
	return "";
    
    Res = string(str);
    string::size_type Pos = Res.find('\n');
    if (Pos == string::npos)
      return Res;
    return string(Res,0,Pos);
}
									/*}}}*/
// RecordParser::LongDesc - Return a longer description			/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::LongDesc()
{
   char *str = NULL, *str2, *x, *y;
   int_32 count, type;
   int n, s;
	
   headerGetEntry(header, RPMTAG_DESCRIPTION, &type, (void**)&str, &count);

   if (!str)
	return "";
    
   // Ouch!
   for (x = str, s = n = 0; *x; x++, s++) {
	if (*x == '\n')
		n++;
   }
   str2 = (char *)malloc (s + n + 4);
   for (x = str, y = str2; *x; x++, y++) {
	*y = *x;
	if (*x == '\n')
		*++y = ' ';
   }
   *y = 0;
   for (y--; (*y == ' ' || *y == '\n') && y > str2; y--)
	*y = 0;
    
   string tmp = string(str2);

   free(str2);

   return tmp;
}
									/*}}}*/
// RecordParser::SourcePkg - Return the source package name if any	/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmRecordParser::SourcePkg()
{
    char *str;
    int_32 count, type;
	
    if (headerGetEntry(header, RPMTAG_SOURCERPM, &type, (void**)&str, &count)==0) {
       string tmp = string(str);
       return tmp;
    } else
	return string();
}
									/*}}}*/
