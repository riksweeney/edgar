svn diff http://reddwarf.local/svn/Edgar/tags/$1/data/maps http://reddwarf.local/svn/Edgar/trunk/data/maps --diff-cmd diff -x -uw >map_diff.diff
svn diff http://reddwarf.local/svn/Edgar/tags/$1/src http://reddwarf.local/svn/Edgar/trunk/src --diff-cmd diff -x -uw >src_diff.diff
