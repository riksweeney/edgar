#!/bin/bash
for i in `ls gfx/*/*.png`; do
	count=`find data | xargs grep $i | wc -l`
	count2=`find src | xargs grep $i | wc -l`
	count=$(($count + $count2)
	if [ $count -eq 0 ];
	then
		echo $i is unused
	fi
done
