// -*- mode: cpp; mode: fold -*-
// Description								/*{{{*/
// $Id: errorshow.h,v 1.1.1.1 2000/08/10 12:42:39 kojima Exp $
/* ######################################################################

   Error Show - Show errors from the apt-pkg error class.

   This should be called when an apt-pkg function fails. It will display
   a dialog box with an errors/warnings that occured or do nothing.
   
   ##################################################################### */
									/*}}}*/
#ifndef ERRORSHOW_H
#define ERRORSHOW_H

#include <string>

bool ShowErrors(string Reason = string(),bool Fatal = false);

#endif
