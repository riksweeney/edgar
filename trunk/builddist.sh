#!/bin/bash
if [ $EUID -ne 0 ];
then
	echo "This must be run as root"
	exit 1
fi

if [ $# -ne 2 ];
then
	echo ""
	echo "Usage $0 <version> <release>"
	echo ""
	exit 1
fi

###### VARIABLES ############

APPVERSION="$1"
APPRELEASE="$2"

ARCH=`uname -m`

APPNAME="edgar"

APPDIR="$APPNAME-$APPVERSION/"

DISTNAME="$APPNAME-$APPVERSION-$APPRELEASE.tar.gz"
DFSGNAME="$APPNAME-$APPVERSION-$APPRELEASE-dfsg.tar.gz"

RPMROOT="/home/$SUDO_USER/rpmbuild"

SPECNAME="$APPNAME.spec"

###### Main Operation ########

echo "Creating Distribution for $APPNAME (Version $APPVERSION, Release $APPRELEASE) ($ARCH)"

echo "Cleaning..."

rm -rf dist/*

mkdir -p dist

cd dist

echo "Getting Subversion Tag $APPVERSION-$APPRELEASE"

svn export svn://svn.code.sf.net/p/legendofedgar/code/tags/$APPVERSION $APPDIR

echo "Getting latest translations from the trunk"

rm -rf $APPDIR/locale

svn export svn://svn.code.sf.net/p/legendofedgar/code/trunk/locale $APPDIR/locale

echo "Removing unwanted data files..."

cp $APPDIR/edgar.spec.base edgar.spec

rm -rf $APPDIR/dev
rm -rf $APPDIR/test
rm $APPDIR/disclaimer.txt
rm $APPDIR/*.spec*
rm $APPDIR/*.sh
rm $APPDIR/*.nsi
rm $APPDIR/*.txt
rm $APPDIR/*.dat
rm $APPDIR/makefile.*

for i in `find . -name *.wav | grep -v doc`;do
	rm $i
done

for i in `find . -name *.xcf | grep -v doc`;do
	rm $i
done

for i in `find . -name *.gif | grep -v doc`;do
	rm $i
done

for i in `find . -name *.blend* | grep -v doc`;do
	rm $i
done

for i in `find . -name *.jpg | grep -v doc`;do
	rm $i
done

echo "Creating GZIP... $DISTNAME"

for f in `ls $APPDIR/locale/*.po`; do \
	FILE=`echo $f | cut -d'/' -f4 | cut -d'.' -f1`
	LINE="/usr/share/locale/$FILE/LC_MESSAGES/edgar.mo"
	echo "Adding $LINE"
	echo $LINE >> edgar.spec
done

tar zhcf $DISTNAME $APPDIR

echo "Creating DFSG version"

cp ../doc/*dfsg* $APPDIR

cp edgar.spec $APPDIR

cd $APPDIR

rm -rf music/* sound/*

for i in `find . -name *.orig | grep -v doc`;do
	rm $i
done

cd ..

tar zhcf $DFSGNAME $APPDIR

echo "Removing Copied Data..."

rm -rf $APPDIR

echo "Preparing to Build..."

mkdir -p $RPMROOT
mkdir -p $RPMROOT/BUILD $RPMROOT/BUILDROOT $RPMROOT/RPMS $RPMROOT/SOURCES $RPMROOT/SPECS $RPMROOT/SRPMS

cp $DISTNAME $RPMROOT/SOURCES

echo "Building RPMs..."

rpmbuild -bb $SPECNAME --target $ARCH --define "name $APPNAME" --define "version $APPVERSION" --define "release $APPRELEASE"

echo "Retrieving RPMs"

mv $RPMROOT/RPMS/$ARCH/*.* .

rm -rf $RPMROOT

echo "Running Alien..."

alien -k *.rpm

echo "Renaming..."

mv *.deb edgar-$APPVERSION-$APPRELEASE.$ARCH.deb

rm edgar.spec

cd ..

chown -R $SUDO_USER:$SUDO_USER dist

echo "All Done..."
