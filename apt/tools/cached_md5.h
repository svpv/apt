/*
 * $Id: cached_md5.h,v 1.1 2001/08/07 20:46:03 kojima Exp $
 *
 * $Log: cached_md5.h,v $
 * Revision 1.1  2001/08/07 20:46:03  kojima
 * Alexander Bokovoy <a.bokovoy@sam-solutions.net>'s patch for cleaning
 * up genpkglist
 *
 *
 */

#ifndef	__CACHED_MD5_H__
#define	__CACHED_MD5_H__

#include <sys/types.h>
#include <string>
#include <map>

class CachedMD5
{
	string filename;

	struct FileData
	{
		string md5;
		time_t timestamp;
	};

	map<string, FileData> md5table;

public:
	void MD5ForFile( string FileName, time_t timestamp, unsigned char *buf );

	CachedMD5( string DirName );
	~CachedMD5();
};

#endif	/* __CACHED_MD5_H__ */
