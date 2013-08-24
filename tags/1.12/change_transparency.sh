#!/bin/bash
# Change 127 0 127 -> 127 255 127
for i in `find gfx -name '*.png'`;do
echo convert $i -fill "#7fff7f" -opaque "#7f007f" -colorspace rgb -type TrueColor $i
done
