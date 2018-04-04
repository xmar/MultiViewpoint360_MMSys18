#!/bin/bash

if [ ! -d "dash_repr" ]; then
    echo "Download the video dataset"
    curl -C - -o mv360videos_mmsys18.tar http://dash.ipv6.enstb.fr/headMovements/mv360videos_mmsys18.tar
    tar xfva mv360videos_mmsys18.tar
fi
docker run --rm -t -i --name recontruct_tiled_mpv360_mmsys18 -v /var/run/docker.sock:/var/run/docker.sock -v `pwd`:/home/reconstruct -v /tmp/trans360 xmar/recontruct_tiled_mpv360_mmsys18 ./reconstruct_viewport_quality.py ${@:1:99}
