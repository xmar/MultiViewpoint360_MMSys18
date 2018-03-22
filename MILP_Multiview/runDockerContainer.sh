#!/bin/bash

echo "Start xmar/mmsys18_milp Docker container with command line: optimal -c Config.ini"
echo "Output will be stored in the outSol folder and cache data in the outSave folder"
echo "Do 'docker stop mmsys18_milp' to stop the container"

mkdir -p outSave && mkdir -p outSol
docker run -v `pwd`:/home/optimal --name mmsys18_milp --rm -i -t xmar/mmsys18_milp -c Config.ini

echo "Container stopped"
