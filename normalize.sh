#!/bin/bash
for i in `find /home/rik/Edgar -name *.wav`;do
	normalize-audio $i
done
for i in `find /home/rik/Edgar -name *.wav`;do
	oggenc -q 2 $i
done
