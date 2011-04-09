dnl -*-m4-*-
dnl    GNUbik -- A 3 dimensional magic cube game.
dnl    Copyright (C) 1998, 2003, 2007, 2010  John Darrington
dnl    
dnl    This program is free software; you can redistribute it and/or modify
dnl    it under the terms of the GNU General Public License as published by
dnl    the Free Software Foundation; either version 3 of the License, or
dnl    (at your option) any later version.
dnl    
dnl    This program is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl    GNU General Public License for more details.
dnl  
dnl    You should have received a copy of the GNU General Public License
dnl    along with this program.  If not, see <http://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.


AX_PREREQ(2.60)
AC_INIT([GNUbik], [2.4], [bug-gnubik@gnu.org], [gnubik], [http://www.gnu.org/software/gnubik])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_GNU_SOURCE
AC_PROG_CC
AM_PROG_CC_C_O

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

PKG_PROG_PKG_CONFIG
m4_pattern_forbid([PKG_CHECK_MODULES])

AM_CONDITIONAL(cc_is_gcc, test x"$GCC" = x"yes" )

dnl Internationalization macros.
AM_GNU_GETTEXT([external], [need-ngettext])
AM_GNU_GETTEXT_VERSION([0.18])

AC_ARG_ENABLE(debug,
     [  --enable-debug    Turn on debugging],
     [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
     esac],[debug=false])

if test x"$debug" = x"true" ; then 
	AC_DEFINE(DEBUG,1,[Set to 1 if debugging features should be compiled in])
fi


dnl Find the X11 include and library directories
AC_PATH_X
AC_PATH_XTRA

AX_CHECK_GL
AX_CHECK_GLU

if test "x$no_gl" = x"yes"; then
   AC_MSG_ERROR("No GL library seems to be installed")
fi

if test "x$no_glu" = x"yes"; then
   AC_MSG_ERROR("No GLU library seems to be installed")
fi


dnl Checks for libraries.
PKG_CHECK_MODULES(GUILE, guile-2.0,,
  AC_SEARCH_LIBS([scm_c_string_length], [guile],,
	AC_MSG_ERROR("Guile 1.8.0 or later is required"))
)

AC_SEARCH_LIBS([cos],[m],,
	AC_MSG_ERROR("No maths library present" ))



PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.20.0,,)
PKG_CHECK_MODULES(GDK_PIXBUF, gdk-pixbuf-2.0,,)
PKG_CHECK_MODULES(GDK_GL_EXT, gdkglext-1.0,,)
PKG_CHECK_MODULES(GTK_GL_EXT, gtkglext-1.0,,)


dnl Glut is used only if building with debug info
dnl If it's there, then include it. Otherwise don't bother
if test x"$debug" = x"true" ; then 
  AX_CHECK_GLUT
  if test "x$no_glut" = x"yes"; then
     AC_MSG_ERROR("Debug features requested but no GLUT library seems to be installed")
  fi
fi


AC_C_CONST
AC_C_INLINE

AC_CHECK_FUNCS(getopt_long)

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)


AC_OUTPUT(Makefile po/Makefile)

