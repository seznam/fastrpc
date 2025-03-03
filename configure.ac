#
# Fast RPC RPC library suports XML and binary format.
# Copyright (C) 2005-7  Seznam.cz, a.s.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Seznam.cz, a.s.
# Radlicka 2, Praha 5, 15000, Czech Republic
# http://www.seznam.cz, mailto:fastrpc@firma.seznam.cz
#
#
# $Id: configure.in,v 1.52 2011-02-25 09:31:03 volca Exp $
#
# DESCRIPTION
# configure template for fastrpc.
#
# AUTHORS
# Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
#
# HISTORY
# 2004-02-14  (miro)
#             Created.
#

# initialize autoconf
AC_INIT([fastrpc], [8.1.14~lubosperpartes-2], fastrpc@firma.seznam.cz)

# initialize automake(use AC_INIT's arguments)
AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_MACRO_DIR([m4])

# tell autoconf where to find sources
AC_CONFIG_SRCDIR([src/frpcvalue.cc])

AC_PREREQ(2.50)

AC_CANONICAL_HOST

case $host in
*solaris*)
    AC_ARG_ENABLE(m64, AS_HELP_STRING([--enable-m64],[enable 64bit mode]), [
            AC_MSG_CHECKING(for -m64 linker options)
            CPPFLAGS="-m64 -DSOLARIS"
            CXXFLAGS="-m64 -DSOLARIS"
            CFLAGS="-m64 -DSOLARIS"
            LDFLAGS="-m64 -L/opt/csw/lib/amd64 -L/usr/local/lib/amd64"
            sbindir='${exec_prefix}/sbin/amd64'
            bindir='${exec_prefix}/bin/amd64'
            libdir='${exec_prefix}/lib/amd64'
            AC_MSG_RESULT([-m64 flag set])],
            [LDFLAGS="-L/opt/csw/lib -L/usr/local/lib"
            CPPFLAGS="-DSOLARIS"
            CXXFLAGS="-DSOLARIS"
            CFLAGS="-DSOLARIS"])
    ;;
esac


# check for C compiled
AC_PROG_CC

# check for C++ compiler
AC_PROG_CXX

# check for std C headers
AC_HEADER_STDC

# check for libtool
AC_PROG_LIBTOOL

AC_LANG_CPLUSPLUS

case $host in
*solaris*)
    AC_ARG_ENABLE(m64, [enable 64bit mode], [], [
        AC_CHECK_LIB(z,gzopen64, , AC_MSG_ERROR(Cannot find libz library!))])
    ;;
esac

m4_ifndef([PKG_PROG_PKG_CONFIG], [m4_fatal([Please install pkg-config.])])
PKG_PROG_PKG_CONFIG

# pkg-config resolvable dependencies for consistent .pc file wit configure
# script;
# add new pkg-config dependencies here
PC_DEPS="libxml-2.0"
AC_SUBST([PC_DEPS])

PKG_CHECK_MODULES([DEPS], [$PC_DEPS])

dnl ------------------------------------------------------------------------
# system dependant part

# search for socket related libraries
AC_SEARCH_LIBS(getnetbyname, socket, , )
AC_SEARCH_LIBS(hstrerror, resolv, , )
AC_SEARCH_LIBS(inet_ntoa, nsl, , )

dnl ------------------------------------------------------------------------

# This version number needs to be changed in several different ways for each
# release. Please read the libtool documentation (info libtool 'Updating
# version info') before touching this.
FASTRPC_MAJOR=8
FASTRPC_MINOR=0
VERSION_INFO="-version-info 14:0:1"

AC_ARG_ENABLE(optimization,[  --enable-optimization compile optimized without debug logging],[
    case "${enableval}" in
        no)
            AC_MSG_NOTICE([disabling optimization.])
            CPPFLAGS="${CPPFLAGS} -DDEBUG=1"
        ;;
        yes)
            AC_MSG_NOTICE([enabling optimization.])
            CXXEXTRAFLAGS="${CXXEXTRAFLAGS} -O2"
            CPPFLAGS="${CPPFLAGS} -UDEBUG"
        ;;
        *)
            AC_MSG_ERROR([Say yes or no to --enable-optimization.])
        ;;
    esac
], [
   CPPFLAGS="${CPPFLAGS} -DDEBUG=1"
])

AC_SUBST(FASTRPC_MAJOR)
AC_SUBST(FASTRPC_MINOR)
AC_SUBST(VERSION_INFO)
AC_SUBST(CXXEXTRAFLAGS)

# generate Makefile
AC_CONFIG_FILES([src/frpcversion.h
    Makefile
    src/Makefile
    version
    libfastrpc.pc])

AC_OUTPUT()
