#!/bin/bash
files=""
for i in `find . -name *.wav`;do
	files+=$i
	files+=" "
done
normalize-audio -m $files
for i in `find . -name *.wav`;do
	oggenc -q 4 $i
done
