/*
 * $Id: gensrclist.cc,v 1.11 2001/12/12 14:50:43 kojima Exp $
 *
 * $Log: gensrclist.cc,v $
 * Revision 1.11  2001/12/12 14:50:43  kojima
 * fixed genbasedir for --flat and relative --topdir specifications
 * fixed --flat in gensrclist
 *
 * Revision 1.10  2001/12/11 13:51:03  kojima
 * added --flat option to srclist, patched gensrclist with stelian's patch
 *
 * Revision 1.9  2001/11/09 21:11:46  kojima
 * * Use 'scandir' for directory traversal (instead of opendir/
 *  readdir) like in genpkglist.
 * * Better package progression indicator.
 *
 * Revision 1.8  2001/08/07 20:46:03  kojima
 * Alexander Bokovoy <a.bokovoy@sam-solutions.net>'s patch for cleaning
 * up genpkglist
 *
 * Revision 1.7  2001/07/12 21:47:33  kojima
 * ignore duplicated version/diff deps packages
 * new release (cnc51)
 *
 * Revision 1.6  2000/11/06 12:53:49  kojima
 * fixed compile errors for RedHat 6.x (with gcc -Wall -Werror)
 *
 * Revision 1.5  2000/11/01 21:32:28  kojima
 * added manpage
 *
 * Revision 1.3  2000/10/30 02:17:17  kojima
 * fixed bugs in source d/l
 *
 * Revision 1.2  2000/10/29 20:25:10  kojima
 * added support for source download
 *
 * Revision 1.1  2000/10/28 02:23:17  kojima
 * started source support
 *
 */
#include <alloca.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <rpm/rpmlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <map>
#include <slist>

#include <apt-pkg/error.h>
#include <apt-pkg/tagfile.h>
#include <apt-pkg/rpminit.h>

#include "cached_md5.h"





int tags[] =  {
   RPMTAG_NAME,
       RPMTAG_EPOCH,
       RPMTAG_VERSION,
       RPMTAG_RELEASE,
       RPMTAG_GROUP,
       RPMTAG_ARCH,
       RPMTAG_PACKAGER,
       RPMTAG_SOURCERPM,
       RPMTAG_SIZE,
       RPMTAG_VENDOR,
       
       RPMTAG_DESCRIPTION, 
       RPMTAG_SUMMARY, 
       
       RPMTAG_REQUIREFLAGS, 
       RPMTAG_REQUIRENAME,
       RPMTAG_REQUIREVERSION
};
int numTags = sizeof(tags) / sizeof(int);

int selectDirent(const struct dirent *ent)
{
   int state = 0;
   const char *p = ent->d_name;
   
   while (1) {
      if (*p == '.') {
	  state = 1;
      } else if (state == 1 && *p == 'r')
	  state++;
      else if (state == 2 && *p == 'p')
	  state++;
      else if (state == 3 && *p == 'm')
	  state++;
      else if (state == 4 && *p == '\0')
	  return 1;
      else if (*p == '\0')
	  return 0;
      else
	  state = 0;
      p++;
   }
}

bool readRPMTable(char *file, map<string, slist<char*>* > &table)
{
   FILE *indexf;
   char buf[512];
   string srpm;
   
   indexf = fopen(file, "r");
   if (!indexf) {
      cerr << "gensrclist: could not open file " << file << " for reading: "
	  << strerror(errno) << endl;
      return false;
   }
   
   while (fgets(buf, 512, indexf)) {
      char *f;
      
      buf[strlen(buf)-1] = '\0';
      f = strchr(buf, ' ');
      *f = '\0';
      f++;
      
      srpm = string(buf);
      
      if (table.find(srpm) != table.end()) {
	 slist<char*> *list = table[srpm];
	 
	 list->push_front(strdup(f));
      } else {
	 slist<char*> *list = new slist<char*>;
	 
	 list->push_front(strdup(f));
	 table[srpm] = list;
      }
   }
   
   fclose(indexf);
   
   return true;
}


void usage()
{
   cerr << "usage: gensrclist [<options>] <dir> <suffix> <srpm index>" << endl;
   cerr << "options:" << endl;
//   cerr << " --mapi         ???????????????????" << endl;
   cerr << " --progress     show a progress bar" << endl;
   cerr << " --flat         use a flat directory structure, where RPMS and SRPMS"<<endl;
   cerr << "                are in the same directory level"<<endl;
}

int main(int argc, char ** argv) 
{
   char buf[300];
   char cwd[200];
   string srpmdir;
   DIR * dir;
   FD_t outfd, fd;
   struct dirent **dirEntries;
   int rc, i;
   Header h;
   Header sigs;
   struct stat sb;
   int_32 size[1];
   int entry_no, entry_cur;
   int jj;
   char *suffix;
   char *directory;
   char *index;
   CachedMD5 *md5cache;
   map<string, slist<char*>* > rpmTable; // table that maps srpm -> generated rpm
   bool mapi = false;
   bool progressBar = false;
   bool flatStructure = false;
   char *arg_dir, *arg_suffix, *arg_srpmindex;

   putenv("LC_ALL=");
   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--mapi") == 0) {
	 mapi = true;
      } else if (strcmp(argv[i], "--flat") == 0) {
	 flatStructure = true;
      } else if (strcmp(argv[i], "--progress") == 0) {
	 progressBar = true;
      } else {
	 break;
      }
   }
   if (argc - i == 3) {
      arg_dir = argv[i++];
      arg_suffix = argv[i++];
      arg_srpmindex = argv[i++];
   }
   else {
      usage();
      exit(1);
   }
   
   if (!readRPMTable(arg_srpmindex, rpmTable))
       exit(1);
   
   md5cache = new CachedMD5(string(arg_dir)+string(arg_suffix));
   
   getcwd(cwd, 200);
   if (*arg_dir != '/') {
      strcpy(buf, cwd);
      strcat(buf, "/");
      strcat(buf, arg_dir);
   } else
       strcpy(buf, arg_dir);
   
   strcat(buf, "/SRPMS.");
   strcat(buf, arg_suffix);
   
   srpmdir = "SRPMS." + string(arg_suffix);
   if (flatStructure) {
      char *prefix;
      // add the last component of the directory to srpmdir
      // that will cancel the effect of the .. used in sourcelist.cc
      // when building the directory from where to fetch srpms in apt
      
      prefix = strrchr(arg_dir, '/');
      if (!prefix)
	  prefix = arg_dir;
      else
	  prefix++;
      
      srpmdir = string(prefix) + '/' + srpmdir;
   }
   
   entry_no = scandir(buf, &dirEntries, selectDirent, alphasort);
   if (entry_no < 0) { 
      cerr << "gensrclist: error opening directory " << buf << ":"
	  << strerror(errno) << endl;
      return 1;
   }

   chdir(buf);
   
   
   
   sprintf(buf, "%s/srclist.%s", cwd, arg_suffix);
   
   unlink(buf);
   
   outfd = fdOpen(buf, O_WRONLY | O_TRUNC | O_CREAT, 0644);
   if (!outfd) {
      cerr << "gensrclist: error creating file" << buf << ":"
	  << strerror(errno);
      return 1;
   }
  
   for (entry_cur = 0; entry_cur < entry_no; entry_cur++) {
      struct stat sb;

      if (progressBar) {
         if (entry_cur)
            printf("\b\b\b\b\b\b\b\b\b\b");
         printf("%04i/%04i ", entry_cur + 1, entry_no);
         fflush(stdout);
      }

      if (stat(dirEntries[entry_cur]->d_name, &sb) < 0) {
          cerr << dirEntries[entry_cur] << ":";
          perror("stat");
          exit(1);
      }
      
      fd = fdOpen(dirEntries[entry_cur]->d_name, O_RDONLY, 0666);
	 
      if (!fd) {
	  cerr << dirEntries[entry_cur]->d_name << ":";
	  perror("open");
	  exit(1);
      }
	 
      size[0] = sb.st_size;
	 
      rc = rpmReadPackageInfo(fd, &sigs, &h);
	 
      if (!rc) {
	    Header newHeader;
	    int i;
	    bool foundInIndex;
	    
	    newHeader = headerNew();
	    
	    // the std tags
	    for (i = 0; i < numTags; i++) {
	       int type, count;
	       void *data;
	       int res;
	       
	       res = headerGetEntry(h, tags[i], &type, &data, &count);
	       if (res != 1) {
		  /*
		   printf("warning: tag %i not found on header for %s\n",
		   tags[i], dirEntries[entry_cur]->d_name);
		   */
		  continue;
	       }
	       headerAddEntry(newHeader, tags[i], type, data, count);
	    }
	    
	    
	    // our additional tags
	    headerAddEntry(newHeader, CRPMTAG_DIRECTORY, RPM_STRING_TYPE,
			   srpmdir.c_str(), 1);
	    
	    headerAddEntry(newHeader, CRPMTAG_FILENAME, RPM_STRING_TYPE, 
			   dirEntries[entry_cur]->d_name, 1);
	    headerAddEntry(newHeader, CRPMTAG_FILESIZE, RPM_INT32_TYPE,
			   size, 1);
	    
	    {
	       unsigned char md5[34];
	       
	       md5cache->MD5ForFile(dirEntries[entry_cur]->d_name, sb.st_mtime, md5);
	       
	       headerAddEntry(newHeader, CRPMTAG_MD5, RPM_STRING_TYPE,
			      md5, 1);
	    }
	    
	    foundInIndex = false;
	    {
	       int count = 0;
	       char **list = NULL;
	       slist<char*> *rpmlist = rpmTable[string(dirEntries[entry_cur]->d_name)];
	       
	       if (rpmlist) {
		  list = new char *[rpmlist->size()];
		  
		  foundInIndex = true;
		  
		  for (slist<char*>::const_iterator i = rpmlist->begin();
		       i != rpmlist->end();
		       i++) {
		     list[count++] = *i;
		  }
	       }
	       
	       if (count) {
		  headerAddEntry(newHeader, CRPMTAG_BINARY,
				 RPM_STRING_ARRAY_TYPE, list, count);
	       }
	    }
	    if (foundInIndex || !mapi)
		headerWrite(outfd, newHeader, HEADER_MAGIC_YES);
	    
	    headerFree(newHeader);
	    headerFree(h);
	    rpmFreeSignature(sigs);
      }
      fdClose(fd);
   } 
   
   fdClose(outfd);
   
   delete md5cache;
   
   return 0;
}
