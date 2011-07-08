for INPUT in `find gfx/boss/cave_boss -name *.gif`;do
	OUTPUT=${INPUT%.*}
	echo convert $INPUT $OUTPUT%02d.png
done
