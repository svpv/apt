// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: pkgrecords.cc,v 1.5 2001/01/11 02:03:27 kojima Exp $
/* ######################################################################
   
   Package Records - Allows access to complete package description records
                     directly from the file.
     
   ##################################################################### 
 */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/pkgrecords.h"
#endif
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/systemfactory.h>

#include <i18n.h>
									/*}}}*/

// Records::pkgRecords - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* This will create the necessary structures to access the status files */
pkgRecords::pkgRecords(pkgCache &Cache) : Cache(Cache), Files(0)
{
   Files = new PkgFile[Cache.HeaderP->PackageFileCount];
   for (pkgCache::PkgFileIterator I = Cache.FileBegin(); 
	I.end() == false; I++)
   {
      // We can not initialize if the cache is out of sync.
      if (I.IsOk() == false)
      {
	 _error->Error(_("Package file %s is out of sync."),I.FileName());
	 return;
      }
   
      // Create the file
      Files[I->ID].File = string(I.FileName());
      
      // Create the parser
      Files[I->ID].Parse = _system->CreateRecordParser(Files[I->ID].File,Cache);
      if (_error->PendingError() == true) {
	 if (Files[I->ID].Parse)
	      delete Files[I->ID].Parse;
	 return;
      }
   }   
}
									/*}}}*/
// Records::~pkgRecords - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRecords::~pkgRecords()
{
   delete [] Files;
}
									/*}}}*/
// Records::Lookup - Get a parser for the package version file		/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRecords::Parser &pkgRecords::Lookup(pkgCache::VerFileIterator const &Ver)
{
   PkgFile &File = Files[Ver.File()->ID];
   File.Parse->Jump(Ver);

   return *File.Parse;
}
									/*}}}*/
// Records::Pkgfile::~PkgFile - Destructor				/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgRecords::PkgFile::~PkgFile()
{
   delete Parse;
}
									/*}}}*/
