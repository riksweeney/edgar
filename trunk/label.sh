if [ $# -ne 1 ];
then
	echo ""
	echo "Usage $0 <version>"
	echo ""
	exit 1
fi

declare CURRENT_TOTAL=0
declare SCRIPT_TOTAL=0
declare MAP_TOTAL=0
declare POTION_TOTAL=0
declare TOTAL=0

CURRENT_TOTAL=`grep TOTAL_SECRETS src/defs.h | cut -d' ' -f3`

SCRIPT_TOTAL=`grep -m 1 ADD_SECRET data/scripts/*.dat | wc -l`

MAP_TOTAL=`grep item/health_potion data/maps/*.dat | wc -l`

POTION_TOTAL=`grep -m 1 item/health_potion data/scripts/*.dat | wc -l`

TOTAL=$(($SCRIPT_TOTAL + $MAP_TOTAL + $POTION_TOTAL))

if [ "$TOTAL" != "$CURRENT_TOTAL" ];
then
	echo "Secret tally does not match"
	echo "Expected '$CURRENT_TOTAL'"
	echo "Found '$TOTAL'"
	#sed -i 's/TOTAL_SECRETS [0-9]*/TOTAL_SECRETS $TOTAL/' src/defs.h
	exit 1
fi

CURRENT_VERSION=`grep "VERSION =" makefile | cut -d ' ' -f 3`

if [ "$CURRENT_VERSION" != "$1" ];
then
	echo "Version mismatch in makefile"
	echo "Expected '$1'"
	echo "Found '$CURRENT_VERSION'"
	exit 1
fi

CURRENT_VERSION=`grep "VERSION =" makefile.windows | cut -d ' ' -f 3`

if [ "$CURRENT_VERSION" != "$1" ];
then
	echo "Version mismatch in makefile.windows"
	echo "Expected '$1'"
	echo "Found '$CURRENT_VERSION'"
	exit 1
fi

CURRENT_VERSION=`grep "VERSION =" makefile.amiga | cut -d ' ' -f 3`

if [ "$CURRENT_VERSION" != "$1" ];
then
	echo "Version mismatch in makefile.amiga"
	echo "Expected '$1'"
	echo "Found '$CURRENT_VERSION'"
	exit 1
fi

svn copy https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/trunk https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/tags/$1 -m "Tagging $1"

echo Update the messages.pot file
