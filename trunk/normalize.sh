#!/bin/bash
find . -type d -exec sh -c "normalize \"{}\"/*.wav" \;
for i in `find /home/rik/Edgar -name *.wav`;do
	oggenc -q 2 $i
done
