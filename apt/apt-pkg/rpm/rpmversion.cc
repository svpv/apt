// $Id: rpmversion.cc,v 1.4 2002/01/14 16:53:12 kojima Exp $

#ifdef __GNUG__
#pragma implementation "apt-pkg/rpmversion.h"
#endif 

#include <apt-pkg/rpmfactory.h>

#include <alloca.h>
#include <stdlib.h>

#include <rpm/rpmlib.h>
#include <rpm/misc.h>

// VersionCompare (op) - Greater than comparison for versions
// ---------------------------------------------------------------------
/* */
int RPMFactory::versionCompare(const char *A, const char *B)
{
   return versionCompare(A,A + strlen(A),B,B + strlen(B));
}

int RPMFactory::versionCompare(string A,string B)
{
   return versionCompare(A.begin(),A.end(),B.begin(),B.end());
}



/* Code ripped from rpmlib */
static void ParseVersion(const char *V, const char *VEnd,
			 char **Epoch, 
			 char **Version,
			 char **Release)
{
   string tmp = string(V, VEnd);
   const char *evr = tmp.c_str();
   const char *epoch, *epoend;
   const char *version, *verend;        /* assume only version is present */
   const char *release, *relend;
   char *s, *se;

   s = (char*)evr;
   while (*evr && isdigit(*s)) s++;   /* s points to epoch terminator */
   se = strrchr(s, '-');               /* se points to version terminator */

   if (*s == ':') {
      epoch = evr;
      *s++ = '\0';
      version = s;
      if (*epoch == '\0') epoch = "0";
   } else {
      epoch = NULL;   /* XXX disable epoch compare if missing */
      version = evr;
   }
   if (se) {
      *se++ = '\0';
      release = se;
   } else {
      release = NULL;
   }

#define Xstrdup(a) (a) ? strdup(a) : NULL
   *Epoch = Xstrdup(epoch);
   *Version = Xstrdup(version);
   *Release = Xstrdup(release);
#undef Xstrdup
}

/* This fragments the version into E:V-R triples and compares each 
   portion seperately. */
int RPMFactory::versionCompare(const char *A, const char *AEnd, 
				     const char *B, const char *BEnd)
{
#if 0
   char *AE, *AV, *AS;
   char *BE, *BV, *BS;
   int rc;
   
#define FREE(x) if (x) free(x)

   ParseVersion(A, AEnd, &AE, &AV, &AS);
   ParseVersion(B, BEnd, &BE, &BV, &BS);
   
   if (AE && !BE) {
       FREE(AE); FREE(AV); FREE(AS);
       FREE(BE); FREE(BV); FREE(BS);
       return 1;
   } else if (!AE && BE) {
       FREE(AE); FREE(AV); FREE(AS);
       FREE(BE); FREE(BV); FREE(BS);
       return -1;
   } else if (AE && BE) 
   {
      int Aep, Bep;
      
      Aep = atoi(AE);
      Bep = atoi(BE);
      
      FREE(AE); FREE(AV); FREE(AS);
      FREE(BE); FREE(BV); FREE(BS);

      if (Aep < Bep)
	  return -1;
      else
	  return 1;
   }
   
   rc = rpmvercmp(AV, BV);
    
   if (rc) {
       FREE(AE); FREE(AV); FREE(AS);
       FREE(BE); FREE(BV); FREE(BS);
       
       return rc;
   }

   rc = rpmvercmp(AS, BS);
   FREE(AE); FREE(AV); FREE(AS);
   FREE(BE); FREE(BV); FREE(BS);

   return rc; 
#else
    char *bufA;
    char *bufB;
    char *p;
    bool okA, okB;
    char *verA, *verB;
    char *relA, *relB;
    int res;
    
    bufA = (char*)alloca(AEnd-A+4);
    bufB = (char*)alloca(BEnd-B+4);
    
    bufA = strncpy(bufA, A, AEnd-A);
    bufA[AEnd-A] = 0;
    
    bufB = strncpy(bufB, B, BEnd-B);
    bufB[BEnd-B] = 0;
    
    // compare epoch
    p = strchr(bufA, ':');
    okA = (p != NULL);

    p = strchr(bufB, ':');
    okB = (p != NULL);

    if (okA && !okB)
	return 2;
    else if (!okA && okB)
	return -2;
    else if (okA && okB) {
	int epoA, epoB;
	
	p = strchr(bufA, ':');
	*p = 0;
	epoA = atoi(bufA);
	verA = p+1;
	
	p = strchr(bufB, ':');
	*p = 0;
	verB = p+1;
	
	epoB = atoi(bufB);
	if (epoA < epoB)
	    return -1;
	else if (epoA > epoB)
	    return 1;
    } else {
	verA = bufA;
	verB = bufB;
    }
    
    // compare version
    p = strchr(verA, '-');
    if (p) {
	*p = 0;
	relA = p+1;
    } else {
	relA = NULL;
    }
    
    p = strchr(verB, '-');
    if (p) {
	*p = 0;
	relB = p+1;
    } else {
	relB = NULL;
    }

    res = rpmvercmp(verA, verB);
    if (res)
	return res;
    
    // compare release
    if (!relA)
	return 0;

    if (!relB)
	return 0;
    
    res = rpmvercmp(relA, relB);
    
    return res;
#endif
}


// CheckDep - Check a single dependency					/*{{{*/
// ---------------------------------------------------------------------
/* This simply preforms the version comparison and switch based on 
   operator. */
bool RPMFactory::checkDep(const char *DepVer,const char *PkgVer,int Op)
{
   int rc;
   int PkgFlags;
   int DepFlags;
   bool invert = false;
   
   DepFlags = 0;
   PkgFlags = RPMSENSE_EQUAL;
   switch (Op & 0x0F)
   {
    case pkgCache::Dep::LessEq:
      DepFlags = RPMSENSE_LESS|RPMSENSE_EQUAL;
      break;

    case pkgCache::Dep::GreaterEq:
      DepFlags = RPMSENSE_GREATER|RPMSENSE_EQUAL;
      break;
      
    case pkgCache::Dep::Less:
      DepFlags = RPMSENSE_LESS;
      break;
      
    case pkgCache::Dep::Greater:
      DepFlags = RPMSENSE_GREATER;
      break;

    case pkgCache::Dep::Equals:
      DepFlags = RPMSENSE_EQUAL;
      break;
      
    case pkgCache::Dep::NotEquals:
      DepFlags = RPMSENSE_EQUAL;
      invert = true;
      break;
      
    default:
      DepFlags = RPMSENSE_ANY; // any version is ok
      break;
   }
   
   rc = rpmRangesOverlap("", PkgVer, PkgFlags, // provide
			 "", DepVer, DepFlags); // request
      
   if (invert)
       return !(rc == 1);
   else
       return (rc == 1);
}

									/*}}}*/
// BaseVersion - Return the upstream version string			/*{{{*/
// ---------------------------------------------------------------------
/* This strips all the debian specific information from the version number */
string RPMFactory::baseVersion(const char *Ver)
{
   // Strip off the bit before the first colon
   const char *I = Ver;
   for (; *I != 0 && *I != ':'; I++);
   if (*I == ':')
      Ver = I + 1;
   
   // Chop off the trailing -
   I = Ver;
   unsigned Last = strlen(Ver);
   for (; *I != 0; I++)
      if (*I == '-')
	 Last = I - Ver;
      
   return string(Ver,Last);
}
									/*}}}*/
