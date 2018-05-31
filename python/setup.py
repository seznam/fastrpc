#!/usr/bin/env python
# coding=utf-8
"""
FastRPC -- Fast RPC library compatible with XML-RPC
Copyright (C) 2005-2014  Seznam.cz, a.s.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Seznam.cz, a.s.
Radlická 3294/10, Praha 5, 15000, Czech Republic
http://www.seznam.cz, mailto:fastrpc@firma.seznam.cz
"""

from debian.changelog import Changelog
from distutils import sysconfig
from os import uname
from os.path import dirname, join
from setuptools import setup, Extension


def _init_posix(init):
    """
    Forces g++ instead of gcc on most systems
    credits to eric jones (eric@enthought.com) (found at Google Groups)
    """
    def wrapper():
        init()

        config_vars = sysconfig.get_config_vars()  # by reference
        if config_vars["MACHDEP"].startswith("sun"):
            # Sun needs forced gcc/g++ compilation
            config_vars['CC'] = 'gcc'
            config_vars['CXX'] = 'g++'

        # FIXME raises hardening-no-fortify-functions lintian warning.
        else:
            # Non-Sun needs linkage with g++
            config_vars['LDSHARED'] = 'g++ -shared -g -W -Wall -Wno-deprecated'

        if uname()[0] == 'SunOS':
            config_vars['CFLAGS'] = '-g -W -Wall -Wno-deprecated -I/opt/szn/include -m64'
            config_vars['LDFLAGS'] = '-L/opt/szn/lib/amd64'
        else:
            config_vars['CFLAGS'] = '-g -W -Wall -Wno-deprecated'

        config_vars['OPT'] = '-g -W -Wall -Wno-deprecated'

    return wrapper
sysconfig._init_posix = _init_posix(sysconfig._init_posix)


here = dirname(__file__)
readme = join(here, 'README')
changelog = join(here, "debian/changelog")

version = str(Changelog(open(changelog, 'rt')).get_version())

author = u"Miroslav Talášek"
author_email = "miroslav.talasek@firma.seznam.cz"

setup(
    name="szn-fastrpc",
    version=version,
    author=author,
    author_email=author_email,
    description=__doc__.strip().split("\n")[0],
    long_description=open(readme, 'rt').read().strip(),
    url="http://github.com/seznam/fastrpc/python",
    py_modules=['fastrpc'],
    ext_modules=[
        Extension("_fastrpc", [
            "fastrpcmodule.cc", "pythonserver.cc", "pyerrors.cc",
            "pythonbuilder.cc", "pythonfeeder.cc",
            "../src/frpcvalue.cc", "../src/frpcarray.cc",
            "../src/frpcstruct.cc", "../src/frpcbinary.cc",
            "../src/frpcdatetime.cc", "../src/frpcstring.cc",
            "../src/frpcbool.cc", "../src/frpcint.cc",
            "../src/frpcdouble.cc", "../src/frpcpool.cc",
            "../src/frpcerror.cc", "../src/frpckeyerror.cc",
            "../src/frpctypeerror.cc", "../src/frpcindexerror.cc",
            "../src/frpcstreamerror.cc", "../src/frpclenerror.cc",
            "../src/frpcserverproxy.cc", "../src/frpcdatabuilder.cc",
            "../src/frpcmarshaller.cc", "../src/frpcunmarshaller.cc",
            "../src/frpcwriter.cc", "../src/frpcbinmarshaller.cc",
            "../src/frpcxmlmarshaller.cc", "../src/frpcbinunmarshaller.cc",
            "../src/frpcxmlunmarshaller.cc", "../src/frpcprotocolerror.cc",
            "../src/frpchttperror.cc", "../src/frpcencodingerror.cc",
            "../src/frpchttpio.cc", "../src/frpchttp.cc",
            "../src/frpchttpclient.cc", "../src/frpctreebuilder.cc",
            "../src/frpctreefeeder.cc", "../src/frpcfault.cc",
            "../src/frpc.cc", "../src/frpcmethodregistry.cc",
            "../src/frpcserver.cc", "../src/frpcresponseerror.cc",
            "../src/frpcconnector.cc", "../src/frpcnull.cc",
            "../src/frpcurlunmarshaller.cc", "../src/frpcjsonmarshaller.cc",
            "../src/frpcb64unmarshaller.cc", "../src/frpcbase64.cc",
            "../src/frpcb64writer.cc", "../src/frpcconfig.cc",
            "../src/frpccompare.cc"
        ], libraries=["xml2"],
           include_dirs=["/usr/include/libxml2", "../src/"]),
    ],
    test_suite='test'
)
