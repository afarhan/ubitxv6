#!/bin/sh

ffmpeg -f alsa -channels 1 -sample_rate 8000   -i plughw:0 -f webm -c:a libopus -b:a 16000 udp://10.8.0.5:9999

#ffmpeg -f alsa -channels 1 -sample_rate 8000   -i plughw:0 -f mpegts -c:a libmp3lame udp://10.8.0.5:9999

