###### VARIABLES ############

APPNAME="edgar"
APPVERSION="0.1"
APPRELEASE="1"

APPDIR="$APPNAME-$APPVERSION/"

DISTNAME="$APPNAME-$APPVERSION-$APPRELEASE.win32.zip"

#############################

echo "Creating Windows Distribution for $APPNAME (Version $APPVERSION, Release $APPRELEASE)"

PATH=/opt/SDL-1.2.13/bin:$PATH

mkdir -p dist

cd dist

echo "Getting Subversion Tag $APPVERSION-$APPRELEASE"

#svn export http://reddwarf/svn/Edgar/tags/$APPVERSION $APPDIR
svn export http://reddwarf/svn/Edgar/trunk $APPDIR

cd $APPDIR

make -f makefile.cross

make -f makefile.cross buildpak

cp ~/.windows/runtime/* .

cp icons/edgar.ico .

makensis install.nsi

mv *.installer.exe ../

cd ..

rm -r $APPDIR
