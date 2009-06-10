if [ $# -ne 2 ]
	then
	echo ""
	echo "Usage $0 <version> <\"tag comment\">"
	echo ""
	exit 1
fi

svn copy http://reddwarf/svn/Edgar/trunk http://reddwarf/svn/BlobWars/tags/$1 -m "$2"
