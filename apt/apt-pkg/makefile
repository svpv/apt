# -*- make -*-
BASE=..
SUBDIR=apt-pkg

# Header location
SUBDIRS = rpm deb contrib
HEADER_TARGETDIRS = apt-pkg

# Bring in the default rules
include ../buildlib/defaults.mak

# The library name
LIBRARY=apt-pkg
MAJOR=3.1
MINOR=0
SLIBS=$(PTHREADLIB)

# Source code for the contributed non-core things
SOURCE = contrib/mmap.cc contrib/error.cc contrib/strutl.cc \
         contrib/configuration.cc contrib/progress.cc contrib/cmndline.cc \
	 contrib/md5.cc contrib/cdromutl.cc contrib/crc-16.cc

# Source code for the main library
SOURCE+= pkgcache.cc version.cc fileutl.cc pkgcachegen.cc depcache.cc \
         orderlist.cc tagfile.cc sourcelist.cc packagemanager.cc \
	 pkgrecords.cc algorithms.cc acquire.cc acquire-item.cc \
	 acquire-worker.cc acquire-method.cc init.cc clean.cc \
	 srcrecords.cc cachefile.cc systemfactory.cc

# Source code for the debian specific components
SOURCE+= deb/deblistparser.cc deb/debrecords.cc deb/dpkgpm.cc deb/dpkginit.cc \
         deb/debsrcrecords.cc \
	 deb/debfactory.cc

# Source code for the rpm specific components
SOURCE+= rpm/rpminit.cc rpm/rpmlistparser.cc rpm/rpmpm.cc\
	rpm/rpmrecords.cc rpm/rpmsrcrecords.cc

SOURCE+= rpm/rpmfactory.cc rpm/rpmversion.cc rpm/rpmpackagedata.cc

# Public apt-pkg header files
HEADERS = algorithms.h depcache.h mmap.h pkgcachegen.h cacheiterators.h \
          error.h orderlist.h sourcelist.h configuration.h fileutl.h \
          packagemanager.h tagfile.h deblistparser.h init.h pkgcache.h \
          progress.h pkgrecords.h debrecords.h cmndline.h \
	  acquire.h acquire-worker.h acquire-item.h acquire-method.h md5.h \
	  dpkgpm.h dpkginit.h cdromutl.h strutl.h clean.h srcrecords.h \
	  debsrcrecords.h cachefile.h crc-16.h systemfactory.h sptr.h

HEADERS+= rpmfactory.h debfactory.h

HEADERS+= rpminit.h rpmlistparser.h rpmpm.h rpmrecords.h rpmpackagedata.h\
	rpmsrcrecords.h
	

HEADERS := $(addprefix apt-pkg/,$(HEADERS))

# Private header files
HEADERS+= system.h i18n.h

include $(LIBRARY_H)
