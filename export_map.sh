#!/bin/bash
maps=( map01 map02 map03 map04 map05 map06 map07 map08 map09 map10 map11 map12 map13 map14 map15 map16 map17 map18 map19 map20 map21 map22 map23 map24 map25 map26 )
for map in ${maps[@]}
do
	rm -f *.png
	rm -f *.bmp
	
	./mapeditor $map export
	
	for y in {0..100000}
	do
		if [ ! -e $y_$map_0.bmp ];
		then
			break
		fi
		tiles=`ls $y_$map_*.bmp | wc -1`
		montage -tile $tiles×1 $y_$map_*.bmp $y_$map.png
	done
	
	tiles=`ls *_$map.png | wc -1`
	montage -tile 1×$tiles *_$map.png $map.jpg
	
	rm -f *.png
	rm -f *.bmp
done
