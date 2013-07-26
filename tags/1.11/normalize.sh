#!/bin/bash
files=""
for i in `find . -name *.wav`;do
	encode=0
	none=${i%.*}
	
	if [ ! -f $none ]; then
		encode=1
	else
		wavAge=`stat -c %Y $i`
		oggAge=`stat -c %Y $none`
		
		if [ $oggAge -lt $wavAge ]; then
			encode=1
		fi
	fi
	
	if [ $encode = 1 ]; then
		oggenc -q 4 $i
		ogg=${none}.ogg
		mv $ogg $none
	fi
done
