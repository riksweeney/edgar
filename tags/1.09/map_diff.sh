#!/bin/bash
svn diff svn://svn.code.sf.net/p/legendofedgar/code/tags/$1/data/maps svn://svn.code.sf.net/p/legendofedgar/code/trunk/data/maps --diff-cmd diff -x -uw >map_diff.diff
svn diff svn://svn.code.sf.net/p/legendofedgar/code/tags/$1/src svn://svn.code.sf.net/p/legendofedgar/code/trunk/src --diff-cmd diff -x -uw >src_diff.diff
svn diff svn://svn.code.sf.net/p/legendofedgar/code/tags/$1/data/scripts svn://svn.code.sf.net/p/legendofedgar/code/trunk/data/scripts --diff-cmd diff -x -uw >script_diff.diff
svn diff svn://svn.code.sf.net/p/legendofedgar/code/tags/$1/data/patch svn://svn.code.sf.net/p/legendofedgar/code/trunk/data/patch --diff-cmd diff -x -uw >patch_diff.diff
svn diff svn://svn.code.sf.net/p/legendofedgar/code/tags/$1/locale svn://svn.code.sf.net/p/legendofedgar/code/trunk/locale --diff-cmd diff -x -uw >locale_diff.diff
