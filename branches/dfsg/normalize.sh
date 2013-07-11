#!/bin/bash
files=""
for i in `find . -name *.wav`;do
	oggenc -q 4 $i
	ogg=${i%.*}.ogg
	none=${i%.*}
	mv $ogg $none
done
