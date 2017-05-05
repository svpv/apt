// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: gzip.cc,v 1.17 2003/02/10 07:34:41 doogie Exp $
/* ######################################################################

   GZip method - Take a file URI in and decompress it into the target 
   file.
   
   ##################################################################### */
									/*}}}*/
// Include Files							/*{{{*/
#include <config.h>

#include <apt-pkg/fileutl.h>
#include <apt-pkg/error.h>
#include <apt-pkg/acquire-method.h>
#include <apt-pkg/strutl.h>
#include <apt-pkg/hashes.h>

#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <stdio.h>
#include <errno.h>
#include <zpkglist.h>

// CNC:2003-02-20 - Moved header to fix compilation error when
// 		    --disable-nls is used.
#include <apti18n.h>
									/*}}}*/

const char *Prog;

class GzipMethod : public pkgAcqMethod
{
   virtual bool Fetch(FetchItem *Itm);
   
   public:
   
   GzipMethod() : pkgAcqMethod("1.1",SingleInstance | SendConfig) {};
};


struct zHashState {
   Hashes *hash;
   unsigned long total;
};

static void zHash(void *buf, unsigned size, void *arg)
{
   struct zHashState *z = (struct zHashState *) arg;
   z->hash->Add((unsigned char *) buf, size);
   z->total += size;
}

static bool endswith(const char *str, const char *suffix)
{
   size_t len1 = strlen(str);
   size_t len2 = strlen(suffix);
   if (len1 < len2)
      return false;
   str += (len1 - len2);
   return memcmp(str, suffix, len2) == 0;
}

// GzipMethod::Fetch - Decompress the passed URI			/*{{{*/
// ---------------------------------------------------------------------
/* */
bool GzipMethod::Fetch(FetchItem *Itm)
{
   URI Get = Itm->Uri;
   string Path = Get.Host + Get.Path; // To account for relative paths
   
   string GzPathOption = "Dir::bin::"+string(Prog);

   FetchResult Res;
   Res.Filename = Itm->DestFile;
   URIStart(Res);
   
   // Open the source and destination files
   FileFd From(Path,FileFd::ReadOnly);

   int GzOut[2];   
   if (pipe(GzOut) < 0)
      return _error->Errno("pipe",_("Couldn't open pipe for %s"),Prog);

   // Fork gzip
   int Process = ExecFork();
   if (Process == 0)
   {
      close(GzOut[0]);
      dup2(From.Fd(),STDIN_FILENO);
      dup2(GzOut[1],STDOUT_FILENO);
      From.Close();
      close(GzOut[1]);
      SetCloseExec(STDIN_FILENO,false);
      SetCloseExec(STDOUT_FILENO,false);
      
      const char *Args[3];
      string Tmp = _config->Find(GzPathOption,Prog);
      Args[0] = Tmp.c_str();
      Args[1] = "-d";
      Args[2] = 0;
      execvp(Args[0],(char **)Args);
      _exit(100);
   }
   From.Close();
   close(GzOut[1]);
   
   FileFd FromGz(GzOut[0]);  // For autoclose   
   FileFd To(Itm->DestFile,FileFd::WriteEmpty);   
   To.EraseOnFailure();
   if (_error->PendingError() == true)
      return false;
   
   // Read data from gzip, generate checksums and write
   Hashes Hash;
   bool Failed = false;

   struct zHashState z = { &Hash, 0 };
   bool Recompress = endswith(Res.Filename.c_str(), ".zpkglist");
   if (Recompress) {
      if (!zpkglistCompress(GzOut[0], To.Fd(), zHash, &z))
	 Failed = true;
      goto skipLoop;
   }

   while (1) 
   {
      unsigned char Buffer[4*1024];
      unsigned long Count;
      
      Count = read(GzOut[0],Buffer,sizeof(Buffer));
      if (Count < 0 && errno == EINTR)
	 continue;
      
      if (Count < 0)
      {
	 _error->Errno("read", _("Read error from %s process"),Prog);
	 Failed = true;
	 break;
      }
      
      if (Count == 0)
	 break;
      
      Hash.Add(Buffer,Count);
      if (To.Write(Buffer,Count) == false)
      {
	 Failed = true;
	 break;
      }      
   }
   
skipLoop:
   // Wait for gzip to finish
   if (ExecWait(Process,_config->Find(GzPathOption,Prog).c_str(),false) == false)
   {
      To.OpFail();
      return false;
   }  
       
   To.Close();
   
   if (Failed == true)
      return false;
   
   // Transfer the modification times
   struct stat Buf;
   if (stat(Path.c_str(),&Buf) != 0)
      return _error->Errno("stat",_("Failed to stat"));

   struct utimbuf TimeBuf;
   TimeBuf.actime = Buf.st_atime;
   TimeBuf.modtime = Buf.st_mtime;
   if (utime(Itm->DestFile.c_str(),&TimeBuf) != 0)
      return _error->Errno("utime",_("Failed to set modification time"));

   if (stat(Itm->DestFile.c_str(),&Buf) != 0)
      return _error->Errno("stat",_("Failed to stat"));
   
   // Return a Done response
   Res.LastModified = Buf.st_mtime;
   Res.Size = Recompress ? z.total : Buf.st_size;
   Res.TakeHashes(Hash);

   URIDone(Res);
   
   return true;
}
									/*}}}*/

int main(int argc, char *argv[])
{
   GzipMethod Mth;

   Prog = strrchr(argv[0],'/');
   Prog++;
   
   return Mth.Run();
}
