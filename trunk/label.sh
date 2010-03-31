if [ $# -ne 2 ]
	then
	echo ""
	echo "Usage $0 <version> <\"tag comment\">"
	echo ""
	exit 1
fi

CURRENT_TOTAL=`grep TOTAL_SECRETS src/defs.h | cut -d' ' -f3`

SCRIPT_TOTAL=`grep -m 1 ADD_SECRET data/scripts/*.dat | wc -l`

MAP_TOTAL=`grep item/health_potion data/maps/*.dat | wc -l`

TOTAL=$(($SCRIPT_TOTAL+$MAP_TOTAL))

if [ "$TOTAL" != "$CURRENT_TOTAL" ]
then
	echo "Secret tally does not match. Found: $TOTAL"
	#sed -i 's/TOTAL_SECRETS [0-9]*/TOTAL_SECRETS $TOTAL/' src/defs.h
	exit 1
fi

svn copy http://reddwarf/svn/Edgar/trunk http://reddwarf/svn/Edgar/tags/$1 -m "Tagging $1"
