#!/bin/bash

docker run --rm -t -i --name recontruct_tiled_mpv360_mmsys18 -v /var/run/docker.sock:/var/run/docker.sock -v `pwd`:/home/reconstruct -v /tmp/trans360 xmar/recontruct_tiled_mpv360_mmsys18 ./reconstruct_viewport_quality.py
