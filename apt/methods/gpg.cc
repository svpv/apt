
#include <apt-pkg/error.h>
#include <apt-pkg/acquire-method.h>
#include <apt-pkg/strutl.h>

#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <stdio.h>
#include <sys/wait.h>


class GPGMethod : public pkgAcqMethod
{
   virtual bool Fetch(FetchItem *Itm);
   
   public:
   
   GPGMethod() : pkgAcqMethod("1.0",SingleInstance | SendConfig) {};
};



static char *getFileSigner(const char *file, const char *outfile, 
			   string &signerKeyID, bool useDefaultPubring)
{
   pid_t pid;
   int fd[2];
   char buffer[1024];
   FILE *f;
   char keyid[64];
   int status;
   bool goodsig = false;

   if (pipe(fd) < 0) {
      return "could not create pipe";
   }
      
   pid = fork();
   if (pid < 0) {
      return "could not spawn new process";
   } else if (pid == 0) {
      string path = _config->Find("Dir::Bin::gpg");
// New option to provide more flexible way to specify
// public ring to check signatures      
      string pubringpath = _config->Find("Apt::GPG::PubringPath");

      close(fd[0]);
      close(STDERR_FILENO);
      close(STDOUT_FILENO);
      dup2(fd[1], STDOUT_FILENO);
      dup2(fd[1], STDERR_FILENO);

      putenv("LANG=");
      putenv("LC_ALL=");
      putenv("LC_MESSAGES=");
      
      if((pubringpath.length() > 0) && useDefaultPubring) {
          execlp(path.c_str(), "gpg", "--batch", "--no-secmem-warning", "--homedir", 
    	         pubringpath.c_str(), "--status-fd", "2", "-o", outfile, file, NULL);
      } else {
          execlp(path.c_str(), "gpg", "--batch", "--no-secmem-warning",
		 "--status-fd", "2", "-o", outfile, file, NULL);
      }
             
      exit(111);
   }
   close(fd[1]);
   keyid[0] = 0;
   goodsig = false;

   f = fdopen(fd[0], "r");
   
   while (1) {
      char *ptr, *ptr1;
      
      if (!fgets(buffer, 1024, f))
	  break;
      
      if (goodsig && keyid[0]) {
	  continue;
      }
      
// Use strstr because response likely to be further than buffer start
#define SIGPACK "[GNUPG:] VALIDSIG"
      if ((ptr1 = strstr(buffer, SIGPACK)) != NULL) {
	 char *sig;
	 ptr = sig = ptr1 + sizeof(SIGPACK);
	 while (isxdigit(*ptr)) ptr++;
	 *ptr = 0;
	 strcpy(keyid, sig);
      }
#undef SIGPACK
      
#define GOODSIG "[GNUPG:] GOODSIG"
      if ((ptr1 = strstr(buffer, GOODSIG)) != NULL) {
	  goodsig = true;
      }
#undef GOODSIG      
   }
   fclose(f);

   waitpid(pid, &status, 0);
   
   if (WEXITSTATUS(status) == 0) {
      signerKeyID = string(keyid);
      return NULL;
   } else if (WEXITSTATUS(status) == 111) {
      return "could not execute gpg to verify signature";
   } else {
      if (!goodsig || !keyid[0])
	  return "file was not signed with a known key. Check if the proper gpg key was imported to your keyring.";

      return "file could not be authenticated";
   }
}


bool GPGMethod::Fetch(FetchItem *Itm)
{
   URI Get = Itm->Uri;
   string Path = Get.Host + Get.Path; // To account for relative paths
   string keyID;
   
   FetchResult Res;
   Res.Filename = Itm->DestFile;
   URIStart(Res);

   // Run GPG on file, extract contents and get the key ID of the signer
   // First use default public ring provided by packaging engine (IPL feature)
   char *msg = getFileSigner(Path.c_str(), Itm->DestFile.c_str(), keyID, true);
   if (msg) {
   // Run GPG on file, extract contents and get the key ID of the signer
   // Now use user's public ring
       char *msg1 = getFileSigner(Path.c_str(), Itm->DestFile.c_str(), keyID, false);
       if (msg1) {
          return _error->Error(msg1);
       }
   }

      
   // Transfer the modification times
   struct stat Buf;
   if (stat(Path.c_str(),&Buf) != 0)
      return _error->Errno("stat","Failed to stat ", Path.c_str());

   struct utimbuf TimeBuf;
   TimeBuf.actime = Buf.st_atime;
   TimeBuf.modtime = Buf.st_mtime;
   if (utime(Itm->DestFile.c_str(),&TimeBuf) != 0)
      return _error->Errno("utime","Failed to set modification time");

   if (stat(Itm->DestFile.c_str(),&Buf) != 0)
      return _error->Errno("stat","Failed to stat");
   
   // Return a Done response
   Res.LastModified = Buf.st_mtime;
   Res.Size = Buf.st_size;
   Res.SignatureKeyID = keyID;
   URIDone(Res);

   return true;
}


int main()
{
   GPGMethod Mth;

   return Mth.Run();
}
