
#ifndef _RPMPACKAGEDATA_H_
#define _RPMPACKAGEDATA_H_


#include <apt-pkg/tagfile.h>
#include <apt-pkg/pkgcache.h>

#include <map>

class RPMPackageData 
{
protected:
   map<string,pkgCache::State::VerPriority> Priorities;

public:
   inline pkgCache::State::VerPriority PackagePriority(string Package) 
   {
      if (Priorities.find(Package) == Priorities.end())
	  return pkgCache::State::Standard;
      else
	  return Priorities[Package]; 
   }
   
   static RPMPackageData *Singleton();
   
   RPMPackageData();
};


#endif
