# -*- make -*-
BASE=..
SUBDIR=gui

# Bring in the default rules
include ../buildlib/defaults.mak

# The program name
PROGRAM=apt
SLIBS = -lapt-pkg -ldeity -lpthread
LIB_MAKES = apt-pkg/makefile deity/makefile

# Source files
SOURCE = apt.cc aptwidgets.cc extracache.cc pkgtree.cc pkgtreeitem.cc \
         policy.cc statuswidgets.cc progressmeter.cc errorshow.cc

include $(PROGRAM_H)
