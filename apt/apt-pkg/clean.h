// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: clean.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Clean - Clean out downloaded directories
   
   ##################################################################### */
									/*}}}*/
#ifndef APTPKG_CLEAN_H
#define APTPKG_CLEAN_H

#ifdef __GNUG__
#pragma interface "apt-pkg/clean.h"
#endif 

#include <apt-pkg/pkgcache.h>

class pkgArchiveCleaner
{
   protected:
   
   virtual void Erase(const char * /*File*/,string /*Pkg*/,string /*Ver*/,struct stat & /*St*/) {};

   public:   
   
   bool Go(string Dir,pkgCache &Cache);
};

#endif
