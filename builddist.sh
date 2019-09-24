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

RPMROOT="/home/$SUDO_USER/rpmbuild"

SPECNAME="$APPNAME.spec"

###### Main Operation ########

echo "Creating Distribution for $APPNAME (Version $APPVERSION, Release $APPRELEASE) ($ARCH)"

echo "Cleaning..."

rm -rf dist/*

mkdir -p dist

cd dist

echo "Getting Subversion Tag $APPVERSION-$APPRELEASE"

svn export https://github.com/riksweeney/edgar/tags/$APPVERSION $APPDIR

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
rm $APPDIR/*.cbp
rm $APPDIR/*.workspace
rm $APPDIR/*.rc
rm $APPDIR/*.gitignore

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

rm *.rpm

echo "Renaming..."

mv *.deb edgar-$APPVERSION-$APPRELEASE.$ARCH.deb

rm edgar.spec

cd ..

chown -R $SUDO_USER:$SUDO_USER dist

echo "All Done..."
