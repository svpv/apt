/*
 * $Id: rpminit.cc,v 1.32 2001/11/13 17:32:08 kojima Exp $
 * Copyright (c) 2000 Conectiva S/A
 * 
 * Author: Alfredo K. Kojima <kojima@conectiva.com>
 *  Hacks: Gustavo Niemeyer <niemeyer@conectiva.com>
 */

#include <config.h>

#include <fcntl.h>
#include <utime.h>

#include <unistd.h>

#include <apt-pkg/configuration.h>
#include <apt-pkg/error.h>
#include <apt-pkg/rpminit.h>

#include <i18n.h>

static pkgRpmLock *global_me = NULL;


#define Rpmiter *(rpmdbMatchIterator*)rpmiter


pkgRpmLock::pkgRpmLock(bool exclusive)
{
    struct stat stbuf;

#ifndef HAVE_RPM4
    string rpmdb_path = pkgRpmLock::RPMDBPath() + "packages.rpm";
    stat(rpmdb_path.c_str(), &stbuf);
    size = stbuf.st_size;
#endif
    
    rpmiter = 0;
    offset = 0;
    global_me = this;
    GetLock(exclusive);
}

pkgRpmLock::~pkgRpmLock()
{
#ifdef HAVE_RPM4    
    if (rpmiter)
	delete (rpmdbMatchIterator*)rpmiter;
#endif
}



pkgRpmLock *pkgRpmLock::SharedRPM()
{   
    return global_me;
}

string pkgRpmLock::RPMDBPath()
{
   string root;
    
   root = _config->FindDir("RPM::DBPath", "");
   if (root.empty() || root == "/") {
       root = _config->FindDir("RPM::RootDir", "/");

       return string(root+"var/lib/rpm/");
   } else {
       return root;
   }
}


/*
 * Well, this actually does more than just locking the rpm db.
 */
bool pkgRpmLock::GetLock(bool exclusive)
{
    rpmReadConfigFiles(NULL, NULL);
   
    string root = _config->Find("RPM::RootDir");
    const char *rootdir = NULL;
    
    if (!root.empty())
	rootdir = root.c_str();

   // we use O_RDWR although we won't write anything on the DB,
   // (the spawned rpm will) so that we effectively get an exclusive 
   // lock on it. That's needed to avoid anyone from running rpm
   // at all while we're running.
    if (rpmdbOpen(rootdir, &db, exclusive ? O_RDWR : O_RDONLY, 0644) != 0) {
	_error->Error(_("could not open RPM database:%s"), rpmErrorString());
	if (getuid() != 0)
	    _error->Error(_("You need to run it as the root user."));
	return false;
    }
#ifdef HAVE_RPM4
    rpmiter = new rpmdbMatchIterator;
    
    Rpmiter = rpmdbInitIterator(db, RPMDBI_PACKAGES, NULL, 0);
    if (!Rpmiter) {
	_error->Error(_("could not create RPM database iterator:%s"), rpmErrorString());
	return false;
    }

    while (rpmdbNextIterator(Rpmiter));
    size = rpmdbGetIteratorOffset(Rpmiter);
    
    rpmdbFreeIterator(Rpmiter);
    Rpmiter = rpmdbInitIterator(db, RPMDBI_PACKAGES, NULL, 0);

#endif
    return true;
}


void pkgRpmLock::Close()
{
#ifdef HAVE_RPM4
    if (Rpmiter)
	rpmdbFreeIterator(Rpmiter);
#endif
    rpmdbClose(db);
}


void pkgRpmLock::Rewind()
{
    offset = 0;
#ifdef HAVE_RPM4
    rpmdbFreeIterator(Rpmiter);
    Rpmiter = rpmdbInitIterator(db, RPMDBI_PACKAGES, NULL, 0);
#endif
}


Header pkgRpmLock::NextHeader()
{
#ifdef HAVE_RPM4
    Header h;
    if ((h = rpmdbNextIterator(Rpmiter))) {
	h = headerLink(h);
    }
    offset = rpmdbGetIteratorOffset(Rpmiter);
    return h;
#else
    if (offset == 0) {
	offset = rpmdbFirstRecNum(db);
    } else {
	offset = rpmdbNextRecNum(db, offset);
    }
    if (offset==0)
	return NULL;
    
    return rpmdbGetRecord(db, offset);
#endif
}



Header pkgRpmLock::GetRecord(unsigned offset)
{
#ifdef HAVE_RPM4
    Header h = NULL;
    rpmdbMatchIterator i;
    if (offset == 0) {
	i = rpmdbInitIterator(db, RPMDBI_PACKAGES, NULL, 0);
    } else {
	i = rpmdbInitIterator(db, RPMDBI_PACKAGES, &offset, sizeof(offset));
    }
    if ((h = rpmdbNextIterator(i))) {
	h = headerLink(h);
    }
    rpmdbFreeIterator(i);
    return h;
#else
    return rpmdbGetRecord(db, offset);
#endif
}
