#!/bin/bash
for i in `find /home/rik/Edgar/sound -name *.wav`;do
	OUTPUT1=${i%.*}
	OUTPUT2=${i%.*}
	OUTPUT1+=_new.wav
	sox $i $OUTPUT1 gain -n
	mv $OUTPUT1 $OUTPUT2.wav
done
for i in `find /home/rik/Edgar/sound -name *.wav`;do
	oggenc -q 4 $i
done
