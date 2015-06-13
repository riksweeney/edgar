#!/bin/bash
svn diff https://github.com/riksweeney/edgar/tags/$1/data/maps https://github.com/riksweeney/edgar/trunk/data/maps --diff-cmd diff -x -uw >map_diff.diff
svn diff https://github.com/riksweeney/edgar/tags/$1/src https://github.com/riksweeney/edgar/trunk/src --diff-cmd diff -x -uw >src_diff.diff
svn diff https://github.com/riksweeney/edgar/tags/$1/data/scripts https://github.com/riksweeney/edgar/trunk/data/scripts --diff-cmd diff -x -uw >script_diff.diff
svn diff https://github.com/riksweeney/edgar/tags/$1/data/patch https://github.com/riksweeney/edgar/trunk/data/patch --diff-cmd diff -x -uw >patch_diff.diff
svn diff https://github.com/riksweeney/edgar/tags/$1/locale https://github.com/riksweeney/edgar/trunk/locale --diff-cmd diff -x -uw >locale_diff.diff
