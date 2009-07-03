if [ $# -ne 1 ]
	then
	echo ""
	echo "Usage $0 <version>"
	echo ""
	exit 1
fi

###### VARIABLES ############

APPVERSION="$1"
APPRELEASE="1"

ARCH="i586"

APPNAME="edgar"

APPDIR="$APPNAME-$APPVERSION/"

DISTNAME="$APPNAME-$APPVERSION-$APPRELEASE.tar.gz"
DATADIRS="data music sound gfx font"

RPMROOT="/usr/src/rpm"

SPECNAME="$APPNAME.spec"

###### Main Operation ########

echo "Creating Distribution for $APPNAME (Version $APPVERSION, Release $APPRELEASE)"

rm -rf dist/*

mkdir -p dist

cd dist

echo "Cleaning..."

echo "Syncing ZIP data file..."

echo "Getting Subversion Tag $APPVERSION-$APPRELEASE"

svn export http://reddwarf/svn/Edgar/tags/$APPVERSION $APPDIR

echo "Removing unwanted data files..."

rm -rf $APPDIR/dev
rm $APPDIR/normalize.sh
rm $APPDIR/disclaimer.txt
rm $APPDIR/*.spec
rm $APPDIR/*.sh
rm $APPDIR/*.nsi
rm $APPDIR/makefile.windows
rm $APPDIR/makefile.amiga

for i in `find . -name *.wav`;do
	rm $i
done

for i in `find . -name *.xcf`;do
	rm $i
done

for i in `find . -name *.gif`;do
	rm $i
done

for i in `find . -name *.blend*`;do
	rm $i
done

for i in `find . -name *.jpg`;do
	rm $i
done

echo "Creating GZIP... $DISTNAME"

cp $APPDIR/edgar.spec.base $APPDIR/edgar.spec

for f in `ls $APPDIR/locale/*.mo`; do \
	FILE=`echo $f | cut -d'/' -f2 | cut -d'.' -f1`
	echo "/usr/share/locale/$FILE/LC_MESSAGES/edgar.mo" >> edgar.spec
done

tar zhcf $DISTNAME $APPDIR

echo "Removing Copied Data..."

rm -rf $APPDIR

echo "Preparing to Build..."

cp $DISTNAME $RPMROOT/SOURCES

echo "Building RPMs..."

cd ..

rpmbuild -bb $SPECNAME --target $ARCH --define "name $APPNAME" --define "version $APPVERSION" --define "release $APPRELEASE"

echo "Retrieving RPMs"

mv $RPMROOT/RPMS/$ARCH/*.* dist/

echo "Running Alien..."

cd dist

alien -k *.rpm

echo "Renaming..."

mv *.deb edgar-$APPVERSION-$APPRELEASE.i386.deb

cd ..

./buildwindows.sh $1

echo "All Done..."
