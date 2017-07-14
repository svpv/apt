// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: acquire-item.cc,v 1.46 2003/02/02 22:19:17 jgg Exp $
/* ######################################################################

   Acquire Item - Item to acquire

   Each item can download to exactly one file at a time. This means you
   cannot create an item that fetches two uri's to two files at the same 
   time. The pkgAcqIndex class creates a second class upon instantiation
   to fetch the other index files because of this.

   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#ifdef __GNUG__
#pragma implementation "apt-pkg/acquire-item.h"
#endif

#include <config.h>

#include <apt-pkg/acquire-item.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/error.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/fileutl.h>

// CNC:2002-07-03
#include <apt-pkg/repository.h>
#include <apt-pkg/md5.h>
#include <apt-pkg/luaiface.h>
#include <iostream>
#include <assert.h>
using namespace std;

#include <apti18n.h>
    
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <fcntl.h>
#include <endian.h>
									/*}}}*/

using std::string;

// CNC:2002-07-03
// VerifyChecksums - Check MD5 and SHA-1 checksums of a file		/*{{{*/
// ---------------------------------------------------------------------
/* Returns false only if the checksums fail (the file not existing is not
   a checksum mismatch) */
static bool VerifyChecksums(string const& File, FileFd& Fd, unsigned long Size, string const& MD5)
{
   unsigned long FdSize = Fd.Size();
   if (FdSize != Size)
   {
      if (_config->FindB("Acquire::Verbose", false) == true)
	 cout << "Size of "<<File<<" did not match what's in the checksum list and was redownloaded."<<endl;
      return false;
   }

   if (MD5.empty() == false)
   {
      MD5Summation md5sum = MD5Summation();
      FileFd F(File, FileFd::ReadOnly);
      
      md5sum.AddFD(Fd.Fd(), FdSize);
      if (md5sum.Result().Value() != MD5)
      {
         if (_config->FindB("Acquire::Verbose", false) == true)
	    cout << "MD5Sum of "<<File<<" did not match what's in the checksum list and was redownloaded."<<endl;
         return false;
      }
   }
   
   return true;
}

static bool VerifyChecksums(string const& File, unsigned long Size, string const& MD5)
{
   int fd = open(File.c_str(), O_RDONLY);
   if (fd < 0) {
      if (errno == ENOENT)
        return true;
      return _error->WarningE("open", _("Could not open file %s"), File.c_str());
   }

   FileFd Fd(fd);
   return VerifyChecksums(File, Fd, Size, MD5);
}

static unsigned long PeekLZ4Size(FileFd& Fd)
{
   // See methods/gzip.cc
   unsigned char buf[14];
   if (!Fd.Read(buf, 14))
	return -1;
   unsigned magic = htole32(0x184D2204);
   if (memcmp(buf, &magic, 4))
	return -1;
   if (!(buf[4] & 8))
	return -1;
   unsigned size;
   memcpy(&size, buf + 10, 4);
   if (size != 0)
	return -1;
   memcpy(&size, buf + 6, 4);
   return le32toh(size);
}

static bool VerifyLZ4Checksums(string const& File, FileFd& Fd, unsigned long Size, string const& MD5)
{
   unsigned long lz4size = PeekLZ4Size(Fd);
   if (lz4size != Size) {
      if (lz4size == (unsigned long) -1)
	 return _error->Warning(_("Could not peek LZ4 size for %s"), File.c_str());
      if (_config->FindB("Acquire::Verbose", false))
	 cout << "Size of " << File << " did not match what's in the checksum list and was redownloaded." << endl;
      return false;
   }
   Fd.Seek(0);

   int lz4pipe[2];
   if (pipe(lz4pipe) < 0)
      return _error->WarningE("pipe", _("Couldn't open pipe for %s"), "lz4");

   int lz4pid = ExecFork();
   if (lz4pid == 0) {
      dup2(Fd.Fd(), 0);
      dup2(lz4pipe[1], 1);
      SetCloseExec(0, false);
      SetCloseExec(1, false);

      string lz4bin = _config->Find("Dir::bin::lz4", "lz4");
      const char *args[] = { lz4bin.c_str(), "-d", NULL };
      execvp(args[0], (char **) args);
      _exit(100);
   }
   close(lz4pipe[1]);

   MD5Summation md5sum = MD5Summation();
   bool added = md5sum.AddFD(lz4pipe[0], Size);
   close(lz4pipe[0]);
   bool waited = ExecWait(lz4pid, "lz4");
   if (!(added && waited))
      return false;

   if (md5sum.Result().Value() != MD5) {
      if (_config->FindB("Acquire::Verbose", false))
	 cout << "MD5Sum of " << File << " did not match what's in the checksum list and was redownloaded." << endl;
      return false;
   }

   return true;
}
                                                                        /*}}}*/
// Acquire::Item::Item - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgAcquire::Item::Item(pkgAcquire *Owner) : Owner(Owner), FileSize(0),
                       PartialSize(0), Mode(0), ID(0), Complete(false), 
                       Local(false), QueueCounter(0)
{
   Owner->Add(this);
   Status = StatIdle;
}
									/*}}}*/
// Acquire::Item::~Item - Destructor					/*{{{*/
// ---------------------------------------------------------------------
/* */
pkgAcquire::Item::~Item()
{
   Owner->Remove(this);
}
									/*}}}*/
// Acquire::Item::Failed - Item failed to download			/*{{{*/
// ---------------------------------------------------------------------
/* We return to an idle state if there are still other queues that could
   fetch this object */
void pkgAcquire::Item::Failed(string Message,pkgAcquire::MethodConfig *Cnf)
{
   Status = StatIdle;
   ErrorText = LookupTag(Message,"Message");
   if (QueueCounter <= 1)
   {
      /* This indicates that the file is not available right now but might
         be sometime later. If we do a retry cycle then this should be
	 retried [CDROMs] */
      if (Cnf->LocalOnly == true &&
	  StringToBool(LookupTag(Message,"Transient-Failure"),false) == true)
      {
	 Status = StatIdle;
	 Dequeue();
	 return;
      }
      
      Status = StatError;
      Dequeue();
   }   
}
									/*}}}*/
// Acquire::Item::Start - Item has begun to download			/*{{{*/
// ---------------------------------------------------------------------
/* Stash status and the file size. Note that setting Complete means 
   sub-phases of the acquire process such as decompresion are operating */
void pkgAcquire::Item::Start(string /*Message*/,unsigned long Size)
{
   Status = StatFetching;
   if (FileSize == 0 && Complete == false)
      FileSize = Size;
}
									/*}}}*/
// Acquire::Item::Done - Item downloaded OK				/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcquire::Item::Done(string Message,unsigned long Size,string,
			    pkgAcquire::MethodConfig *Cnf)
{
   // We just downloaded something..
   string FileName = LookupTag(Message,"Filename");
   if (Complete == false && FileName == DestFile)
   {
      if (Owner->Log != 0)
	 Owner->Log->Fetched(Size,atoi(LookupTag(Message,"Resume-Point","0").c_str()));
   }

   if (FileSize == 0)
      FileSize= Size;
   
   Status = StatDone;
   ErrorText = string();
   Owner->Dequeue(this);
}
									/*}}}*/
// Acquire::Item::Rename - Rename a file				/*{{{*/
// ---------------------------------------------------------------------
/* This helper function is used by alot of item methods as thier final
   step */
void pkgAcquire::Item::Rename(string From,string To)
{
   if (rename(From.c_str(),To.c_str()) != 0)
   {
      char S[300];
      snprintf(S,sizeof(S),_("rename failed, %s (%s -> %s)."),strerror(errno),
	      From.c_str(),To.c_str());
      Status = StatError;
      ErrorText = S;
   }   
}
									/*}}}*/

// AcqIndex::AcqIndex - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* The package file is added to the queue and a second class is 
   instantiated to fetch the revision file */   
// CNC:2002-07-03
pkgAcqIndex::pkgAcqIndex(pkgAcquire *Owner,pkgRepository *Repository,
			 string URI,string URIDesc,string ShortDesc) :
                      Item(Owner), RealURI(URI), Repository(Repository)
{
   Decompression = false;
   Erase = false;
   
   string FinalFile = _config->FindDir("Dir::State::lists");
   DestFile = FinalFile + "partial/";

   // Create the item
   // CNC:2002-07-03
   string zext = _config->Find("Acquire::ComprExtension", ".xz");
   Desc.URI = URI + zext;
   Desc.Description = URIDesc;
   Desc.Owner = this;
   Desc.ShortDesc = ShortDesc;
      
   // CNC:2002-07-03
   // If we're verifying authentication, check whether the size and
   // checksums match, if not, delete the cached files and force redownload
   string MD5Hash;
   unsigned long Size;

   if (Repository && Repository->HasRelease())
   {
      if (Repository->FindChecksums(Desc.URI, Size, MD5Hash))
	 (void) 0;
      else if (Repository->FindChecksums(URI + ".xz", Size, MD5Hash))
	 Desc.URI = URI + (zext = ".xz");
      else if (Repository->FindChecksums(URI + ".zst", Size, MD5Hash))
	 Desc.URI = URI + (zext = ".zst");
      else if (Repository->FindChecksums(URI + ".bz2", Size, MD5Hash))
	 Desc.URI = URI + (zext = ".bz2");
      else {
	 _error->Error("Release file did not contain checksum information for %s",
		       Desc.URI.c_str());
	 return;
      }

      string URIFileName = URItoFileName(URI);
      DestFile += URIFileName, FinalFile += URIFileName;
      unlink(DestFile.c_str()), unlink(FinalFile.c_str());

      if (zext == ".zst") {
	 // No need to recompress .zst to .lz4.
	 // Verify the compressed size and checksum.
	 unlink((FinalFile + ".lz4").c_str());
	 DestFile += ".zst", FinalFile += ".zst";
      }
      else {
	 // Final file gets recompressed to .lz4.
	 // Verify the uncompressed size and checksum.
	 if (!Repository->FindChecksums(URI, Size, MD5Hash)) {
	    _error->Error("Release file did not contain checksum information for %s",
			  URI.c_str());
	    return;
	 }
	 unlink((FinalFile + ".zst").c_str());
	 DestFile += zext, FinalFile += ".lz4";
      }

      int fd = open(FinalFile.c_str(), O_RDONLY);
      if (fd < 0 && errno != ENOENT) {
	 _error->WarningE("open", _("Could not open file %s"), FinalFile.c_str());
	 if (unlink(FinalFile.c_str()) == 0)
	    _error->Warning(_("Unlinked file %s"), FinalFile.c_str());
	 else if (errno != ENOENT) {
	    _error->Errno("unlink", _("Cannot unlink file %s"), FinalFile.c_str());
	    return;
	 }
      }
      else if (fd >= 0) {
	 FileFd Fd(fd);
	 bool verified;
	 if (zext == ".zst")
	    verified = VerifyChecksums(FinalFile, Fd, Size, MD5Hash);
	 else
	    verified = VerifyLZ4Checksums(FinalFile, Fd, Size, MD5Hash);
	 if (verified) {
	    if (_config->FindB("Acquire::Verbose", false))
	       cout << "File " << FinalFile << " is up to date" << endl;
	    return;
	 }
	 unlink(DestFile.c_str()), unlink(FinalFile.c_str());
      }
   }
   else if (Repository && Repository->IsAuthenticated())
   {
      _error->Error(_("Release information not available for %s"),
		    URI.c_str());
      return;
   }

   QueueURI(Desc);
}
									/*}}}*/
// AcqIndex::Custom600Headers - Insert custom request headers		/*{{{*/
// ---------------------------------------------------------------------
/* The only header we use is the last-modified header. */
string pkgAcqIndex::Custom600Headers()
{
   string Final = _config->FindDir("Dir::State::lists");
   Final += URItoFileName(RealURI);
   
   struct stat Buf;
   if (stat(Final.c_str(),&Buf) != 0)
      return "\nIndex-File: true";
   
   return "\nIndex-File: true\nLast-Modified: " + TimeRFC1123(Buf.st_mtime);
}
									/*}}}*/
// AcqIndex::Done - Finished a fetch					/*{{{*/
// ---------------------------------------------------------------------
/* This goes through a number of states.. On the initial fetch the
   method could possibly return an alternate filename which points
   to the uncompressed version of the file. If this is so the file
   is copied into the partial directory. In all other cases the file
   is decompressed with a gzip uri. */
void pkgAcqIndex::Done(string Message,unsigned long Size,string MD5,
		       pkgAcquire::MethodConfig *Cfg)
{
   Item::Done(Message,Size,MD5,Cfg);

   if (Decompression == true)
   {
      // CNC:2002-07-03
      unsigned long FSize;
      string MD5Hash;

      if (Repository != NULL && Repository->HasRelease() == true &&
	  Repository->FindChecksums(RealURI,FSize,MD5Hash) == true)
      {
	 // We must always get here if the repository is authenticated
	 
	 if (FSize != Size)
	 {
	    Status = StatError;
	    ErrorText = _("Size mismatch");
	    Rename(DestFile,DestFile + ".FAILED");
	    if (_config->FindB("Acquire::Verbose",false) == true) 
	       _error->Warning("Size mismatch of index file %s: %lu was supposed to be %lu",
			       RealURI.c_str(), Size, FSize);
	    return;
	 }
	    
	 if (MD5.empty() == false && MD5Hash != MD5)
	 {
	    Status = StatError;
	    ErrorText = _("MD5Sum mismatch");
	    Rename(DestFile,DestFile + ".FAILED");
	    if (_config->FindB("Acquire::Verbose",false) == true) 
	       _error->Warning("MD5Sum mismatch of index file %s: %s was supposed to be %s",
			       RealURI.c_str(), MD5.c_str(), MD5Hash.c_str());
	    return;
	 }
      }
      else
      {
	 // Redundant security check
	 assert(Repository == NULL || Repository->IsAuthenticated() == false);
      }
	 
      // Done, move it into position
      string FinalFile = _config->FindDir("Dir::State::lists");
      FinalFile += URItoFileName(RealURI);
      Rename(DestFile,FinalFile);
      chmod(FinalFile.c_str(),0644);
      
      /* We restore the original name to DestFile so that the clean operation
         will work OK */
      DestFile = _config->FindDir("Dir::State::lists") + "partial/";
      DestFile += URItoFileName(RealURI);
      
      // Remove the compressed version.
      if (Erase == true)
	 unlink(DestFile.c_str());
      return;
   }

   Erase = false;
   Complete = true;
   
#if 0
   // Handle the unzipd case
   string FileName = LookupTag(Message,"Alt-Filename");
   if (FileName.empty() == false)
   {
      // The files timestamp matches
      if (StringToBool(LookupTag(Message,"Alt-IMS-Hit"),false) == true)
	 return;
      
      Decompression = true;
      Local = true;
      DestFile += ".decomp";
      Desc.URI = "copy:" + FileName;
      QueueURI(Desc);
      Mode = "copy";
      return;
   }
#endif

   string FileName = LookupTag(Message,"Filename");
   if (FileName.empty() == true)
   {
      Status = StatError;
      ErrorText = "Method gave a blank filename";
   }
   
   // The files timestamp matches
   if (StringToBool(LookupTag(Message,"IMS-Hit"),false) == true)
      return;

   if (FileName == DestFile)
      Erase = true;
   else
      Local = true;
   
   Decompression = true;
   DestFile += ".decomp";

   // CNC:2002-07-03
   const char *prog = "bzip2";
   if (flExtension(Desc.URI) == "xz")
      prog = "xz";
   Desc.URI = string(prog) + ":" + FileName;
   QueueURI(Desc);
   // CNC:2002-07-03
   Mode = prog;
}
									/*}}}*/

// AcqIndexRel::pkgAcqIndexRel - Constructor				/*{{{*/
// ---------------------------------------------------------------------
/* The Release file is added to the queue */
// CNC:2002-07-03
pkgAcqIndexRel::pkgAcqIndexRel(pkgAcquire *Owner,pkgRepository *Repository,
			       string URI,string URIDesc,string ShortDesc,
			       bool Master) :
                      Item(Owner), RealURI(URI), Master(Master),
		      Repository(Repository)
{
   // CNC:2002-07-09
   assert(Master == false || Repository != NULL);
   Authentication = false;
   Erase = false;

   DestFile = _config->FindDir("Dir::State::lists") + "partial/";
   DestFile += URItoFileName(URI);
   
   // Create the item
   Desc.URI = URI;
   Desc.Description = URIDesc;
   Desc.ShortDesc = ShortDesc;
   Desc.Owner = this;

   // CNC:2002-07-09
   string MD5Hash;
   unsigned long Size;
   if (Master == false && Repository != NULL)
   {
      if (Repository->HasRelease() == true)
      {
	 if (Repository->FindChecksums(RealURI,Size,MD5Hash) == false)
	 {
	    if (Repository->IsAuthenticated() == true)
	    {
	       _error->Error(_("%s is not listed in the checksum list for its repository"),
			     RealURI.c_str());
	       return;
	    }
	    else
	       _error->Warning("Release file did not contain checksum information for %s",
			       RealURI.c_str());
	 }

	 string FinalFile = _config->FindDir("Dir::State::lists");
	 FinalFile += URItoFileName(RealURI);

	 if (VerifyChecksums(FinalFile,Size,MD5Hash) == false)
	 {
	    unlink(FinalFile.c_str());
	    unlink(DestFile.c_str()); // Necessary?
	 }
      }
      else if (Repository->IsAuthenticated() == true)
      {
	 _error->Error(_("Release information not available for %s"),
		       URI.c_str());
	 return;
      }
   }

   QueueURI(Desc);
}
									/*}}}*/
// AcqIndexRel::Custom600Headers - Insert custom request headers	/*{{{*/
// ---------------------------------------------------------------------
/* The only header we use is the last-modified header. */
string pkgAcqIndexRel::Custom600Headers()
{
   string Final = _config->FindDir("Dir::State::lists");
   Final += URItoFileName(RealURI);
   
   struct stat Buf;
   if (stat(Final.c_str(),&Buf) != 0)
      return "\nIndex-File: true";

   // CNC:2002-07-11
   string LOI = "";
   if (Master == true)
      LOI = "\nLocal-Only-IMS: true";
   return LOI + "\nIndex-File: true\nLast-Modified: " + TimeRFC1123(Buf.st_mtime);
}
									/*}}}*/
// AcqIndexRel::Done - Item downloaded OK				/*{{{*/
// ---------------------------------------------------------------------
/* The release file was not placed into the download directory then
   a copy URI is generated and it is copied there otherwise the file
   in the partial directory is moved into .. and the URI is finished. */
void pkgAcqIndexRel::Done(string Message,unsigned long Size,string MD5,
			  pkgAcquire::MethodConfig *Cfg)
{
   Item::Done(Message,Size,MD5,Cfg);

   // CNC:2002-07-03
   if (Authentication == true) 
   {
      if (Repository->IsAuthenticated() == true)
      {
	 // Do the fingerprint matching magic
	 string FingerPrint = LookupTag(Message,"Signature-Fingerprint");

	 if (FingerPrint.empty() == true)
	 {
	    Status = StatError;
	    ErrorText = _("No valid signatures found in Release file");
	    return;
	 }

 	 bool found = false;
 	 for (vector<string>::const_iterator I = Repository->FingerPrint.begin();
 	      I != Repository->FingerPrint.end(); I++)
 	 {
 	    // Match fingerprint of Release file
 	    if ((*I) == FingerPrint)
 	    {
 	       found = true;
 	       break;
 	    }
 	 }
 
 	 if (!found)
 	 {
 	    Status = StatError;
 	    ErrorText = _("Signature fingerprint of Release file does not match (expected ");
 	    for (vector<string>::const_iterator I = Repository->FingerPrint.begin();
 		 I != Repository->FingerPrint.end(); I++)
 	      ErrorText += "\n"+(*I);
 	    ErrorText += _(", got ")+FingerPrint+")";
  	    return;
  	 }

      }

      // Done, move it into position
      string FinalFile = _config->FindDir("Dir::State::lists");
      FinalFile += URItoFileName(RealURI);
      Rename(DestFile,FinalFile);
      chmod(FinalFile.c_str(),0644);

      /* We restore the original name to DestFile so that the clean operation
         will work OK */
      DestFile = _config->FindDir("Dir::State::lists") + "partial/";
      DestFile += URItoFileName(RealURI);
      
      // Remove the compressed version.
      if (Erase == true)
	 unlink(DestFile.c_str());

      // Update the hashes and file sizes for this repository
      if (Repository->ParseRelease(FinalFile) == false && 
	  Repository->IsAuthenticated() == true)
      {
         Status = StatError;
         ErrorText = _("Could not read checksum list from Release file");
      }
      return;
   }
   
   string FileName = LookupTag(Message,"Filename");
   if (FileName.empty() == true)
   {
      Status = StatError;
      ErrorText = "Method gave a blank filename";
      return;
   }

   // CNC:2002-07-11
   Erase = false;
   Complete = true;
   
   // The files timestamp matches
   if (StringToBool(LookupTag(Message,"IMS-Hit"),false) == true)
   {
      // CNC:2002-07-11
      if (Master == true)
      {
	 // We've got a LocalOnly IMS
	 string FinalFile = _config->FindDir("Dir::State::lists");
	 FinalFile += URItoFileName(RealURI);
	 Repository->ParseRelease(FinalFile);
      }
      return;
   }
   
   // We have to copy it into place
   if (FileName != DestFile)
   {
      Local = true;
      Desc.URI = "copy:" + FileName;
      QueueURI(Desc);
      return;
   }
   
   // CNC:2002-07-03
   unsigned long FSize;
   string MD5Hash;
   if (Master == false && Repository != NULL
       && Repository->HasRelease() == true
       && Repository->FindChecksums(RealURI,FSize,MD5Hash) == true)
   {
      if (FSize != Size)
      {
	 Status = StatError;
	 ErrorText = _("Size mismatch");
	 Rename(DestFile,DestFile + ".FAILED");
	 if (_config->FindB("Acquire::Verbose",false) == true) 
	    _error->Warning("Size mismatch of index file %s: %lu was supposed to be %lu",
			    RealURI.c_str(), Size, FSize);
	 return;
      }
      if (MD5.empty() == false && MD5Hash != MD5)
      {
	 Status = StatError;
	 ErrorText = _("MD5Sum mismatch");
	 Rename(DestFile,DestFile + ".FAILED");
	 if (_config->FindB("Acquire::Verbose",false) == true) 
	    _error->Warning("MD5Sum mismatch of index file %s: %s was supposed to be %s",
			    RealURI.c_str(), MD5.c_str(), MD5Hash.c_str());
	 return;
      }
   }

   if (Master == false || Repository->IsAuthenticated() == false)
   {
      // Done, move it into position
      string FinalFile = _config->FindDir("Dir::State::lists");
      FinalFile += URItoFileName(RealURI);
      Rename(DestFile,FinalFile);
      chmod(FinalFile.c_str(),0644);

      // extract checksums from the Release file
      if (Master == true)
	 Repository->ParseRelease(FinalFile);
   }
   else 
   {
      if (FileName == DestFile)
	 Erase = true;
      else
	 Local = true;
   
      // Still have the authentication phase
      Authentication = true;
      DestFile += ".auth";
      Desc.URI = "gpg:" + FileName;
      QueueURI(Desc);
      Mode = "gpg";
   }
}
									/*}}}*/
// AcqIndexRel::Failed - Silence failure messages for missing rel files	/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqIndexRel::Failed(string Message,pkgAcquire::MethodConfig *Cnf)
{
   if (Cnf->LocalOnly == true || 
       StringToBool(LookupTag(Message,"Transient-Failure"),false) == false)
   {      
      // CNC:2002-07-03
      if (Master == false || Repository->IsAuthenticated() == false)
      {
         // Ignore this
	 Status = StatDone;
	 Complete = false;
	 Dequeue();
	 return;
      }
   }
   
   Item::Failed(Message,Cnf);
}
									/*}}}*/

// AcqArchive::AcqArchive - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* This just sets up the initial fetch environment and queues the first
   possibilitiy */
pkgAcqArchive::pkgAcqArchive(pkgAcquire *Owner,pkgSourceList *Sources,
			     pkgRecords *Recs,pkgCache::VerIterator const &Version,
			     string &StoreFilename) :
               Item(Owner), Version(Version), Sources(Sources), Recs(Recs), 
               StoreFilename(StoreFilename), Vf(Version.FileList())
{
   Retries = _config->FindI("Acquire::Retries",0);

   if (Version.Arch() == 0)
   {
      _error->Error(_("I wasn't able to locate a file for the %s package. "
		      "This might mean you need to manually fix this package. "
		      "(due to missing arch)"),
		    Version.ParentPkg().Name());
      return;
   }
   
   /* We need to find a filename to determine the extension. We make the
      assumption here that all the available sources for this version share
      the same extension.. */
   // Skip not source sources, they do not have file fields.
   for (; Vf.end() == false; Vf++)
   {
      if ((Vf.File()->Flags & pkgCache::Flag::NotSource) != 0)
	 continue;
      break;
   }
   
   // Does not really matter here.. we are going to fail out below
   if (Vf.end() != true)
   {     
      // If this fails to get a file name we will bomb out below.
      pkgRecords::Parser &Parse = Recs->Lookup(Vf);
      if (_error->PendingError() == true)
	 return;
            
      // Generate the final file name as: package_version_arch.foo
      std::ostringstream BTss;
      BTss << Version.BTime();

      StoreFilename = QuoteString(Version.ParentPkg().Name(),"_:") + '_' +
	              QuoteString(Version.VerStr(),"_:") + '_' +
     	              QuoteString(Version.Arch(),"_:.") + '_' +
		      QuoteString(BTss.str(),"_") +
	              "." + flExtension(Parse.FileName());
   }
      
   // Select a source
   if (QueueNext() == false && _error->PendingError() == false)
      _error->Error(_("I wasn't able to locate file for the %s package. "
		    "This might mean you need to manually fix this package."),
		    Version.ParentPkg().Name());
}
									/*}}}*/
// AcqArchive::QueueNext - Queue the next file source			/*{{{*/
// ---------------------------------------------------------------------
/* This queues the next available file version for download. It checks if
   the archive is already available in the cache and stashs the MD5 for
   checking later. */
bool pkgAcqArchive::QueueNext()
{   
   for (; Vf.end() == false; Vf++)
   {
      // Ignore not source sources
      if ((Vf.File()->Flags & pkgCache::Flag::NotSource) != 0)
	 continue;

      // Try to cross match against the source list
      pkgIndexFile *Index;
      if (Sources->FindIndex(Vf.File(),Index) == false)
	    continue;
      
      // Grab the text package record
      pkgRecords::Parser &Parse = Recs->Lookup(Vf);
      if (_error->PendingError() == true)
	 return false;
      
      string PkgFile = Parse.FileName();
      MD5 = Parse.MD5Hash();
      if (PkgFile.empty() == true)
	 return _error->Error(_("The package index files are corrupted. No Filename: "
			      "field for package %s."),
			      Version.ParentPkg().Name());

      // See if we already have the file. (Legacy filenames)
      FileSize = Version->Size;
      string FinalFile = _config->FindDir("Dir::Cache::Archives") + flNotDir(PkgFile);
      struct stat Buf;
      if (stat(FinalFile.c_str(),&Buf) == 0)
      {
	 // Make sure the size matches
	 if ((unsigned)Buf.st_size == Version->Size)
	 {
	    Complete = true;
	    Local = true;
	    Status = StatDone;
	    StoreFilename = DestFile = FinalFile;
	    return true;
	 }
	 
	 /* Hmm, we have a file and its size does not match, this means it is
	    an old style mismatched arch */
	 unlink(FinalFile.c_str());
      }

      // Check it again using the new style output filenames
      FinalFile = _config->FindDir("Dir::Cache::Archives") + flNotDir(StoreFilename);
      if (stat(FinalFile.c_str(),&Buf) == 0)
      {
	 // Make sure the size matches
	 if ((unsigned)Buf.st_size == Version->Size)
	 {
	    Complete = true;
	    Local = true;
	    Status = StatDone;
	    StoreFilename = DestFile = FinalFile;
	    return true;
	 }
	 
	 /* Hmm, we have a file and its size does not match, this shouldnt
	    happen.. */
	 unlink(FinalFile.c_str());
      }

      DestFile = _config->FindDir("Dir::Cache::Archives") + "partial/" + flNotDir(StoreFilename);
      
      // Check the destination file
      if (stat(DestFile.c_str(),&Buf) == 0)
      {
	 // Hmm, the partial file is too big, erase it
	 if ((unsigned)Buf.st_size > Version->Size)
	    unlink(DestFile.c_str());
	 else
	    PartialSize = Buf.st_size;
      }
      
      // Create the item
      Local = false;
      Desc.URI = Index->ArchiveURI(PkgFile);
      Desc.Description = Index->ArchiveInfo(Version);
      Desc.Owner = this;
      Desc.ShortDesc = Version.ParentPkg().Name();
      QueueURI(Desc);

      Vf++;
      return true;
   }
   return false;
}   
									/*}}}*/

// CNC:2003-03-19
#ifdef WITH_LUA
// ScriptsAcquireDone - Script trigger.					/*{{{*/
// ---------------------------------------------------------------------
/* */
template<class T>
static void ScriptsAcquireDone(const char *ConfKey,
			       string &StoreFilename,
			       string &ErrorText,
			       T &Status)
{
   if (_lua->HasScripts(ConfKey) == true) {
      _lua->SetGlobal("acquire_filename", StoreFilename.c_str());
      _lua->SetGlobal("acquire_error", (const char *)NULL);
      _lua->RunScripts(ConfKey, true);
      const char *Error = _lua->GetGlobalStr("acquire_error");
      if (Error != NULL && *Error != 0) {
	 Status = pkgAcquire::Item::StatError;
	 ErrorText = Error;
      }
      _lua->ResetGlobals();
   }
}
									/*}}}*/
#endif

// AcqArchive::Done - Finished fetching					/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqArchive::Done(string Message,unsigned long Size,string Md5Hash,
			 pkgAcquire::MethodConfig *Cfg)
{
   Item::Done(Message,Size,Md5Hash,Cfg);
   
   // Check the size
   if (Size != Version->Size)
   {
      Status = StatError;
      ErrorText = _("Size mismatch");
      return;
   }
   
   // Check the md5
   if (Md5Hash.empty() == false && MD5.empty() == false)
   {
      if (Md5Hash != MD5)
      {
	if (_config->FindB("Debug::pkgAcquire::Auth", false)) {
	    cerr << "md5 mismatch: " << Md5Hash << "!=" << MD5 << endl;
	}
 	 Status = StatError;
	 ErrorText = _("MD5Sum mismatch");
	 Rename(DestFile,DestFile + ".FAILED");
	 return;
      }
   }

   // Grab the output filename
   string FileName = LookupTag(Message,"Filename");
   if (FileName.empty() == true)
   {
      Status = StatError;
      ErrorText = "Method gave a blank filename";
      return;
   }

   Complete = true;

   // Reference filename
   if (FileName != DestFile)
   {
      StoreFilename = DestFile = FileName;
      Local = true;

// CNC:2003-03-19
#ifdef WITH_LUA
      ScriptsAcquireDone("Scripts::Acquire::Archive::Done",
			 StoreFilename, ErrorText, Status);
#endif

      return;
   }
   
   // Done, move it into position
   string FinalFile = _config->FindDir("Dir::Cache::Archives");
   FinalFile += flNotDir(StoreFilename);
   Rename(DestFile,FinalFile);
   
   StoreFilename = DestFile = FinalFile;
   Complete = true;

// CNC:2003-03-19
#ifdef WITH_LUA
   ScriptsAcquireDone("Scripts::Acquire::Archive::Done",
		      StoreFilename, ErrorText, Status);
#endif

}
									/*}}}*/
// AcqArchive::Failed - Failure handler					/*{{{*/
// ---------------------------------------------------------------------
/* Here we try other sources */
void pkgAcqArchive::Failed(string Message,pkgAcquire::MethodConfig *Cnf)
{
   ErrorText = LookupTag(Message,"Message");
   
   /* We don't really want to retry on failed media swaps, this prevents 
      that. An interesting observation is that permanent failures are not
      recorded. */
   if (Cnf->Removable == true && 
       StringToBool(LookupTag(Message,"Transient-Failure"),false) == true)
   {
      // Vf = Version.FileList();
      while (Vf.end() == false) Vf++;
      StoreFilename = string();
      Item::Failed(Message,Cnf);
      return;
   }
   
   if (QueueNext() == false)
   {
      // This is the retry counter
      if (Retries != 0 &&
	  Cnf->LocalOnly == false &&
	  StringToBool(LookupTag(Message,"Transient-Failure"),false) == true)
      {
	 Retries--;
	 Vf = Version.FileList();
	 if (QueueNext() == true)
	    return;
      }
      
      StoreFilename = string();
      Item::Failed(Message,Cnf);
   }
}
									/*}}}*/
// AcqArchive::Finished - Fetching has finished, tidy up		/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqArchive::Finished()
{
   if (Status == pkgAcquire::Item::StatDone &&
       Complete == true)
      return;
   StoreFilename = string();
}
									/*}}}*/

// AcqFile::pkgAcqFile - Constructor					/*{{{*/
// ---------------------------------------------------------------------
/* The file is added to the queue */
pkgAcqFile::pkgAcqFile(pkgAcquire *Owner,string URI,string MD5,
		       unsigned long Size,string Dsc,string ShortDesc) :
                       Item(Owner), Md5Hash(MD5)
{
   Retries = _config->FindI("Acquire::Retries",0);
   
   DestFile = flNotDir(URI);
   
   // Create the item
   Desc.URI = URI;
   Desc.Description = Dsc;
   Desc.Owner = this;

   // Set the short description to the archive component
   Desc.ShortDesc = ShortDesc;
      
   // Get the transfer sizes
   FileSize = Size;
   struct stat Buf;
   if (stat(DestFile.c_str(),&Buf) == 0)
   {
      // Hmm, the partial file is too big, erase it
      if ((unsigned)Buf.st_size > Size)
	 unlink(DestFile.c_str());
      else
	 PartialSize = Buf.st_size;
   }
   
   QueueURI(Desc);
}
									/*}}}*/
// AcqFile::Done - Item downloaded OK					/*{{{*/
// ---------------------------------------------------------------------
/* */
void pkgAcqFile::Done(string Message,unsigned long Size,string MD5,
		      pkgAcquire::MethodConfig *Cnf)
{
   // Check the md5
   if (Md5Hash.empty() == false && MD5.empty() == false)
   {
      if (Md5Hash != MD5)
      {
	if (_config->FindB("Debug::pkgAcquire::Auth", false)) {
	    cerr << "md5 mismatch: " << Md5Hash << "!=" << MD5 << endl;
	}
	 Status = StatError;
	 ErrorText = "MD5Sum mismatch";
	 Rename(DestFile,DestFile + ".FAILED");
	 return;
      }
   }
   
   Item::Done(Message,Size,MD5,Cnf);

   string FileName = LookupTag(Message,"Filename");
   if (FileName.empty() == true)
   {
      Status = StatError;
      ErrorText = "Method gave a blank filename";
      return;
   }

   Complete = true;
   
   // The files timestamp matches
   if (StringToBool(LookupTag(Message,"IMS-Hit"),false) == true)
      return;
   
   // We have to copy it into place
   if (FileName != DestFile)
   {
      Local = true;
      if (_config->FindB("Acquire::Source-Symlinks",true) == false ||
	  Cnf->Removable == true)
      {
	 Desc.URI = "copy:" + FileName;
	 QueueURI(Desc);
	 return;
      }
      
      // Erase the file if it is a symlink so we can overwrite it
      struct stat St;
      if (lstat(DestFile.c_str(),&St) == 0)
      {
	 if (S_ISLNK(St.st_mode) != 0)
	    unlink(DestFile.c_str());
	 // CNC:2003-12-11 - Check if FileName == DestFile
	 else {
	    struct stat St2;
	    if (stat(FileName.c_str(), &St2) == 0
	        && St.st_ino == St2.st_ino)
	       return;
	 }
      }
      
      // Symlink the file
      if (symlink(FileName.c_str(),DestFile.c_str()) != 0)
      {
	 ErrorText = "Link to " + DestFile + " failure ";
	 Status = StatError;
	 Complete = false;
      }      
   }
}
									/*}}}*/
// AcqFile::Failed - Failure handler					/*{{{*/
// ---------------------------------------------------------------------
/* Here we try other sources */
void pkgAcqFile::Failed(string Message,pkgAcquire::MethodConfig *Cnf)
{
   ErrorText = LookupTag(Message,"Message");
   
   // This is the retry counter
   if (Retries != 0 &&
       Cnf->LocalOnly == false &&
       StringToBool(LookupTag(Message,"Transient-Failure"),false) == true)
   {
      Retries--;
      QueueURI(Desc);
      return;
   }
   
   Item::Failed(Message,Cnf);
}
									/*}}}*/
// vim:sts=3:sw=3
