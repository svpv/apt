
#ifndef _RPMFACTORY_H_
#define _RPMFACTORY_H_

#include <apt-pkg/systemfactory.h>
#include <map>
#include <slist>
#include <vector>

class RPMFactory : public SystemFactory
{
    map<string,int> *filedeps;
    map<string,string> *multiarchs; // pkgs with multiple architectures

 protected:
    bool addStatusSize(unsigned long &TotalSize);
    
    bool mergeInstalledPackages(OpProgress &Progress,pkgCacheGenerator &Gen,
				unsigned long &CurrentSize,
				unsigned long TotalSize);
    
    virtual bool packageCacheCheck(string CacheFile);
    
    virtual bool checkSourceType(int type, bool binary=true);
    
    bool preProcess(pkgSourceList &List, OpProgress &Progress);
    
 public:
    virtual int versionCompare(const char *A, const char *B);
    virtual int versionCompare(const char *A, const char *AEnd, const char *B, 
			       const char *BEnd);
    virtual int versionCompare(string A,string B);
    virtual bool checkDep(const char *DepVer,const char *PkgVer,int Op);
    virtual string baseVersion(const char *Ver);

 public:
    pkgCacheGenerator::ListParser *CreateListParser(FileFd &File);
    pkgRecords::Parser *CreateRecordParser(string File, pkgCache &Cache);
    pkgSrcRecords::Parser *CreateSrcRecordParser(string File, pkgSourceList::const_iterator SrcItem);
    pkgPackageManager *CreatePackageManager(pkgDepCache &Cache);
    
    RPMFactory::RPMFactory();
};

#endif
