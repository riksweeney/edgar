for INPUT in `ls boss/grimlore/*.gif`; do
OUTPUT=${INPUT%.*}
convert $INPUT $OUTPUT%02d.png
done
