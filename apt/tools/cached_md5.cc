/*
 * $Id: cached_md5.cc,v 1.2 2001/11/13 17:32:08 kojima Exp $
 *
 * $Log: cached_md5.cc,v $
 * Revision 1.2  2001/11/13 17:32:08  kojima
 * Patches from Dmitry Levin <ldv@alt-linux.org>
 * apt-0.3.19cnc52-configure.patch -- patch for configure to add Russian
 *                                    translation and better support of
 *                                    RPM4's db3 usage.
 * apt-0.3.19cnc52-i18n.patch      -- i18n patch. All APT messages now can be
 *                                    localized.
 * apt-0.3.19cnc52-replace-support.patch
 *                                 -- support for Replace option. It is
 *                                 better detection whether package is truly
 *                                 removed or is going to be replaced by
 *                                 package with different name.
 * Patch from Ivan Zakharyashev:
 * apt-cdrom.newfix_imz.patch      -- Fixes apt-cdrom to better manage
 *                                    repository search when CD has both RPMS
 *                                    and SRPMS.
 * Alexander Bokovoy <ab@avilink.net>:
 * apt-gpg-pubring.patch           -- uses --homedir instead of --keyring
 *                                    option to GPG. It is generally better
 *                                    because you also can specify GPG
 *                                    options in
 *                                    ${Apt::GPG::PubringPath}/options
 * apt-ru.po                       -- updated Russian translation for APT
 *                                    (Dmitry Levin and me).
 *
 * Revision 1.1  2001/08/07 20:46:03  kojima
 * Alexander Bokovoy <a.bokovoy@sam-solutions.net>'s patch for cleaning
 * up genpkglist
 *
 *
 */
#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <rpm/rpmlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "cached_md5.h"

#include <apt-pkg/error.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/rpminit.h>
#include <apt-pkg/configuration.h>

// from rpmlib
extern  "C" {
   extern int mdfile( const char *fn, unsigned char *digest );
};

extern const char *__progname;

CachedMD5::CachedMD5( string DirName )
{
	string fname = DirName;
	for ( string::iterator i = fname.begin(); i != fname.end(); ++i )
		if ( '/' == *i )
			*i = '_';

	filename = _config->FindDir( "Dir::Cache", "/var/cache/apt" ) + '/' +
	__progname + '/' +
	fname +	".md5cache";

    FILE *f = fopen( filename.c_str(), "r" );
    if (!f) {
	return;
    }

    while (1) {
	string file;
	FileData data;
	char buf[BUFSIZ];

	if ( !fgets(buf, sizeof(buf), f) )
	    break;
	
	char *p = strchr( buf, ' ' );
	assert(p);

	file = string( buf, p++ );
	
	char *pp = strchr( p, ' ' );
	assert(p);
	data.md5 = string( p, pp++ );
	
	data.timestamp = atol( pp );
	
	md5table[file] = data;
    }

    fclose(f);
}


CachedMD5::~CachedMD5()
{
	FILE *f = fopen( filename.c_str(), "w+" );
	if (!f)
	{
		// probably running as != root and not a real problem
		
		//cerr << __progname << ": could not open file " <<
		//filename << " for writing:" << strerror(errno) << endl;
	} else
	{
		for ( map<string,FileData>::const_iterator iter = md5table.begin();
		iter != md5table.end();
		iter++ )
		{
			string file = (*iter).first;
			const FileData &data = (*iter).second;
	
			fprintf( f, "%s %s %lu\n",
				file.c_str(),
				data.md5.c_str(),
				data.timestamp );
		}
		fclose(f);
	}
}


void CachedMD5::MD5ForFile( string FileName, time_t timestamp,
	unsigned char *buf)
{
	if ( md5table.find(FileName) != md5table.end()
	     && timestamp == md5table[FileName].timestamp )
	{
		strcpy( (char*)buf, md5table[FileName].md5.c_str() );
		return;
	}

	FileData data;

	mdfile( FileName.c_str(), buf );
	data.md5 = string( (char*)buf );
	data.timestamp = timestamp;

	md5table[FileName] = data;
}
