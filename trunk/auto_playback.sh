for i in {1001..100000}
do
REPLAY_FILE="test_replay"${i}.dat
if [ -e test/"$REPLAY_FILE" ]
then
	j=$((i - 1))
	if [ $j = 1000 ]
	then
		echo ./edgar -playback test/$REPLAY_FILE
		./edgar -playback test/$REPLAY_FILE
	else
		echo ./edgar -load $j -playback test/$REPLAY_FILE
		./edgar -load $j -playback test/$REPLAY_FILE
	fi
else
	exit
fi
done
