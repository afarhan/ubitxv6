#!/bin/sh

while true;
do
  ffmpeg  -f alsa -channels 1 -sample_rate 8000   -i plughw:0 -f webm -c:a libopus -b:a 16000 -listen 1 -seekable 0 -multiple_requests 1 http://0.0.0.0:9999
done

# serve with
# <audio src="http://<host_ip>:9999" type="audio/webm" controls />
