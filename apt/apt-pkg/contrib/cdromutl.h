// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: cdromutl.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   CDROM Utilities - Some functions to manipulate CDROM mounts.
   
   ##################################################################### */
									/*}}}*/
#ifndef PKGLIB_CDROMUTL_H
#define PKGLIB_ACQUIRE_METHOD_H

#include <string>

#ifdef __GNUG__
#pragma interface "apt-pkg/cdromutl.h"
#endif 

bool MountCdrom(string Path);
bool UnmountCdrom(string Path);
bool IdentCdrom(string CD,string &Res,unsigned int Version = 2);

#endif
