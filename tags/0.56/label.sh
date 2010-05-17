if [ $# -ne 1 ]
	then
	echo ""
	echo "Usage $0 <version>"
	echo ""
	exit 1
fi

declare CURRENT_TOTAL=0
declare SCRIPT_TOTAL=0
declare MAP_TOTAL=0
declare TOTAL=0

CURRENT_TOTAL=`grep TOTAL_SECRETS src/defs.h | cut -d' ' -f3`

SCRIPT_TOTAL=`grep -m 1 ADD_SECRET data/scripts/*.dat | wc -l`

MAP_TOTAL=`grep item/health_potion data/maps/*.dat | wc -l`

TOTAL=$(($SCRIPT_TOTAL+$MAP_TOTAL))

if [ "$TOTAL" != "$CURRENT_TOTAL" ]
then
	echo "Secret tally does not match"
	echo "Expected '$CURRENT_TOTAL'"
	echo "Found '$TOTAL'"
	#sed -i 's/TOTAL_SECRETS [0-9]*/TOTAL_SECRETS $TOTAL/' src/defs.h
	exit 1
fi

svn copy http://reddwarf/svn/Edgar/trunk http://reddwarf/svn/Edgar/tags/$1 -m "Tagging $1"
