#!/bin/bash
hash debuild >/dev/null 2>&1 || {
    hash dpkg-buildpackage >/dev/null 2>&1 || {
        echo >&2 "Please, install devscripts or dpkg-dev to build python-fastrpc debian package."
        exit 1
    }
    dpkg-buildpackage -uc -us
    exit 0
}

debuild -uc -us
