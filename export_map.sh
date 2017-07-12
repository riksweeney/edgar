#!/bin/bash
maps=( map01 map02 map03 map04 map05 map06 map07 map08 map09 map10 map11 map12 map13 map14 map15 map16 map17 map18 map19 map20 map21 map22 map23 map24 map25 map26 )
#maps=( map03 )
for map in ${maps[@]}
do
	rm -f *.png
	
	./mapeditor $map export
	
	for y in {0..100000}
	do
		yy=`printf "%03d" $y`
		if [ ! -e "$yy"_"$map"_000.png ];
		then
			break
		fi
		for i in `ls "$yy"_"$map"_*.png`
		do
			convert "$i" -resize 25% temp_"$i".png
		done
		tiles=`ls temp_"$yy"_"$map"_*.png | wc -l`
		echo Combining $tiles columns
		montage -mode concatenate -tile "$tiles"x1 temp_"$yy"_"$map"_*.png "$yy"_"$map".png
	done
	
	tiles=`ls *_"$map".png | wc -l`
	echo Combining $tiles rows
	montage -mode concatenate -tile 1x"$tiles" *_"$map".png "$map".tiff
	
	rm -f *.png
done
