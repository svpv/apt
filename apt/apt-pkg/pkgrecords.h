// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgrecords.h,v 1.3 2000/10/28 01:48:45 kojima Exp $
/* ######################################################################
   
   Package Records - Allows access to complete package description records
                     directly from the file.
   
   The package record system abstracts the actual parsing of the 
   package files. This is different than the generators parser in that
   it is used to access information not generate information. No 
   information touched by the generator should be parable from here as
   it can always be retreived directly from the cache.
   
   ##################################################################### */
									/*}}}*/
// Header section: pkglib
#ifndef PKGLIB_PKGRECORDS_H
#define PKGLIB_PKGRECORDS_H

#ifdef __GNUG__
#pragma interface "apt-pkg/pkgrecords.h"
#endif 

#include <apt-pkg/pkgcache.h>
#include <apt-pkg/fileutl.h>

class pkgRecords
{
   public:
   class Parser;
   
   private:
   
   pkgCache &Cache;
   
   // List of package files
   struct PkgFile
   {
       string File;
       Parser *Parse;

       PkgFile() : Parse(0) { File=""; };
       ~PkgFile();
   };
   PkgFile *Files;
   
   public:

   // Lookup function
   Parser &Lookup(pkgCache::VerFileIterator const &Ver);

   // Construct destruct
   pkgRecords(pkgCache &Cache);
   ~pkgRecords();
};

class pkgRecords::Parser
{
   protected:
      
   public:
   friend pkgRecords;
   
   virtual bool Jump(pkgCache::VerFileIterator const &Ver) = 0;

   // These refer to the archive file for the Version
   virtual string FileName() {return string();};
   virtual string MD5Hash() {return string();};
   virtual string SourcePkg() {return string();};
   
   // These are some general stats about the package
   virtual string Maintainer() {return string();};
   virtual string ShortDesc() {return string();};
   virtual string LongDesc() {return string();};

   virtual ~Parser() {};
};

#endif
