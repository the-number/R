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


ACLOCAL_AMFLAGS = -I m4
