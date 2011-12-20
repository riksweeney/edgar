#!/bin/bash
sudo modprobe snd-aloop
rm /home/rik/temp/*.bmp
./edgar -load 0 -playback replay.dat -bmpwrite /home/rik/temp
mv /home/rik/.asoundrc2 /home/rik/.asoundrc
sox --buffer 1024 -t alsa -w -s -r 44100 -c 2 hw:1,1 -t wav /home/rik/edgar.wav & ./edgar -load 0 -playback replay.dat && killall sox
mv /home/rik/.asoundrc /home/rik/.asoundrc2
rm /home/rik/edgar.mp4
ffmpeg -threads 2 -f image2 -r 62 -i "/home/rik/temp/edgar%06d.bmp" -sameq "/home/rik/edgar.mp4"
