###### VARIABLES ############

APPNAME="edgar"
APPVERSION="0.1"
APPRELEASE="1"

APPDIR="$APPNAME-$APPVERSION/"

DISTNAME="$APPNAME-$APPVERSION-$APPRELEASE.win32.zip"

#############################

echo "Cleaning..."

mkdir -p dist
rm -rf dist/*
mkdir -p dist/$APPDIR

echo "Copying Files..."

cp makefile dist/$APPDIR
cp install.nsi dist/$APPDIR
cp icons/edgar.ico dist/$APPDIR
cp -r src dist/$APPDIR
cp -r data dist/$APPDIR
cp -r gfx dist/$APPDIR
cp -r music dist/$APPDIR
cp -r sound dist/$APPDIR
cp -r doc dist/$APPDIR
cp -r icons dist/$APPDIR
cp -r locale dist/$APPDIR

cd dist/$APPDIR

make pak
make buildpak
rm pak
rm *.o
make

cp ~/Windows/runtime/* .

rm *.o
rm -rf data
rm -rf gfx
rm -rf icons
rm -rf music
rm -rf sound
rm -rf src
rm -rf makefile
rm -rf makefile.windows

makensis install.nsi

mv *.installer.exe ../

cd ..

rm -r $APPDIR
