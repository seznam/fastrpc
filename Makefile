#
# FILE              $Id: Makefile,v 1.4 2007-05-25 15:31:22 vasek Exp $
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

PYTHON_BINARIES = /usr/bin/python



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

	cp -va --parents python-fastrpc.tar.gz api_doc.doxygen Doxyfile \
	        fastrpcmodule.cc Makefile setup.py debian/fastrpc.control \
		debian/fastrpc.version debian/make.sh example/client_example.py \
	        python-fastrpc

	tar cvzf python-fastrpc.tar.gz python-fastrpc

	rm -Rf python-fastrpc
