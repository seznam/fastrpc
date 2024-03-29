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

from distutils import sysconfig
from os import uname
from os.path import dirname, join
from setuptools import find_packages, setup, Extension
import sys


if hasattr(sysconfig, "_init_posix"):
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
readme = join(here, 'README.md')

try:
    ModuleNotFoundError
except NameError:
    ModuleNotFoundError = ImportError

try:
    from setuptools import dist
    dist.Distribution(dict(setup_requires="pkginfo"))
    from pkginfo import UnpackedSDist
    d = UnpackedSDist(__file__)
    version = d.version
except (ModuleNotFoundError, ValueError):
    try:
        from debian.changelog import Changelog
        changelog = join(here, "debian/changelog")
        version = str(Changelog(open(changelog, 'rt')).get_version())
    except ModuleNotFoundError:
        print("don't know how to determine module version")
        sys.exit(1)

author = u"Miroslav Talášek"
author_email = "miroslav.talasek@firma.seznam.cz"
exclude = []

if sys.version_info.major < 3:
    exclude.append('fastrpc.handler*')

setup(
    name="fastrpc",
    version=version,
    author=author,
    author_email=author_email,
    description=__doc__.strip().split("\n")[0],
    long_description=open(readme, 'rt').read().strip(),
    url="http://github.com/seznam/fastrpc/python",
    packages=find_packages(include=['fastrpc*'], exclude=exclude),
    ext_modules=[
        Extension("_fastrpc", [
            "fastrpcmodule.cc",
            "pythonserver.cc",
            "pyerrors.cc",
            "pythonbuilder.cc",
            "pythonfeeder.cc",
        ], libraries=["fastrpc"]),
    ], headers=[
        "frpcpythonhelper.h",
        "pyobjectwrapper.h",
        "pythonbuilder.h",
        "pythoncompat.h",
        "pythonfeeder.h",
        "fastrpcmodule.h",
    ], install_requires=[
        "pkginfo",
    ], test_suite='test'
)

