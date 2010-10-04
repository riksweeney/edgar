#!/bin/bash
files=""
for i in `find /home/rik/Edgar/sound -name *.wav`;do
	files="$files $i"
done
normalize-audio -b $files
for i in `find /home/rik/Edgar/sound -name *.wav`;do
	oggenc -q 2 $i
done
