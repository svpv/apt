

#ifndef _SYSTEMFACTORY_H_
#define _SYSTEMFACTORY_H_

#include <apt-pkg/pkgcachegen.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/srcrecords.h>

class FileFd;
class pkgPackageManager;
class pkgDepCache;
class pkgCache;
class OpProgress;
class pkgSourceList;
class MMap;

class SystemFactory {
// for cache generation
 protected:
    inline virtual bool addStatusSize(unsigned long &TotalSize) {return false;};
    
    virtual bool sourceCacheCheck(pkgSourceList &List);
    inline virtual bool packageCacheCheck(string CacheFile) = 0;
    
    bool generateSrcCache(pkgSourceList &List,OpProgress &Progress,
			  pkgCacheGenerator &Gen,
			  unsigned long &CurrentSize,unsigned long &TotalSize);
        
    inline virtual bool mergeInstalledPackages(OpProgress &Progress,
					       pkgCacheGenerator &Gen,
					       unsigned long &CurrentSize,
					       unsigned long TotalSize) {
	return false;
    };
        
    inline virtual bool preProcess(pkgSourceList &List, OpProgress &Progress) {return true;}
    
 public:
    inline virtual bool checkSourceType(int type, bool binary=true) {return false;}

    bool makeStatusCache(pkgSourceList &List,OpProgress &Progress);
    MMap *makeStatusCacheMem(pkgSourceList &List,OpProgress &Progress);

    virtual int versionCompare(const char *A, const char *B);
    virtual int versionCompare(const char *A, const char *AEnd, const char *B, 
			       const char *BEnd);
    virtual int versionCompare(string A,string B);
    virtual bool checkDep(const char *DepVer,const char *PkgVer,int Op);
    virtual string baseVersion(const char *Ver);
    
    // for other stuffs
 public:
    SystemFactory::SystemFactory();
    
    virtual pkgCacheGenerator::ListParser *CreateListParser(FileFd &File) = 0;
    virtual pkgRecords::Parser *CreateRecordParser(string File, pkgCache &Cache) = 0;
    virtual pkgSrcRecords::Parser *CreateSrcRecordParser(string File, pkgSourceList::const_iterator SrcItem) = 0;
    virtual pkgPackageManager *CreatePackageManager(pkgDepCache &Cache);

};

extern SystemFactory *_system;

#endif
