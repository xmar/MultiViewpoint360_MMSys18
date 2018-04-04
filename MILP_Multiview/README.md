---
title:	"Multi-viewpoint 360: MMSys'18"
author: Xavier Corbillon, Francesca De Simone, Gwendal Simon, and Pascal Frossard
date:	April 19, 2018
---

# MILP

## Description

This folder contains the IBM Cplex C++ implementation of the MILP describe in the MMSys'18 paper (cf. README in the root folder).
A docker image can be used to run the MILP on the dataset.

Note that in the software source code, and in the results, the reactive scenario is called vertical and the proactive scenario is called horizontal (cf. paper to see the definition of the reactive/proactive scenario).

The docker image was built and tested using Docker version 18.03.0-ce.

## Run the containers

To run the software inside the docker container, you can run the script named *runDockerContainer.sh*:

    ./runDockerContainer.sh

If you do not have the docker image mmsys18_milp on your compute, it will download it. The script has to be run inside this folder.
The script will create two repository: *outSave* that contains raw save of previews iteration of the solver, and *outSol* that contains processed data about the already run iteration. 

The results in the outSol output are either text file readable by pgfplots in latex (ie. csv like files with first line  being the column name), or is a text file that indicates which video segment was downloaded (it is needed by the reconstruction script).

All the data needed to run the software is already in the git.

*Config.ini* contains the configuration of the MILP software.
*scenarios* folder contains the prepared data used by the software: tile visibility for each chunk for each users, average available bandwidth during the chunk, video bit-rate/PSNR for each tile for each chunk.

*genConfigData.py* is a script used to generate the configuration file and to generate the bandwidth. It is not needed to run this script because the output is already in the git.

## Get the compiled docker image

You can directly download the docker image by using the following command:

    docker pull xmar/mmsys18_milp:mmsys18

## Build the Docker image

To build the image yourself, you can run the following command inside this folder:

    docker build -t xmar/mmsys18_milp:mmsys18 .

