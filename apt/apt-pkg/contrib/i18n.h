/* i18n.h -- Internationalization stuff (ripped from bash)

   Copyright (C) 1996 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.
  
   Modified 9/2000 by Alfredo K. Kojima for apt

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#if !defined (_I18N_H_)
#define _I18N_H_

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined (HAVE_LIBINTL_H)
#  include <libintl.h>
#endif

#if defined (HAVE_LOCALE_H)
#  include <locale.h>
#endif

#if defined (HAVE_SETLOCALE) && !defined (LC_ALL)
#  undef HAVE_SETLOCALE
#endif

#if ENABLE_NLS
#  define _(a) (const char*)gettext(a)
#else
#  undef bindtextdomain
#  define bindtextdomain(Domain, Directory) /* empty */
#  undef textdomain
#  define textdomain(Domain) /* empty */
#  undef setlocale
#  define setlocale(cat, log)
#  define _(a) a
#endif

#endif /* !_I18N_H_ */
