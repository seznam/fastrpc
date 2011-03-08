#
# FILE              $Id: Makefile,v 1.6 2011-03-08 08:23:37 volca Exp $
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

PYTHON_BINARIES ?= /usr/bin/python



# all versions depend upon this file (dirty hack)
all: lib/*/fastrpcmodule.so

# create all modules
# for all found python binaries create
#
lib/*/fastrpcmodule.so: pyobjectwrapper.h fastrpcmodule.cc fastrpcmodule.h \
						pythonserver.cc pyerrors.cc pythonbuilder.h \
						pythonbuilder.cc pythonfeeder.h pythonfeeder.cc
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

dist:
	rm -Rf python-fastrpc
	mkdir python-fastrpc

	cp -va --parents api_doc.doxygen Doxyfile \
		Makefile setup.py debian/fastrpc.control \
		debian/make.sh debian/changelog debian/compat debian/control \
		debian/rules example/client_example.py \
		fastrpcmodule.cc pyerrors.cc pythonbuilder.cc \
		pythonfeeder.cc pythonserver.cc fastrpcmodule.h \
		pyobjectwrapper.h pythonbuilder.h pythonfeeder.h \
	        python-fastrpc

	tar cvzf python-fastrpc.tar.gz python-fastrpc

	rm -Rf python-fastrpc
