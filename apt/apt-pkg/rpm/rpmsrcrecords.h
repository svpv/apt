// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: rpmsrcrecords.h,v 1.1 2000/10/29 20:25:46 kojima Exp $
/* ######################################################################
   
   SRPM Records - Parser implementation for RPM style source indexes
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_RPMSRCRECORDS_H
#define PKGLIB_RPMSRCRECORDS_H

#ifdef __GNUG__
#pragma interface "apt-pkg/rpmsrcrecords.h"
#endif 

#include <apt-pkg/srcrecords.h>
#include <apt-pkg/fileutl.h>
#include <rpm/rpmlib.h>

class rpmSrcRecordParser : public pkgSrcRecords::Parser
{
   long iOffset;
   Header header;
   const char *StaticBinList[400];
   
public:
   virtual bool Restart();
   virtual bool Step(); 
   virtual bool Jump(unsigned long Off);

   virtual string Package();
   virtual string Version();
   virtual string Maintainer();
   virtual string Section();
   virtual const char **Binaries();
   virtual unsigned long Offset();
   virtual string AsStr();
   virtual bool Files(vector<pkgSrcRecords::File> &F);
   
   rpmSrcRecordParser(FileFd *F,pkgSourceList::const_iterator SrcItem);
};

#endif
