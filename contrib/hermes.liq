#!/usr/bin/liquidsoap

## plays audio from a webcaster stream:
# https://github.com/webcast/webcaster
# https://webcast.github.io/webcaster/

set("decoder.mime_types.gstreamer",["video/webm", "audio/webm"])
set("decoder.mime_types.ffmpeg",["video/webm", "audio/webm"])

# Live DJ stuff
set("harbor.bind_addr","0.0.0.0")
set("harbor.verbose",true)
# livedj = input.harbor(
#       "mount",
#       port=8080,
#       password="hackme"
#)

output.alsa(fallible=true,bufferize = false, audio_to_stereo(input.harbor('mount',port=8080,password="hackme")))
