svn diff http://reddwarf.local/svn/Edgar/tags/$1/data/maps http://reddwarf.local/svn/Edgar/trunk/data/maps -x --ignore-eol-style >map_diff.diff
svn diff http://reddwarf.local/svn/Edgar/tags/$1/src http://reddwarf.local/svn/Edgar/trunk/src -x --ignore-eol-style >src_diff.diff
