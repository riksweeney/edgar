if [ ! -e test/test_replay1001.dat ]
then
	echo ./edgar -record test/test_replay1001.dat -saveonexit
	./edgar -record test/test_replay1001.dat -saveonexit
	zip test/test_replay1001.zip test/test_replay1001.dat
	exit
else
for i in {1002..100000}
do
REPLAY_FILE="test_replay"${i}
if [ ! -e test/"$REPLAY_FILE".dat ]
then
	j=$((i - 1))
	echo ./edgar -load $j -record test/$REPLAY_FILE.dat -saveonexit
	./edgar -load $j -record test/$REPLAY_FILE.dat -saveonexit
	zip test/$REPLAY_FILE.zip test/$REPLAY_FILE.dat
	exit
fi
done
fi
