
#include <unistd.h>

#include <apt-pkg/error.h>
#include <apt-pkg/rpmpackagedata.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/configuration.h>

#include <i18n.h>



RPMPackageData::RPMPackageData()
{
   string path = _config->FindFile("Dir::Etc::RpmPriorities");
   FileFd F(path, FileFd::ReadOnly);
   if (_error->PendingError()) 
   {
      _error->Error(_("could not open package priority file %s"), path.c_str());
      return;
   }
   pkgTagFile Tags(F);
   pkgTagSection Section;

   if (!Tags.Step(Section)) 
   {
      _error->Error(_("no data in %s"), path.c_str());
       return;
   }
   
   for (int i = 0; i < 5; i++) 
   {
      static const char *priorities[] = 
      {
	 "Important", "Required", "Standard", "Optional", "Extra"
      };
      static pkgCache::State::VerPriority states[] = {
	 pkgCache::State::Important,
	     pkgCache::State::Required,
	     pkgCache::State::Standard,
	     pkgCache::State::Optional,
	     pkgCache::State::Extra
      };
      
      string Packages = Section.FindS(priorities[i]);
      if (Packages.empty()) 
	 continue;

      const char *C = Packages.c_str();
      while (*C != 0)
      {
	 string pkg;
	 
	 if (ParseQuoteWord(C,pkg))
	     Priorities[pkg] = states[i];
      }
   }
}


RPMPackageData *RPMPackageData::Singleton()
{
    static RPMPackageData *data = NULL;
    
    if (!data)
	data = new RPMPackageData();

    return data;
}
