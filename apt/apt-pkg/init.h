// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: init.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Init - Initialize the package library

   This function must be called to configure the config class before
   calling many APT library functions.
   
   ##################################################################### */
									/*}}}*/
// Header section: pkglib
#ifndef PKGLIB_INIT_H
#define PKGLIB_INIT_H

#include <apt-pkg/configuration.h>

bool pkgInitialize(Configuration &Cnf);

#endif
