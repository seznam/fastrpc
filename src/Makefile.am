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
AM_CXXFLAGS = -Wall -g -fPIC $(DEPS_CFLAGS) ${CXXEXTRAFLAGS}

# extra compiler options
AM_CPPFLAGS = -D_ISOC99_SOURCE -D__ENABLE_WSTRING=1

# install these headers
pkginclude_HEADERS = frpc.h \
					 value/frpcvalue.h value/frpcarray.h value/frpcstruct.h value/frpcbinary.h value/frpcdatetime.h \
                  value/frpcstring.h value/frpcbool.h value/frpcint.h value/frpcdouble.h value/frpcpool.h \
                  error/frpcerror.h error/frpckeyerror.h error/frpctypeerror.h error/frpcindexerror.h \
				  error/frpcstreamerror.h error/frpclenerror.h \
				  frpcserverproxy.h frpcdatabuilder.h \
                  marshaller/frpcmarshaller.h marshaller/frpcunmarshaller.h writer/frpcwriter.h error/frpcprotocolerror.h \
                  error/frpchttperror.h error/frpcencodingerror.h http/frpchttpio.h http/frpchttp.h http/frpchttpclient.h \
                  frpctreebuilder.h frpctreefeeder.h error/frpcfault.h frpcmethod.h frpcdefaultmethod.h \
                  frpcmethodregistry.h writer/frpcserver.h error/frpcresponseerror.h frpcplatform.h frpcheadmethod.h \
                  frpcsocket.h frpcsocketunix.h frpcsocketwin.h frpcplatform.h \
                  frpcversion.h frpcconnector.h frpcconverters.h frpcnull.h \
                  marshaller/frpcbinmarshaller.h marshaller/frpcxmlmarshaller.h frpcinternals.h frpccompare.h marshaller/frpcb64marshaller.h \
                  marshaller/frpcjsonmarshaller.h writer/frpcb64writer.h frpcconfig.h


noinst_HEADERS = marshaller/frpcbinunmarshaller.h marshaller/frpcxmlunmarshaller.h marshaller/frpcurlunmarshaller.h marshaller/frpcb64unmarshaller.h marshaller/frpcbase64.h \
                 nonglibc.h

# compile this library
lib_LTLIBRARIES = libfastrpc.la

# from the these sources
libfastrpc_la_SOURCES = value/frpcvalue.cc value/frpcarray.cc value/frpcstruct.cc value/frpcbinary.cc value/frpcdatetime.cc \
                        value/frpcstring.cc value/frpcbool.cc value/frpcint.cc value/frpcdouble.cc value/frpcpool.cc \
                        error/frpcerror.cc error/frpckeyerror.cc error/frpctypeerror.cc error/frpcindexerror.cc \
                        error/frpcstreamerror.cc error/frpclenerror.cc \
						frpcserverproxy.cc frpcdatabuilder.cc \
                        marshaller/frpcmarshaller.cc marshaller/frpcunmarshaller.cc writer/frpcwriter.cc \
                        marshaller/frpcbinmarshaller.cc  marshaller/frpcxmlmarshaller.cc \
                        marshaller/frpcbinunmarshaller.cc marshaller/frpcxmlunmarshaller.cc error/frpcprotocolerror.cc \
                        error/frpchttperror.cc frpcencodingerror.cc http/frpchttpio.cc http/frpchttp.cc http/frpchttpclient.cc \
                        frpctreebuilder.cc frpctreefeeder.cc error/frpcfault.cc frpc.cc frpcmethodregistry.cc \
                        writer/frpcserver.cc error/frpcresponseerror.cc frpcconnector.cc value/frpcnull.cc \
                        marshaller/frpcurlunmarshaller.cc marshaller/frpcjsonmarshaller.cc marshaller/frpcb64unmarshaller.cc marshaller/frpcbase64.cc \
                        writer/frpcb64writer.cc frpcconfig.cc

# with these flags (version info etc.)
libfastrpc_la_LDFLAGS = @VERSION_INFO@ $(DEPS_LIBS)


# test program
#noinst_PROGRAMS =

#doc:
    #doxygen
