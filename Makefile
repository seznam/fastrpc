#
# FILE              $Id: Makefile,v 1.1 2005-07-19 13:02:55 vasek Exp $
#
# DESCRIPTION       A makefile for fastrpc python module.
#
# AUTHOR            Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
#
# Copyright (c) 2005 Seznam.cz, a.s.
# All Rights Reserved.
#
# HISTORY
#       2005-02-09 (miro)
#                   Created.
#

PYTHON_BINARIES = /usr/bin/python2.2



# all versions depend upon this file (dirty hack)
all: lib/*/fastrpcmodule.so

# create all modules
# for all found python binaries create 
#
lib/*/fastrpcmodule.so:
	(for python in $(PYTHON_BINARIES); do \
                echo "building python module for `basename $${python}`" || exit $?; \
                libddir=`pwd`"/lib/"`basename $${python}` || exit $?; \
                 $${python} setup.py build_ext || exit $?; \
                 $${python} setup.py install_lib --skip-build \
                                                --install-dir $${libddir} || exit $?; \
         done)

# clean all
clean:
	(for python in $(PYTHON_BINARIES); do \
                libdir=`pwd`"/lib/"`basename $${python}`; \
                $${python} setup.py clean --quiet --all; \
                rm -rf $${libdir}; \
         done)
