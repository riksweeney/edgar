#!/bin/bash -e
for i in {1001..100000}
do
REPLAY_FILE="test_replay"${i}
if [ -e test/"$REPLAY_FILE".zip ]
then
	j=$((i - 1))
	if [ $j = 1000 ]
	then
		unzip test/$REPLAY_FILE.zip
		echo ./edgar -playback test/$REPLAY_FILE.dat -saveonexit
		./edgar -playback test/$REPLAY_FILE.dat -saveonexit
		rm test/$REPLAY_FILE.dat
	else
		unzip test/$REPLAY_FILE.zip
		echo ./edgar -load $j -playback test/$REPLAY_FILE.dat -saveonexit
		./edgar -load $j -playback test/$REPLAY_FILE.dat -saveonexit
		rm test/$REPLAY_FILE.dat
	fi
else
	exit
fi
done
