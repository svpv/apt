
bin_PROGRAMS = genpkglist gensrclist countpkglist
bin_SCRIPTS = genbasedir

EXTRA_DIST = genbasedir

AM_CPPFLAGS = -D_RPM_4_4_COMPAT
LDADD = ../apt-pkg/libapt-pkg.la $(RPMLIBS)

genpkglist_SOURCES = genpkglist.cc cached_md5.cc cached_md5.h
gensrclist_SOURCES = gensrclist.cc cached_md5.cc cached_md5.h
countpkglist_SOURCES = countpkglist.cc
