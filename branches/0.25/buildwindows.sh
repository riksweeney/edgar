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

for i in `find . -name rock_2_4w.jpg`;do
	rm $i
done

cd $APPDIR

make VERSION=$1

make buildpak

cp edgar.pak windows.pak

make clean

make -f makefile.windows VERSION=$1

cp /opt/Windows/lib/*.dll .

cp icons/edgar.ico .

mv windows.pak edgar.pak

makensis install.nsi

mv *.installer.exe ../edgar-$1-1.installer.exe

cd ..

rm -r $APPDIR
