## Process this file with automake to produce Makefile.in  -*- makefile -*-

SUBDIRS= po

AUTOMAKE_OPTIONS = gnits 1.10 subdir-objects

SUFFIXES=
ALL_LOCAL=
CLEAN_LOCAL=


icondir=$(pkgdatadir)/icons
AM_CPPFLAGS=-DLOCALEDIR=\"$(localedir)\" -DPKGICONDIR=\"$(icondir)\"

AM_CFLAGS=

if cc_is_gcc
AM_CFLAGS+=-Wall -Wwrite-strings \
-Wpointer-arith -Wno-sign-compare -Wmissing-prototypes
endif


include $(top_srcdir)/src/automake.mk
include $(top_srcdir)/scripts/automake.mk
include $(top_srcdir)/doc/automake.mk
include $(top_srcdir)/po/automake.mk
include $(top_srcdir)/icons/automake.mk


ACLOCAL_AMFLAGS = -I m4

all-local: $(ALL_LOCAL)

$(PACKAGE)-$(VERSION).tar.gz.directive: $(PACKAGE)-$(VERSION).tar.gz
	echo version: 1.1 > $@
	echo directory: gnubik >> $@
	echo filename: $< >> $@

$(PACKAGE)-$(VERSION).tar.gz.directive.asc: $(PACKAGE)-$(VERSION).tar.gz.directive
	gpg --clearsign $<

$(PACKAGE)-$(VERSION).tar.gz.sig: $(PACKAGE)-$(VERSION).tar.gz
	gpg -b $<

.PHONY: release

release: distcheck $(PACKAGE)-$(VERSION).tar.gz.directive.asc $(PACKAGE)-$(VERSION).tar.gz.sig

