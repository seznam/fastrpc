#
# Fast RPC
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
# $Id: Makefile.am,v 1.13 2011-02-25 09:21:07 volca Exp $
#
# DESCRIPTION
# A Fast RPC library.
#
# AUTHORS
# Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
#
# HISTORY
# 2005-02-14  (miro)
#             Created.
#


# warn on all
AM_CXXFLAGS = -Wall -fPIC $(DEPS_CFLAGS) ${CXXEXTRAFLAGS}

# extra compiler options
AM_CPPFLAGS = -D_ISOC99_SOURCE -D__ENABLE_WSTRING=1 -std=c++11

# install these headers
include_HEADERS = frpc.h frpcvalue.h frpcarray.h frpcstruct.h frpcbinary.h frpcdatetime.h \
                  frpcstring.h frpcbool.h frpcint.h frpcdouble.h frpcpool.h \
                  frpcerror.h frpckeyerror.h frpctypeerror.h frpcindexerror.h \
                  frpcstreamerror.h frpclenerror.h frpcserverproxy.h frpcdatabuilder.h \
                  frpcmarshaller.h frpcunmarshaller.h frpcwriter.h frpcprotocolerror.h \
                  frpchttperror.h frpcencodingerror.h frpchttpio.h frpchttp.h frpchttpclient.h \
                  frpctreebuilder.h frpctreefeeder.h frpcfault.h frpcmethod.h frpcdefaultmethod.h \
                  frpcmethodregistry.h frpcserver.h frpcresponseerror.h frpcplatform.h frpcheadmethod.h \
                  frpcsocket.h frpcsocketunix.h frpcsocketwin.h frpcplatform.h \
                  frpcversion.h frpcconnector.h frpcconverters.h frpcnull.h \
                  frpcbinmarshaller.h frpcxmlmarshaller.h frpcinternals.h frpccompare.h frpcb64marshaller.h \
                  frpcjsonmarshaller.h frpcb64writer.h frpcconfig.h frpcbinunmarshaller-extras.h            \
				  frpcbinaryref.h frpclogging.h


noinst_HEADERS = frpcbinunmarshaller.h frpcxmlunmarshaller.h frpcurlunmarshaller.h frpcb64unmarshaller.h frpcbase64.h \
                 nonglibc.h

# compile this library
lib_LTLIBRARIES = libfastrpc.la

# from the these sources
libfastrpc_la_SOURCES = frpcvalue.cc frpcarray.cc frpcstruct.cc frpcbinary.cc frpcdatetime.cc \
                        frpcstring.cc frpcbool.cc frpcint.cc frpcdouble.cc frpcpool.cc \
                        frpcerror.cc frpckeyerror.cc frpctypeerror.cc frpcindexerror.cc \
                        frpcstreamerror.cc frpclenerror.cc frpcserverproxy.cc frpcdatabuilder.cc \
                        frpcmarshaller.cc frpcunmarshaller.cc frpcwriter.cc \
                        frpcbinmarshaller.cc  frpcxmlmarshaller.cc \
                        frpcbinunmarshaller.cc frpcxmlunmarshaller.cc frpcprotocolerror.cc \
                        frpchttperror.cc frpcencodingerror.cc frpchttpio.cc frpchttp.cc frpchttpclient.cc \
                        frpctreebuilder.cc frpctreefeeder.cc frpcfault.cc frpc.cc frpcmethodregistry.cc \
                        frpcserver.cc frpcresponseerror.cc frpcconnector.cc frpcnull.cc \
                        frpcurlunmarshaller.cc frpcjsonmarshaller.cc frpcb64unmarshaller.cc frpcbase64.cc \
                        frpcb64writer.cc frpcconfig.cc frpccompare.cc

# with these flags (version info etc.)
libfastrpc_la_LDFLAGS = @VERSION_INFO@ $(DEPS_LIBS)

check_PROGRAMS=test-base64 test-protocol test-marshallers
test_base64_SOURCES=../test/base64.cc
test_protocol_SOURCES=../test/protocol.cc
test_marshallers_SOURCES=../test/marshallers.cc
test_base64_LDADD=libfastrpc.la
test_protocol_LDADD=libfastrpc.la
test_marshallers_LDADD=libfastrpc.la

TESTS=test-base64 test-protocol @top_srcdir@/test/marshallers.test

#doc:
    #doxygen
