svn diff http://reddwarf.local/svn/Edgar/tags/$1/data/maps http://reddwarf.local/svn/Edgar/trunk/data/maps --diff-cmd diff -x -uw >map_diff.diff
svn diff http://reddwarf.local/svn/Edgar/tags/$1/src http://reddwarf.local/svn/Edgar/trunk/src --diff-cmd diff -x -uw >src_diff.diff
svn diff http://reddwarf.local/svn/Edgar/tags/$1/data/scripts http://reddwarf.local/svn/Edgar/trunk/data/scripts --diff-cmd diff -x -uw >script_diff.diff
grep addMedal src_diff.diff > medal_diff.diff
grep ADD_MEDAL script_diff.diff >> medal_diff.diff
