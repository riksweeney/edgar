for i in {1001..100000}
do
REPLAY_FILE="test_replay"${i}.dat
if [ -e "$REPLAY_FILE" ]
then
	j=$((i - 1))
	echo ./edgar -load $j -playback $REPLAY_FILE
	./edgar -load $j -playback $REPLAY_FILE
else
	exit
fi
done
