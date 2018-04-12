---
title:	"Dynamic Adaptive Streaming for Multi-Viewpoint Omnidirectional Videos: In proceeding of ACM Multimedia System (MMSys'18)"
author: Xavier Corbillon, Francesca De Simone, Gwendal Simon, and Pascal Frossard
date:	April 19, 2018
---

# ffmpeg and MP4Box Docker container

## Description

This folder contains a Docker file used to generate a docker image that contains ffmpeg and MP4box.

The docker image was built and tested using Docker version 18.03.0-ce.

## Get the compiled docker image

You can directly download the docker image by using the following command:

    docker pull xmar/ffmpeg_mp4box:mmsys18

## Build the Docker image

To build the image yourself, you can run the following command inside this folder:

    docker build -t xmar/ffmpeg_mp4box:mmsys18 .
