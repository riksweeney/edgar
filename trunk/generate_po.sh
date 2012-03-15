#!/bin/bash
FILES=`find . -name *.c`

xgettext --no-wrap --keyword='_' $FILES

msgcat --no-wrap locale/edgar.pot > t
mv t locale/edgar.pot
head -n 15 locale/edgar.pot > t
./po_creator >> t
msgcat t > locale/edgar.pot
