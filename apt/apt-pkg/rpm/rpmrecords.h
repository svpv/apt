// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmrecords.h,v 1.3 2001/06/05 17:14:24 kojima Exp $
/* ######################################################################
   
   RPM Package Records - Parser for RPM hdlist/rpmdb files
   
   This provides display-type parsing for the Packages file. This is 
   different than the the list parser which provides cache generation
   services. There should be no overlap between these two.
   
   ##################################################################### */
									/*}}}*/
// Header section: pkglib
#ifndef PKGLIB_RPMRECORDS_H
#define PKGLIB_RPMRECORDS_H

#ifdef __GNUG__
#pragma interface "apt-pkg/rpmrecords.h"
#endif 


#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/fileutl.h>
#include <rpm/rpmlib.h>

class rpmRecordParser : public pkgRecords::Parser
{
   FileFd *Fd;
   unsigned Offset;

   Header header;
    
   protected:
   
   virtual bool Jump(pkgCache::VerFileIterator const &Ver);
   
   public:

   // These refer to the archive file for the Version
   virtual string FileName();
   virtual string MD5Hash();
   virtual string SourcePkg();
   
   // These are some general stats about the package
   virtual string Maintainer();
   virtual string ShortDesc();
   virtual string LongDesc();

   inline Header GetRecord() { return header; };
   
   rpmRecordParser(string File,pkgCache &Cache);
   ~rpmRecordParser();
};


#endif
