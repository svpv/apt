// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: sourcelist.h,v 1.9 2001/11/12 16:34:00 kojima Exp $
/* ######################################################################

   SourceList - Manage a list of sources
   
   The Source List class provides access to a list of sources. It 
   can read them from a file and generate a list of all the distinct
   sources.
   
   All sources have a type associated with them that defines the layout
   of the archive. The exact format of the file is documented in
   files.sgml.

   ##################################################################### 
 */
									/*}}}*/
// Header section: pkglib
#ifndef PKGLIB_SOURCELIST_H
#define PKGLIB_SOURCELIST_H

#include <string>
#include <vector>
#include <map>
#include <iostream.h>
#include <apt-pkg/pkgcache.h>

#ifdef __GNUG__
#pragma interface "apt-pkg/sourcelist.h"
#endif

class pkgAquire;
class pkgSourceList
{
   public:

   struct Item;

   struct VendorItem 
   {
      string VendorID;
      string Fingerprint;
      string Name;
   };
   
   enum RepositoryType {Deb, DebSrc, Rpm, RpmSrc};

   struct FileData 
   {
       string md5_hash;
       unsigned int size;
   };
    
   struct RepositoryItem 
   {
      RepositoryType Type;
      map<string,FileData> HashIndex; // filename -> filedata

      VendorItem *Vendor;

      string URI;
      string Dist;
      
      bool SetType(string S);
      bool SetURI(string S);
      bool SetVendor(vector<VendorItem*> vendors, string S);
      
      bool AuthSucceeded() { HashIndex.size() > 0; };
       
      bool MD5HashForFile(string file, string &hash, unsigned int &size);
      
      string SiteOnly(string URI);

      string HashesURI();
      string HashesInfo();

      bool UpdateHashes(string File);
            
      bool ParseHashData(string str, string &hash, unsigned long &size, 
			 string &file);
   };
   typedef vector<RepositoryItem*>::iterator rep_iterator;
   

   /* Each item in the source list, each line can have more than one
      item */
   struct Item
   {
      RepositoryItem *Repository;
      string Section;

      inline RepositoryType Type() const { return Repository->Type; };
      inline string URI() const { return Repository->URI; };
      string Dist;
      
      string PackagesURI(bool PathOnly = false) const;
      string PackagesInfo() const;

      string ReleaseURI(bool PathOnly = false) const;
      string ReleaseInfo() const;
      
      string SourceInfo(string Pkg,string Ver,string Comp) const;

      string ArchiveInfo(pkgCache::VerIterator Ver) const;
      string ArchiveURI(string File) const;
   };
   typedef vector<Item>::const_iterator const_iterator;

   
 protected:

   vector<RepositoryItem*> Repositories;
   vector<VendorItem*> Vendors;
   vector<Item> List;
   
 public:

   bool ReadMainList();
   bool Read(string File);
   
   bool ReadVendors(string File);

   // Repository accessors
   inline rep_iterator rep_begin() {return Repositories.begin();};
   inline rep_iterator rep_end() {return Repositories.end();};

   // List accessors
   inline const_iterator begin() const {return List.begin();};
   inline const_iterator end() const {return List.end();};
   inline unsigned int size() const {return List.size();};
   inline bool empty() const {return List.empty();};
   
   pkgSourceList();
//   pkgSourceList(string File);   
};

ostream &operator <<(ostream &O,pkgSourceList::Item &Itm);

#endif
