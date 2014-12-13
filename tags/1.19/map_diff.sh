#!/bin/bash
svn diff https://svn.code.sf.net/p/legendofedgar/code/tags/$1/data/maps https://svn.code.sf.net/p/legendofedgar/code/trunk/data/maps --diff-cmd diff -x -uw >map_diff.diff
svn diff https://svn.code.sf.net/p/legendofedgar/code/tags/$1/src https://svn.code.sf.net/p/legendofedgar/code/trunk/src --diff-cmd diff -x -uw >src_diff.diff
svn diff https://svn.code.sf.net/p/legendofedgar/code/tags/$1/data/scripts https://svn.code.sf.net/p/legendofedgar/code/trunk/data/scripts --diff-cmd diff -x -uw >script_diff.diff
svn diff https://svn.code.sf.net/p/legendofedgar/code/tags/$1/data/patch https://svn.code.sf.net/p/legendofedgar/code/trunk/data/patch --diff-cmd diff -x -uw >patch_diff.diff
svn diff https://svn.code.sf.net/p/legendofedgar/code/tags/$1/locale https://svn.code.sf.net/p/legendofedgar/code/trunk/locale --diff-cmd diff -x -uw >locale_diff.diff
