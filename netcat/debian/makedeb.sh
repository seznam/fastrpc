#!/bin/bash

DEB_PACKAGE="fastrpc-netcat"

# common part, don't touch!

if [ "$MAINTAINER" == "" ]; then
    echo "You don't have MAINTAINER environment variable set"
    echo "Put something like:"
    echo
    echo "    export MAINTAINER=\"Name Surname <email@addr.ess>\""
    echo
    echo "to your .bashrc file and relogin"
    exit 1
fi

WORKDIR=".tmp"

if [ -d $WORKDIR ]; then
    rm -rf $WORKDIR
fi

mkdir -p $WORKDIR

# custom part, this is what you probably want to edit:

mkdir -m755 -p $WORKDIR/usr/bin
mkdir -m755 -p $WORKDIR/usr/share/man/man1
mkdir -m755 -p $WORKDIR/usr/share/fastrpc-netcat

cp fastrpc-netcat $WORKDIR/usr/bin
cp fastrpc-netcat3 $WORKDIR/usr/bin
cat fastrpc-netcat.1 | gzip > $WORKDIR/usr/share/man/man1/fastrpc-netcat.1.gz
cp COPYING $WORKDIR/usr/share/fastrpc-netcat

chmod 755 $WORKDIR/usr/bin/fastrpc-netcat
chmod 755 $WORKDIR/usr/bin/fastrpc-netcat3
chmod 644 $WORKDIR/usr/share/man/man1/fastrpc-netcat.1.gz
chmod 644 $WORKDIR/usr/share/fastrpc-netcat/COPYING

# common part, don't touch!

DEB_VERSION="`cat debian/version`"
DEB_ARCHITECTURE="all"
DEB_MAINTAINER="$MAINTAINER"
DEB_INSTALLED_SIZE="`du -k -s $WORKDIR | awk '{print $1;}'`"

mkdir -p $WORKDIR/DEBIAN
cat debian/control | sed "
    s/__PACKAGE__/${DEB_PACKAGE}/g
    s/__VERSION__/${DEB_VERSION}/g
    s/__ARCHITECTURE__/${DEB_ARCHITECTURE}/g
    s/__MAINTAINER__/${DEB_MAINTAINER}/g
    s/__INSTALLED_SIZE__/${DEB_INSTALLED_SIZE}/g
" > $WORKDIR/DEBIAN/control

for file in preinst postinst prerm postrm; do
    if [ -f debian/$file ]; then
        cp debian/$file $WORKDIR/DEBIAN
        chmod 755 $WORKDIR/DEBIAN/$file
    fi
done

mkdir -p packages
dpkg-deb -b $WORKDIR packages/${DEB_PACKAGE}_${DEB_VERSION}_${DEB_ARCHITECTURE}.deb

rm -rf $WORKDIR
