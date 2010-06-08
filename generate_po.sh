FILES=`find . -name *.c`

xgettext --no-wrap --keyword='_' $FILES
