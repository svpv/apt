/*
 * Copyright (c) 2000 Conectiva S/A
 * 
 * Author: Alfredo K. Kojima <kojima@conectiva.com>
 */

#ifndef _RPMINIT_H_
#define _RPMINIT_H_

#include <config.h>

#include <rpm/rpmlib.h>

#include <string>


class pkgRpmLock
{
   rpmdb db;
   unsigned int size;
   unsigned int offset;
   
   void *rpmiter; // only used in rpm4
   
 public:
   bool GetLock(bool exclusive);
   void Close();

   static pkgRpmLock *SharedRPM();
   
   static string RPMDBPath();
   
   void Rewind();
   Header NextHeader();
   Header GetRecord(unsigned offset);
   inline void Offset(unsigned &total, unsigned &current)
   {
      total = size;
      current = offset;
   }
   
   pkgRpmLock(bool exclusive);
   ~pkgRpmLock();
};

#define CRPMTAG_FILENAME 1000000
#define CRPMTAG_FILESIZE 1000001
#define CRPMTAG_MD5      1000005

#define CRPMTAG_DIRECTORY 1000010
#define CRPMTAG_BINARY    1000011

#define CRPMTAG_UPDATE_SUMMARY    1000020
#define CRPMTAG_UPDATE_IMPORTANCE 1000021
#define CRPMTAG_UPDATE_DATE       1000022
#define CRPMTAG_UPDATE_URL        1000023



#endif
