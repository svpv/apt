
#include <apt-pkg/error.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/deblistparser.h>
#include <apt-pkg/dpkgpm.h>
#include <apt-pkg/debfactory.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/debrecords.h>
#include <apt-pkg/debsrcrecords.h>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <system.h>



pkgPackageManager *DebianFactory::CreatePackageManager(pkgDepCache &Cache)
{
   return new pkgDPkgPM(Cache);
}


pkgRecords::Parser *DebianFactory::CreateRecordParser(string File, pkgCache &Cache)
{
    FileFd f;
    
    f = FileFd(File, FileFd::ReadOnly);
    if (_error->PendingError())
	return NULL;
    
    return new debRecordParser(f, Cache);
}


pkgSrcRecords::Parser *DebianFactory::CreateSrcRecordParser(string File, 
							    pkgSourceList::const_iterator SrcItem)
{
    FileFd *f;
    
    f = new FileFd(File, FileFd::ReadOnly);
    if (_error->PendingError()) {
	delete f;
	return NULL;
    }
    
    return new debSrcRecordParser(f, SrcItem);
}


bool DebianFactory::checkSourceType(int type, bool binary)
{
   if (binary)
       return (type == pkgSourceList::Deb);
   else
       return (type == pkgSourceList::DebSrc);
}


pkgCacheGenerator::ListParser *DebianFactory::CreateListParser(FileFd &File)
{
    return new debListParser(File);
}


									/*}}}*/
// PkgCacheCheck - Check if the package cache is uptodate		/*{{{*/
// ---------------------------------------------------------------------
/* This does a simple check of all files used to compose the cache */
bool DebianFactory::packageCacheCheck(string CacheFile)
{
   if (_error->PendingError() == true)
      return false;
   
   // Open the source package cache
   if (FileExists(CacheFile) == false)
      return false;
   
   FileFd CacheF(CacheFile,FileFd::ReadOnly);
   if (_error->PendingError() == true)
   {
      _error->Discard();
      return false;
   }
   
   MMap Map(CacheF,MMap::Public | MMap::ReadOnly);
   if (_error->PendingError() == true || Map.Size() == 0)
   {
      _error->Discard();
      return false;
   }
   
   pkgCache Cache(Map);
   if (_error->PendingError() == true)
   {
      _error->Discard();
      return false;
   }
   
   // Status files that must be in the cache
   string Status[3];
   Status[0] = _config->FindFile("Dir::State::xstatus");
   Status[1]= _config->FindFile("Dir::State::userstatus");
   Status[2] = _config->FindFile("Dir::State::status");
   
   // Cheack each file
   for (pkgCache::PkgFileIterator F(Cache); F.end() == false; F++)
   {
      if (F.IsOk() == false)
	 return false;
      
      // See if this is one of the status files
      for (int I = 0; I != 3; I++)
	 if (F.FileName() == Status[I])
	    Status[I] = string();
   }
   
   // Make sure all the status files are loaded.
   for (int I = 0; I != 3; I++)
   {
      if (Status[I].empty() == false && FileExists(Status[I]) == true)
	 return false;
   }   
   
   return true;
}
									/*}}}*/
// AddStatusSize - Add the size of the status files			/*{{{*/
// ---------------------------------------------------------------------
/* This adds the size of all the status files to the size counter */
bool DebianFactory::addStatusSize(unsigned long &TotalSize)
{
   // Grab the file names
   string xstatus = _config->FindFile("Dir::State::xstatus");
   string userstatus = _config->FindFile("Dir::State::userstatus");
   string status = _config->FindFile("Dir::State::status");
   
   // Grab the sizes
   struct stat Buf;
   if (stat(xstatus.c_str(),&Buf) == 0)
      TotalSize += Buf.st_size;
   if (stat(userstatus.c_str(),&Buf) == 0)
      TotalSize += Buf.st_size;
   if (stat(status.c_str(),&Buf) != 0)
      return _error->Errno("stat","Couldn't stat the status file %s",status.c_str());
   TotalSize += Buf.st_size;
   
   return true;
}
									/*}}}*/
// MergeInstalledPackages (was MergeStatus) - Add the status files to the cache			/*{{{*/
// ---------------------------------------------------------------------
/* This adds the status files to the map */
bool DebianFactory::mergeInstalledPackages(OpProgress &Progress,
					   pkgCacheGenerator &Gen,
					   unsigned long &CurrentSize,
					   unsigned long TotalSize)
{
   // Grab the file names   
   string Status[3];
   Status[0] = _config->FindFile("Dir::State::xstatus");
   Status[1]= _config->FindFile("Dir::State::userstatus");
   Status[2] = _config->FindFile("Dir::State::status");
   
   for (int I = 0; I != 3; I++)
   {
      pkgCacheGenerator::ListParser *Parser;
      
      // Check if the file exists and it is not the primary status file.
      string File = Status[I];
      if (I != 2 && FileExists(File) == false)
	 continue;

      FileFd Pkg(File,FileFd::ReadOnly);
      Parser = new debListParser(Pkg);

      Progress.OverallProgress(CurrentSize,TotalSize,Pkg.Size(),"Reading Package Lists");
      if (_error->PendingError() == true)
	 return _error->Error("Problem opening %s",File.c_str());
      CurrentSize += Pkg.Size();

      Progress.SubProgress(0,"Local Package State - " + flNotDir(File));
      if (Gen.SelectFile(File,pkgCache::Flag::NotSource) == false)
	 return _error->Error("Problem with SelectFile %s",File.c_str());
      
      if (Gen.MergeList(*Parser) == false)
	 return _error->Error("Problem with MergeList %s",File.c_str());
      Progress.Progress(Pkg.Size());
      
      delete Parser;
   }
   
   return true;
}
									/*}}}*/
