if [ ! -e test/test_replay1001.dat ]
then
	echo ./edgar -record test/test_replay1001.dat -saveonexit
	./edgar -record test/test_replay1001.dat -saveonexit
	exit
else
for i in {1002..100000}
do
REPLAY_FILE="test_replay"${i}.dat
if [ ! -e test/"$REPLAY_FILE" ]
then
	j=$((i - 1))
	echo ./edgar -load $j -record test/$REPLAY_FILE -saveonexit
	./edgar -load $j -record test/$REPLAY_FILE -saveonexit
	exit
fi
done
fi
