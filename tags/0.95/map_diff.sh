#!/bin/bash
svn diff https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/tags/$1/data/maps https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/trunk/data/maps --diff-cmd diff -x -uw >map_diff.diff
svn diff https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/tags/$1/src https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/trunk/src --diff-cmd diff -x -uw >src_diff.diff
svn diff https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/tags/$1/data/scripts https://legendofedgar.svn.sourceforge.net/svnroot/legendofedgar/trunk/data/scripts --diff-cmd diff -x -uw >script_diff.diff
grep addMedal src_diff.diff > medal_diff.diff
grep ADD_MEDAL script_diff.diff >> medal_diff.diff
