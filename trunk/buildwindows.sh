if [ $# -ne 1 ]
	then
	echo ""
	echo "Usage $0 <version>"
	echo ""
	exit 1
fi

###### VARIABLES ############

APPNAME="edgar"
APPVERSION="$1"
APPRELEASE="1"

APPDIR="$APPNAME-$APPVERSION/"

DISTNAME="$APPNAME-$APPVERSION-$APPRELEASE.win32.zip"

#############################

echo "Creating Windows Distribution for $APPNAME (Version $APPVERSION, Release $APPRELEASE)"

mkdir -p dist

cd dist

rm -rf $APPDIR

echo "Getting Subversion Tag $APPVERSION-$APPRELEASE"

svn export http://reddwarf/svn/Edgar/tags/$APPVERSION $APPDIR

echo "Removing unwanted data files..."

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

cd $APPDIR

make VERSION=$1 -j3

make buildpak

cp edgar.pak windows.pak

make clean

make -f makefile.windows VERSION=$1 -j3

cp /opt/Windows/lib/*.dll .

cp icons/edgar.ico .

mv windows.pak edgar.pak

for f in `ls $APPDIR/locale/*.mo`; do \
	FILE=`echo $f | cut -d'/' -f2 | cut -d'.' -f1`
	echo "Moving $FILE.mo to locale/$FILE/edgar.mo"
	mkdir -p locale/$FILE
	mv locale/$FILE.mo locale/$FILE/edgar.mo
done

makensis install.nsi

mv *.installer.exe ../edgar-$1-1.installer.exe

cd ..

rm -r $APPDIR
