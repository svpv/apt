/*
 * $Id: rpmfactory.cc,v 1.27 2001/11/13 17:32:08 kojima Exp $
 */

#include <config.h>

#include <apt-pkg/rpmlistparser.h>
#include <apt-pkg/rpmpm.h>
#include <apt-pkg/rpmfactory.h>
#include <apt-pkg/rpminit.h>
#include <apt-pkg/rpmrecords.h>
#include <apt-pkg/rpmsrcrecords.h>

#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/cacheiterators.h>
#include <apt-pkg/fileutl.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/sourcelist.h>

#include <rpm/rpmlib.h>
#include <rpm/rpmmacro.h>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <system.h>
#include <sys/utsname.h>

#include <i18n.h>

RPMFactory::RPMFactory()
{
   filedeps = new map<string,int>();
   multiarchs = new map<string,string>();
}


pkgPackageManager *RPMFactory::CreatePackageManager(pkgDepCache &Cache)
{
   return new pkgRPMPM(Cache);
}


pkgRecords::Parser *RPMFactory::CreateRecordParser(string File, pkgCache &Cache)
{
   if (File == pkgRpmLock::RPMDBPath())
       return new rpmRecordParser("", Cache);
   else
       return new rpmRecordParser(File, Cache);
}


pkgSrcRecords::Parser *RPMFactory::CreateSrcRecordParser(string File,
				     pkgSourceList::const_iterator SrcItem)
{
   FileFd *Fd = new FileFd(File, FileFd::ReadOnly);
   if (_error->PendingError()) 
   {
      delete Fd;
      return NULL;
   }
   
   return new rpmSrcRecordParser(Fd, SrcItem);
}


bool RPMFactory::checkSourceType(int type, bool binary)
{
   if (binary)
       return (type == pkgSourceList::Rpm);
   else
       return (type == pkgSourceList::RpmSrc);
}


bool RPMFactory::packageCacheCheck(string CacheFile)
{    
    if (_error->PendingError())
	return false;
    
    // Open the source package cache
    if (!FileExists(CacheFile)) {
	return false;
    }
    
    FileFd CacheF(CacheFile,FileFd::ReadOnly);
    if (_error->PendingError())
    {
	_error->Discard();
	return false;
    }
    
    MMap Map(CacheF,MMap::Public | MMap::ReadOnly);
    if (_error->PendingError() || Map.Size() == 0)
    {
	_error->Discard();
	return false;
    }
    
    pkgCache Cache(Map);
    if (_error->PendingError())
    {
	_error->Discard();
	return false;
    }
    
    string blaa="packages.rpm";
#ifdef HAVE_RPM4
    if (rpmExpandNumeric("%{_dbapi}") == 3)
	blaa = "Packages";
#endif

    string rpmdb_path = pkgRpmLock::RPMDBPath() + blaa;
    
    int bla = 0;
    
    // Check each file
    for (pkgCache::PkgFileIterator F(Cache); F.end() == false; F++)
    {
	if (F.IsOk() == false) {
	    return false;
	}
	if (strcmp(rpmdb_path.c_str(), F.FileName()) != 0) {
	    continue;
	}
	bla++;
    }
    
    // only one file
    if (bla != 1) {
	return false;
    }

    return true;
}


pkgCacheGenerator::ListParser *RPMFactory::CreateListParser(FileFd &File)
{
    return new rpmListParser(File, filedeps, multiarchs);
}


									/*}}}*/
// AddStatusSize - Add the size of the status files			/*{{{*/
// ---------------------------------------------------------------------
bool RPMFactory::addStatusSize(unsigned long &TotalSize)
{
   //struct stat Buf;
   unsigned size, bla;
   
   pkgRpmLock::SharedRPM()->Offset(size, bla);
   
   TotalSize += size;
   
   return true;
}
									/*}}}*/

// MergeInstalledPackages (was MergeStatus) - Add the status files to the cache			/*{{{*/
// ---------------------------------------------------------------------
/* This adds the status files to the map */
bool RPMFactory::mergeInstalledPackages(OpProgress &Progress,
					   pkgCacheGenerator &Gen,
					   unsigned long &CurrentSize,
					   unsigned long TotalSize)
{
    unsigned size, bla;
    string suffix = "packages.rpm";
#ifdef HAVE_RPM4
    if (rpmExpandNumeric("%{_dbapi}") == 3)
	suffix = "Packages";
#endif

    string tmp = string(pkgRpmLock::RPMDBPath()+suffix).c_str();

    const char *dbpath = tmp.c_str();

    pkgRpmLock::SharedRPM()->Rewind();
    // Check if the file exists and it is not the primary status file.
    rpmListParser *Parser = new rpmListParser(filedeps, multiarchs);


    pkgRpmLock::SharedRPM()->Offset(size, bla);

    Progress.OverallProgress(CurrentSize,TotalSize, size,_("Reading Package Lists"));
    if (_error->PendingError() == true)
	return _error->Error(_("Problem opening %s"), dbpath);
    CurrentSize += size;

    Progress.SubProgress(0,_("Local Package State - ") + flNotDir(dbpath));
    if (Gen.SelectFile(dbpath,pkgCache::Flag::NotSource) == false)
	return _error->Error(_("Problem with SelectFile %s"), dbpath);

    if (Gen.MergeList(*Parser) == false)
	return _error->Error(_("Problem with MergeList %s"), dbpath);
    Progress.Progress(size);

    return true;
}




void gatherFileDependencies(map<string,int>*filedeps, Header header)
{
    int type, count;
    char **namel;
    char **verl;
    int *flagl;
    int res;
    
    res = headerGetEntry(header, RPMTAG_REQUIRENAME, &type,
			 (void **)&namel, &count);
    res = headerGetEntry(header, RPMTAG_REQUIREVERSION, &type, 
			 (void **)&verl, &count);
    res = headerGetEntry(header, RPMTAG_REQUIREFLAGS, &type,
			 (void **)&flagl, &count);

    while (count--) {
	if (*namel[count] == '/') { // ugh file dep!
	    (*filedeps)[string(namel[count])] = 1;
	}
    }
}


bool RPMFactory::preProcess(pkgSourceList &List, OpProgress &Progress)
{    
    string ListDir = _config->FindDir("Dir::State::lists");
    unsigned total, complete;
    struct stat stbuf;
    const char *baseArch;
    const char *thisArch;
    struct utsname un;
    map<string,string> archmap;

    baseArch = _config->Find("APT::architecture").c_str();

    if (uname(&un) < 0) {
	thisArch = baseArch;
    } else {
	thisArch = un.machine;
    }

    // calculate size of files

    pkgRpmLock::SharedRPM()->Offset(total, complete);

    for (pkgSourceList::const_iterator I = List.begin(); I != List.end(); I++)
    {
	// Only check relevant source types.
	if (!checkSourceType(I->Type()))
	    continue;

	string File = ListDir + URItoFileName(I->PackagesURI());
	
	if (FileExists(File) == false)
	    continue;
	
	if (stat(File.c_str(), &stbuf) == 0) 
	{
	    total += stbuf.st_size;
	}
    }

        
    complete = 0;
    Progress.OverallProgress(0, 100, 100, _("Processing File Dependencies"));
    Progress.SubProgress(total, _("Looking for file dependencies"));    

    for (pkgSourceList::const_iterator I = List.begin(); I != List.end(); I++)
    {
	// Only check relevant source types.
	if (!checkSourceType(I->Type()))
	    continue;

	string File = ListDir + URItoFileName(I->PackagesURI());
	
	if (FileExists(File) == false)
	    continue;
	
	FileFd F(File, FileFd::ReadOnly);
	if (_error->PendingError() == true)
	    return _error->Error(_("Problem opening %s"),File.c_str());

	FD_t fdt = fdDup(F.Fd());
	while (1) 
	{
	    Header hdr;
	    int type, count, res;
	    char *arch;
	    
	    hdr = headerRead(fdt, HEADER_MAGIC_YES);
	    if (!hdr)
		break;
	    gatherFileDependencies(filedeps, hdr);
	   

	    /*
	     * Make it so that for each version, we keep track of the best
	     * architecture.
	     */
	    res = headerGetEntry(hdr, RPMTAG_ARCH, &type,
				 (void **)&arch, &count);
	    assert(type == RPM_STRING_TYPE);
	    if (res) {
		char *name;
		char *version;
		char *release;
		int_32 *epoch;
	        int res;
		char buf[256];

		headerGetEntry(hdr, RPMTAG_NAME, &type,
			       (void **)&name, &count);
		headerGetEntry(hdr, RPMTAG_VERSION, &type,
			       (void **)&version, &count);
		headerGetEntry(hdr, RPMTAG_RELEASE, &type,
			       (void **)&release, &count);
 	        res = headerGetEntry(hdr, RPMTAG_EPOCH, &type,
				    (void **)&epoch, &count);

	        if (res == 1) {
		    snprintf(buf, 255, "%i:%s-%s", *epoch, version, release);
		} else {
		    snprintf(buf, 255, "%s-%s", version, release);
		}
		string n = string(name)+"#"+string(buf);

		if (archmap.find(n) != archmap.end()) 
		{
		    if (strcmp(archmap[n].c_str(), arch) != 0) {
			int a = rpmMachineScore(RPM_MACHTABLE_INSTARCH, archmap[n].c_str());
			int b = rpmMachineScore(RPM_MACHTABLE_INSTARCH, arch);

			if (b < a && b > 0) {
			    (*multiarchs)[n] = string(arch);
			    // this is a multiarch pkg
			    archmap[n] = (*multiarchs)[n];
			} else {
			    (*multiarchs)[n] = archmap[n];
			}
		    }
		}
		else
		{
		    int a = rpmMachineScore(RPM_MACHTABLE_INSTARCH, arch);
		    if (a > 0)
		    {
		        archmap[n] = string(arch);
			(*multiarchs)[n] = string(arch);
	            }
		}
	    }
	    headerFree(hdr);
	    
	    Progress.Progress(complete+F.Tell());
	}
	fdClose(fdt);
	complete += F.Size();
    }

    pkgRpmLock::SharedRPM()->Rewind();
    while (1)
    {
	Header hdr;
	unsigned comp;

	hdr = pkgRpmLock::SharedRPM()->NextHeader();
	if (!hdr)
	    break;
	gatherFileDependencies(filedeps, hdr);
	headerFree(hdr);
	pkgRpmLock::SharedRPM()->Offset(total, comp);
	Progress.Progress(complete+comp);
    }
    Progress.Done();

    return true;
}
