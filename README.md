---
title:	"Multi-viewpoint 360: MMSys'18"
author: Xavier Corbillon, Francesca De Simone, Gwendal Simon, and Pascal Frossard
date:	April 19, 2018
---

# Multi-viewpoint 360: MMSys'18

## Description

This repository contains piece of software used to generate the results presented
the paper:

*Xavier Corbillon, Francesca De Simone, Gwendal Simon, and Pascal Frossard, &ldquo;Dynamic Adaptive Streaming for Multi-Viewpoint Omnidirectional Videos&rdquo;, in proc. of ACM Multimedia System (MMSys), 2018.*

Each folder contains a Dockerfile to generate a docker image.

The docker images were tested with *Docker version 18.03.0-ce*.

Each folder contains a README file that describe its content. 

| Folder name | Description |
|------------:|:-----------:|
| MILP_Multiview | Implementation of the MILP using C++ and IBM Cplex |
| ffmpeg_mp4box_container | Contains a dockerfile to generate an image with ffmpeg and MP4Box |
| reconstruct | Script using the output from the MILP to reconstruct the client received bit-streaming, to extract the viewport, and compute an objective metric |
