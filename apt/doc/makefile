# -*- make -*-
BASE=..
SUBDIR=doc doc/pt_BR

# Bring in the default rules
include ../buildlib/defaults.mak

# SGML Documents
SOURCE = dpkg-tech.sgml design.sgml files.sgml guide.sgml cache.sgml \
         method.sgml offline.sgml
include $(DEBIANDOC_H)

# Man pages
SOURCE = apt-cache.8 apt-get.8 apt-cdrom.8 apt.conf.5 \
        sources.list.5 vendors.list.5 apt-config.8 \
        pt_BR/apt-cache.8 pt_BR/apt-get.8 pt_BR/apt-cdrom.8 \
        pt_BR/apt.conf.5 pt_BR/sources.list.5 pt_BR/vendors.list.5 \
        pt_BR/apt-config.8
include $(YODL_MANPAGE_H)

# Examples
SOURCE = examples/apt.conf examples/sources.list examples/vendors.list examples/configure-index
TO = $(DOC)
include $(COPY_H)
