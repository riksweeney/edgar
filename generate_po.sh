#!/bin/bash
FILES=`find . -name *.c`

xgettext --no-wrap --keyword='_' $FILES
head -n 15 locale/edgar.pot > t
./po_creator >> t
mv t locale/edgar.pot
