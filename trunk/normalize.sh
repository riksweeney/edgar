#!/bin/bash
files=""
for i in `find . -name *.wav`;do
	oggenc -q 4 $i
done
