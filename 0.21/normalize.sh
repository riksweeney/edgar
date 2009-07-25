#!/bin/bash
files=""
for i in `find /home/rik/Edgar -name *.wav`;do
	files+=$i
	files+=" "
done
normalize-audio -m $files
for i in `find /home/rik/Edgar -name *.wav`;do
	oggenc -q 2 $i
done
