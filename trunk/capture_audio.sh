rm /home/rik/temp/*.bmp
./edgar -load 0 -playback replay.dat -bmpwrite /home/rik/temp
mv /home/rik/.asoundrc2 /home/rik/.asoundrc
sox --buffer 1024 -t alsa -w -s -r 44100 -c 2 hw:1,1 -t wav /home/rik/edgar.wav & ./edgar -load 0 -playback replay.dat && killall sox
mv /home/rik/.asoundrc /home/rik/.asoundrc2
rm /home/rik/edgar.mp4
/home/rik/Documents/create_movie.sh
