for INPUT in `ls boss/grimlore/*.gif`; do
OUTPUT=${INPUT%.*}
echo convert $INPUT $OUTPUT%02d.png
done
