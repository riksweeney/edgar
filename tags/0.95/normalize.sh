#!/bin/bash
for i in `find /home/rik/Edgar/sound -name *.wav`;do
	OUTPUT1=${i%.*}
	OUTPUT2=${i%.*}
	OUTPUT1+=_new.wav
	
	if [ ! -f $OUTPUT2.ogg ];
	then
		sox $i $OUTPUT1 gain -n
		mv $OUTPUT1 $OUTPUT2.wav
		oggenc -q 4 $i
	fi
done
