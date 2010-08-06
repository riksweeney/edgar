#!/bin/bash
for i in `ls gfx/*/*.png`; do
	count=`find data | xargs grep $i | wc -l`
	if [ $count -eq 0 ]; then
		echo $i is unused
	fi
done
