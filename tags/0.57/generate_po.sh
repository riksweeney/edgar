FILES=`find . -name *.c`

xgettext --keyword='_' $FILES
