#!/usr/bin/env python

# This version is used when packaging.
VERSION          = "1.0.0"

# Maintainer of this module.
MAINTAINER       = "Miroslav Talasek"
MAINTAINER_EMAIL = "miroslav.talasek@firma.seznam.cz"

# Descriptions
DESCRIPTION      = "FastRPC - RPC protocol suport Binary and XML format."
LONG_DESCRIPTION = "FastRPC - RPC protocol suport Binary and XML format.\n"

# You probably don't need to edit anything below this line

from distutils.core import setup, Extension

########################################################################
# Forces g++ instead of gcc on most systems
# credits to eric jones (eric@enthought.com) (found at Google Groups)
import distutils.sysconfig

old_init_posix = distutils.sysconfig._init_posix

def _init_posix():
    old_init_posix()
    distutils.sysconfig._config_vars['CFLAGS'] = '-fexceptions -g'
    distutils.sysconfig._config_vars['OPT'] = '-fexceptions -g'
    distutils.sysconfig._config_vars['LDSHARED'] = 'g++ -shared -fexceptions -g'

distutils.sysconfig._init_posix = _init_posix
########################################################################

from os import environ
import string

# Main core
setup (
    name             = "fastrpc",
    version          = VERSION,
    author           = "Miroslav Talasek",
    author_email     = "miroslav.talasek@firma.seznam.cz",
    maintainer       = MAINTAINER,
    maintainer_email = MAINTAINER_EMAIL,
    description      = DESCRIPTION,
    long_description = LONG_DESCRIPTION,
    ext_modules = [
        Extension ("fastrpcmodule", ["fastrpcmodule.cc", "pythonserver.cc",
                                     "pyerrors.cc", "pythonbuilder.cc",
                                     "pythonfeeder.cc"],
                   libraries=["fastrpc"]),
        ]
)
