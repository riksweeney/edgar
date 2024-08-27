#!/bin/bash
git diff $1 master -- data/maps > map_diff.diff
git diff $1 master -- src > src_diff.diff
git diff $1 master -- data/scripts > script_diff.diff
git diff $1 master -- data/patch > patch_diff.diff
git diff $1 master -- locale > locale_diff.diff
