// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmsrcrecords.cc,v 1.5 2001/11/13 17:32:08 kojima Exp $
/* ######################################################################
   
   SRPM Records - Parser implementation for RPM style source indexes
      
   ##################################################################### 
 */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmsrcrecords.h"
#endif 

#include <apt-pkg/rpmsrcrecords.h>
#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/rpminit.h>									/*}}}*/

#include <i18n.h>

rpmSrcRecordParser::rpmSrcRecordParser(FileFd *F, pkgSourceList::const_iterator SrcItem)
    : Parser(F, SrcItem)
{
}





// SrcRecordParser::Binaries - Return the binaries field		/*{{{*/
// ---------------------------------------------------------------------
/* This member parses the binaries field into a pair of class arrays and
   returns a list of strings representing all of the components of the
   binaries field. The returned array need not be freed and will be
   reused by the next Binaries function call. */
const char **rpmSrcRecordParser::Binaries()
{
   int i = 0;
   char **bins;
   int type, count;

   if (headerGetEntry(header, CRPMTAG_BINARY, &type, (void**)&bins, &count)!=1)
       return NULL;
   for (i = 0; (unsigned)i < sizeof(StaticBinList)/sizeof(char*) && i < count; i++)
   {
      StaticBinList[i] = bins[i];
   }
   StaticBinList[i] = 0;
//   free(bins);
   
   return StaticBinList;
}
									/*}}}*/
// SrcRecordParser::Files - Return a list of files for this source	/*{{{*/
// ---------------------------------------------------------------------
/* This parses the list of files and returns it, each file is required to have
   a complete source package */
bool rpmSrcRecordParser::Files(vector<pkgSrcRecords::File> &List)
{
   char *srpm, *md5;
   char *dir;
   int type, count;
   int_32 *size;
    
   List.erase(List.begin(),List.end());
   
   if (headerGetEntry(header, CRPMTAG_FILENAME, &type, (void**)&srpm, &count)==0)
       return false;

   if (headerGetEntry(header, CRPMTAG_MD5, &type, (void**)&md5, &count)==0)
   {
      return _error->Error(_("error parsing file record"));
      return false;
   }

   if (headerGetEntry(header, CRPMTAG_FILESIZE, &type, (void**)&size, &count)==0)
   {
      return _error->Error(_("error parsing file record"));
      return false;
   }

   if (headerGetEntry(header, CRPMTAG_DIRECTORY, &type, (void**)&dir, &count)==0)
   {
      return _error->Error(_("error parsing file record"));
      return false;
   }

   pkgSrcRecords::File F;

   F.MD5Hash = string(md5);
   F.Size = size[0];
   F.Path = string(dir)+"/"+string(srpm);
   
   List.push_back(F);
   
   return true;
}
									/*}}}*/


bool rpmSrcRecordParser::Restart()
{
    iOffset = 0;
   return true;
}


bool rpmSrcRecordParser::Step() 
{
    FD_t fdt = fdDup(File->Fd());
    iOffset = File->Tell();
    header = headerRead(fdt, HEADER_MAGIC_YES);
    fdClose(fdt);
    
    if (header != NULL)
	return true;
    else
	return false;
}


bool rpmSrcRecordParser::Jump(unsigned long Off)
{
    iOffset = Off;
    
    File->Seek(iOffset);
    FD_t fdt = fdDup(File->Fd());
    header = headerRead(fdt, HEADER_MAGIC_YES);
    fdClose(fdt);
    
    if (header != NULL)
	return true;
    else
	return false;
}

string rpmSrcRecordParser::Package() 
{
   char *str;
   int_32 count, type;

   headerGetEntry(header, RPMTAG_NAME, &type, (void**)&str, &count);
    
   string tmp = string(str);
//   free(str);

   return tmp;
}


string rpmSrcRecordParser::Version() 
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
    

// RecordParser::Maintainer - Return the maintainer email		/*{{{*/
// ---------------------------------------------------------------------
/* */
string rpmSrcRecordParser::Maintainer()
{
   char *str;
   int_32 count, type;

   headerGetEntry(header, RPMTAG_PACKAGER, &type, (void**)&str, &count);
    
   string tmp = string(str);

   return tmp;
}
									/*}}}*/


string rpmSrcRecordParser::Section() 
{
   char *str;
   int_32 count, type;

   headerGetEntry(header, RPMTAG_GROUP, &type, (void**)&str, &count);
    
   string tmp = string(str);

   return tmp;
}

unsigned long rpmSrcRecordParser::Offset() 
{
    return iOffset;
}

string rpmSrcRecordParser::AsStr()
{
    return "NOT IMPLEMENTED!!!\n\n";
}

