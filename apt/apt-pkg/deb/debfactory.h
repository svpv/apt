
#ifndef _DEBIANFACTORY_H_
#define _DEBIANFACTORY_H_

#include <apt-pkg/systemfactory.h>


class DebianFactory : public SystemFactory
{
// for cache generation
 protected:
    bool addStatusSize(unsigned long &TotalSize);
    
    bool mergeInstalledPackages(OpProgress &Progress,pkgCacheGenerator &Gen,
			       unsigned long &CurrentSize,
			       unsigned long TotalSize);

 protected:
    bool packageCacheCheck(string CacheFile);
   
    bool checkSourceType(int type, bool binary=true);

 public:
// other stuffs
    pkgCacheGenerator::ListParser *CreateListParser(FileFd &File);
    pkgRecords::Parser *CreateRecordParser(string File, pkgCache &Cache);
    pkgSrcRecords::Parser *CreateSrcRecordParser(string File, pkgSourceList::const_iterator SrcItem);
    pkgPackageManager *CreatePackageManager(pkgDepCache &Cache);
};

#endif
