#!/usr/bin/env python3

import argparse
import os
import time
import subprocess as sub
import numpy as np
import docker
import shutil

def GetVideoInfo(pathToVideo):
    p = sub.Popen([ffprobe, '-v', 'error', '-of', 'flat=s=_', '-select_streams', 'v:0', '-show_entries', 'stream=height,width,avg_frame_rate',pathToVideo], stdout=sub.PIPE)
    r = dict()
    l = ['width', 'height','fps']
    i = 0
    for line in p.stdout:
        r[l[i]] = eval(line.decode('utf-8')[:-1].replace('"','').split('=')[-1]) 
        i+=1
    if p.wait() != 0:
        tqdm.write('Error: cannot read resolution of output video '+pathToVideo) 
        exit(1)
    r['fps'] = np.ceil(r['fps'])
    return r

def ParseResults(nbTileList, scenarioList, bandwidthList, nbUser, nbBandwidth, nbChunk):
    if not os.path.exists('outputs/results'):
        os.makedirs('outputs/results')
    out = 'outputs/results/Test_medianGap_Lag2_qual3_tile{}_gap0.030000.txt'
    for nbTile in nbTileList:
        with open(out.format(nbTile), 'w') as o:
            o.write('bandwidth Opt Opt_25 Opt_75 Vert Vert_25 Vert_75 Hori Hori_25 Hori_75 Opt_MSSSIM Opt_MSSSIM_25 Opt_MSSSIM_75 Vert_MSSSIM Vert_MSSSIM_25 Vert_MSSSIM_75 Hori_MSSSIM Hori_MSSSIM_25 Hori_MSSSIM_75\n')
            for bandwidth in sorted(bandwidthList):
                rPSNR = {'Opt': [], 'Vert': [], 'Hori': []}
                rMSSSIM = {'Opt': [], 'Vert': [], 'Hori': []}
                for scenario in ['Opt', 'Vert', 'Hori']:
                    for u in range(nbUser):
                        for b in range(nbBandwidth):
                            outDirPath = 'outputs/{}_{}_tile_user_{}_bandidth_{}_{}Mbps'.format(scenario, nbTile, u, b, bandwidth)
                            if os.path.exists(outDirPath):
                                with open('{}/quality.txt'.format(outDirPath), 'r') as i:
                                    first = True
                                    for line in i:
                                        if first:
                                            first = False
                                            continue
                                        vals = line.split(' ')
                                        msssim = float(vals[0])
                                        psnr = float(vals[1])
                                        rMSSSIM[scenario].append(msssim)
                                        if scenario == 'Opt' or len(rPSNR['Opt']) <= len(rPSNR[scenario]):
                                            rPSNR[scenario].append(psnr)
                                        else:
                                            rPSNR[scenario].append(psnr - rPSNR['Opt'][len(rPSNR[scenario])])
                #for scenario in ['Opt', 'Vert', 'Hori']:
                #    rPSNR[scenario] = sorted(rPSNR[scenario])
                opt = np.percentile(rPSNR['Opt'], 50) if len(rPSNR['Opt']) > 0 else -1
                opt25 = np.percentile(rPSNR['Opt'], 25) if len(rPSNR['Opt']) > 0 else -1
                opt75 = np.percentile(rPSNR['Opt'], 75) if len(rPSNR['Opt']) > 0 else -1
                vert = np.percentile(rPSNR['Vert'], 50) if len(rPSNR['Vert']) > 0 else -1
                vert25 = np.percentile(rPSNR['Vert'], 25) if len(rPSNR['Vert']) > 0 else -1
                vert75 = np.percentile(rPSNR['Vert'], 75) if len(rPSNR['Vert']) > 0 else -1
                hori = np.percentile(rPSNR['Hori'], 50) if len(rPSNR['Hori']) > 0 else -1
                hori25 = np.percentile(rPSNR['Hori'], 25) if len(rPSNR['Hori']) > 0 else -1
                hori75 = np.percentile(rPSNR['Hori'], 75) if len(rPSNR['Hori']) > 0 else -1
                optMSSSIM = np.percentile(rMSSSIM['Opt'], 50) if len(rMSSSIM['Opt']) > 0 else -1
                optMSSSIM25 = np.percentile(rMSSSIM['Opt'], 25) if len(rMSSSIM['Opt']) > 0 else -1
                optMSSSIM75 = np.percentile(rMSSSIM['Opt'], 75) if len(rMSSSIM['Opt']) > 0 else -1
                vertMSSSIM = np.percentile(rMSSSIM['Vert'], 50) if len(rMSSSIM['Vert']) > 0 else -1
                vertMSSSIM25 = np.percentile(rMSSSIM['Vert'], 25) if len(rMSSSIM['Vert']) > 0 else -1
                vertMSSSIM75 = np.percentile(rMSSSIM['Vert'], 75) if len(rMSSSIM['Vert']) > 0 else -1
                horiMSSSIM = np.percentile(rMSSSIM['Hori'], 50) if len(rMSSSIM['Hori']) > 0 else -1
                horiMSSSIM25 = np.percentile(rMSSSIM['Hori'], 25) if len(rMSSSIM['Hori']) > 0 else -1
                horiMSSSIM75 = np.percentile(rMSSSIM['Hori'], 75) if len(rMSSSIM['Hori']) > 0 else -1

                o.write('{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}\n'.format(bandwidth, opt, opt25, opt75, vert, vert25, vert75, hori, hori25, hori75, optMSSSIM, optMSSSIM25, optMSSSIM75, vertMSSSIM, vertMSSSIM25, vertMSSSIM75, horiMSSSIM, horiMSSSIM25, horiMSSSIM75))





if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Recompose the downloaded bitstream, extract viewport and compute quality metric')

    args = parser.parse_args()

    resultsPathId = 'results/Test_selectedSegment_Lag2_qual3_tile{nbTile}_gap0.030000_{ScenarioType}_{Bandwidth:.06f}Mbps.txt'

    MP4Box = 'MP4Box'
    ffmpeg = 'ffmpeg'
    ffprobe = 'ffprobe'
    trans = 'trans'

    dockerClient = docker.from_env()
    isRunningOnDocker = len(dockerClient.containers.list(filters={'name':'recontruct_tiled_mpv360_mmsys18'})) == 1

    nbTileList = [1, 6, 24]
    scenarioList = ['Opt', 'Vert', 'Hori']
    #bandwidthList = [3, 5, 7, 10, 15, 20]
    bandwidthList = [3, 7, 10]

    for nbTile in nbTileList:
        for scenario in scenarioList:
            for bandwidth in bandwidthList:
                viewpoints = None
                selectedTile = None
                nbUser = None
                nbBandwidth = None
                nbChunk = None

                # Parse the result file
                pathResult = resultsPathId.format(nbTile=nbTile, ScenarioType=scenario, Bandwidth=bandwidth)
                if not os.path.exists(pathResult):
                    print('Error:',pathResult,'doesn\'t exist')
                    exit(1)
                with open(pathResult, 'r') as i:
                    first = True
                    for line in i:
                        if first:
                            if nbUser is None:
                                vals = line.split(' ')
                                nbUser = int(vals[0])
                                nbBandwidth = int(vals[1])
                                viewpoints = [[] for u in range(nbUser*nbBandwidth)]
                                selectedTile = [[] for u in range(nbUser*nbBandwidth)]
                                continue
                            else:
                                first = False
                                continue
                        vals = line.split(' ')
                        chunkId = int(vals[0])
                        nbChunk = chunkId+1
                        counter = 1
                        for u in range(nbUser):
                            for b in range(nbBandwidth):
                                viewpoints[u*nbBandwidth+b].append(int(vals[counter]))
                                counter += 1
                                segs = []
                                for t in range(nbTile):
                                    segs.append(int(vals[counter]))
                                    counter += 1
                                selectedTile[u*nbBandwidth+b].append(segs)
                #DEBUG
                nbChunk = 32
                #nbUser = 1
                #nbBandwidth = 1
                for u in range(nbUser):
                    for b in range(nbBandwidth):
                        # Generate a temporary output video
                        dashInitPaths = 'dash_repr/cam{{vId}}_{nbTileNameOut}_{{qualityName}}mbps/baseballcam{{vId}}_4k_yuv_{tileName}{{qualityName}}mbps{track}init.mp4'.format(nbTileNameOut= 'notiles' if nbTile == 1 else ('3x2' if nbTile == 6 else '6x4'), tileName='' if nbTile == 1 else ('3x2_' if nbTile == 6 else '6x4_'), track='_dash' if nbTile == 1 else '_set1_')
                        dashSegPaths = 'dash_repr/cam{{vId}}_{nbTileNameOut}_{{qualityName}}mbps/baseballcam{{vId}}_4k_yuv_{tileName}{{qualityName}}mbps_dash{track}{{chunkId}}.m4s'.format(nbTileNameOut= 'notiles' if nbTile == 1 else ('3x2' if nbTile == 6 else '6x4'), tileName='' if nbTile == 1 else ('3x2_' if nbTile == 6 else '6x4_'), track='' if nbTile == 1 else '_track{trackId}_')
                        zero = 'zero_{nbTileNameOut}/zero_dash_track{{trackId}}_{{chunkId}}.m4s'.format(nbTileNameOut= 'notiles' if nbTile == 1 else ('3x2' if nbTile == 6 else '6x4'))

                        qualityNameList = [5, 8, 16]
                        for tileId in range(nbTile):
                            for qualityId in range(3):
                                for camId in range(3):
                                    initPath = dashInitPaths.format(vId=camId+1, trackId=tileId+2, chunkId=chunkId+1, qualityLevel=qualityId, qualityName=qualityNameList[qualityId])
                                    if not os.path.exists(initPath):
                                        print(initPath, 'doesn\'t exist')
                                        exit(1)
                                    for chunkId in range(nbChunk):
                                        segPath = dashSegPaths.format(vId=camId+1, trackId=tileId+2, chunkId=chunkId+1, qualityLevel=qualityId, qualityName=qualityNameList[qualityId])
                                        if not os.path.exists(segPath):
                                            print(segPath, 'doesn\'t exist')
                                            exit(1)

                        outDirPath = 'outputs/{}_{}_tile_user_{}_bandidth_{}_{}Mbps'.format(scenario, nbTile, u, b, bandwidth)
                        if os.path.exists('{}/quality.txt'.format(outDirPath)):
                            print('{}/quality.txt'.format(outDirPath), 'exists, computation skipped')
                            ParseResults(nbTileList, scenarioList, bandwidthList, nbUser, nbBandwidth, nbChunk)
                            continue
                        #Generate client video
                        print('Prepare the client video')
                        if not os.path.exists('/tmp/trans360'):
                            os.makedirs('/tmp/trans360')
                        pathToMP4File = '/tmp/trans360/myVideo_{}_{}.mp4'.format(u, b)
                        if nbTile > 1:
                            with open(pathToMP4File, 'wb') as o:
                                #add init file
                                with open(dashInitPaths.format(vId=1, qualityLevel=0, qualityName=qualityNameList[0]), 'rb') as i:
                                    o.write(i.read())
                                #add segments:
                                for chunkId in range(nbChunk):
                                    for tileId in range(nbTile):
                                        qualityId = selectedTile[u*nbBandwidth+b][chunkId][tileId]
                                        vId = viewpoints[u*nbBandwidth+b][chunkId]
                                        if tileId == 0:
                                            #Add metadata track (only once per chunk)
                                            with open( dashSegPaths.format(vId=1, trackId=1, chunkId=chunkId+1, qualityLevel=0, qualityName=qualityNameList[0]), 'rb') as i:
                                                o.write(i.read())
                                        if qualityId < 0:
                                            #print('skiped:', zero.format(trackId=tileId+2, chunkId=chunkId+1))
                                            #with open(zero.format(trackId=tileId+2, chunkId=chunkId+1), 'rb') as i:
                                            #    o.write(i.read())
                                            with open( dashSegPaths.format(vId=(vId+1)%3+1, trackId=tileId+2, chunkId=chunkId+1, qualityLevel=0, qualityName=qualityNameList[qualityId]), 'rb') as i:
                                                o.write(i.read())
                                        else:
                                            with open( dashSegPaths.format(vId=vId+1, trackId=tileId+2, chunkId=chunkId+1, qualityLevel=qualityId, qualityName=qualityNameList[qualityId]), 'rb') as i:
                                                o.write(i.read())
                            #Merge tracks together
                            sub.check_call([MP4Box, '-raw', '1', pathToMP4File], cwd='/tmp')
                            #Put the extracted stream into a MP4 container
                            sub.check_call([MP4Box, '-add', pathToMP4File[:-4]+'_track1.hvc', '-new', pathToMP4File], cwd='/tmp')
                            os.remove(pathToMP4File[:-4]+'_track1.hvc')
                        else:
                            #p = sub.Popen(['ffmpeg', '-i', '-', '-c:v', 'libx265', '-x265-params', 'lossless=1', '-y', pathToMP4File], stdin=sub.PIPE)
                            p = sub.Popen(['ffmpeg', '-i', '-', '-c:v', 'copy', '-y', pathToMP4File], stdin=sub.PIPE)
                            for chunkId in range(nbChunk):
                                tmp = '/tmp/vid{}.mp4'.format(chunkId)
                                with open(tmp, 'wb') as o:
                                    qualityId = selectedTile[u*nbBandwidth+b][chunkId][0]
                                    vId = viewpoints[u*nbBandwidth+b][chunkId]
                                    if qualityId < 0:
                                        print('Do not support currently having no segment download for a chunk in the no tile scenario')
                                        exit(1)
                                    with open(dashInitPaths.format(vId=vId+1, qualityLevel=qualityId, qualityName=qualityNameList[qualityId]), 'rb') as i:
                                        o.write(i.read())
                                    with open( dashSegPaths.format(vId=vId+1, trackId=tileId+2, chunkId=chunkId+1, qualityLevel=qualityId, qualityName=qualityNameList[qualityId]), 'rb') as i:
                                        o.write(i.read())
                                sub.check_call([MP4Box, '-raw', '1', tmp], cwd='/tmp')
                                sub.check_call([MP4Box, '-add', tmp[:-4]+'_track1.hvc', '-new', tmp], cwd='/tmp')
                                os.remove(tmp[:-4]+'_track1.hvc')
                                sub.check_call([ffmpeg, '-i', tmp, '-c:v', 'copy', '-bsf:v', 'hevc_mp4toannexb', '-f', 'mpegts', '-y', '-'], stdout=p.stdin)
                                os.remove(tmp)
                            p.stdin.close()
                            p.wait()
                        fps = 30
                        #processingStep = 6
                        processingStep = 15
                        print('Prepare configuration file')
                        pathToOriginal = '/tmp/trans360/original_4096x2048.y4m'
                        with open('/tmp/trans360/Config.ini', 'w') as o, open('Confi.ini.template', 'r') as i:
                            s = ''
                            for line in i:
                                s += line
                            o.write(s.format(originInputVid= pathToOriginal, clientInputVid= pathToMP4File, nbFrame= nbChunk*fps, processingStep=processingStep, NavigationTracePath= os.path.abspath('Navigations/Navigation_user_{}.txt'.format(u))))

                        #Generate original video
                        vidInfo = GetVideoInfo('original/cam1_4k_30fps.mp4')
                        res = '{width}x{height}'.format(**vidInfo)
                        if not os.path.exists('oneFrame.yuv'):
                            sub.check_call([ffmpeg, '-i', 'original/cam1_4k_30fps.mp4', '-c:v', 'rawvideo', '-f', 'rawvideo', '-s', res, '-pix_fmt', 'yuv420p', '-y', '-f', 'rawvideo', '-vframes', '1', 'oneFrame.yuv'])
                        oneFrameSize = os.path.getsize('oneFrame.yuv')
                        print('Prepare the orinal video and start trans')
                        try:

                            if not os.path.exists(pathToOriginal):
                                sub.check_call(['mkfifo', pathToOriginal])
                                #pass
                            pTrans = None
                            pYuv = sub.Popen([ffmpeg, '-c:v', 'rawvideo', '-f', 'rawvideo', '-s', '4096x2048', '-pix_fmt', 'yuv420p', '-r', '30', '-i', '-', '-pix_fmt', 'yuv420p', '-f', 'yuv4mpegpipe', '-vframes', '{}'.format(nbChunk*30), '-y', pathToOriginal], stdin=sub.PIPE, bufsize=(fps+2)*oneFrameSize)
                            p1 = sub.Popen([ffmpeg, '-i', 'original/cam1_4k_30fps.mp4', '-c:v', 'rawvideo', '-f', 'rawvideo', '-s', res, '-pix_fmt', 'yuv420p', '-y', '-f', 'rawvideo', '-'], stdout=sub.PIPE, bufsize=oneFrameSize)
                            p2 = sub.Popen([ffmpeg, '-i', 'original/cam2_4k_30fps.mp4', '-c:v', 'rawvideo', '-f', 'rawvideo', '-s', res, '-pix_fmt', 'yuv420p', '-y', '-f', 'rawvideo', '-'], stdout=sub.PIPE, bufsize=oneFrameSize)
                            p3 = sub.Popen([ffmpeg, '-i', 'original/cam3_4k_30fps.mp4', '-c:v', 'rawvideo', '-f', 'rawvideo', '-s', res, '-pix_fmt', 'yuv420p', '-y', '-f', 'rawvideo', '-'], stdout=sub.PIPE, bufsize=oneFrameSize)
                            l1 = b''
                            l2 = b''
                            l3 = b''

                            poller = None
                            for chunkId in range(nbChunk):
                                vId = viewpoints[u*nbBandwidth+b][chunkId]
                                for f in range(fps):
                                    while len(l1) < oneFrameSize:
                                        l1 += p1.stdout.read(oneFrameSize)
                                        l2 += p2.stdout.read(oneFrameSize)
                                        l3 += p3.stdout.read(oneFrameSize)
                                    byteRange = l1[:oneFrameSize] if vId == 0 else (l2[:oneFrameSize] if vId == 1 else l3[:oneFrameSize])
                                    pYuv.stdin.write(byteRange)
                                    l1 = l1[oneFrameSize:]
                                    l2 = l2[oneFrameSize:]
                                    l3 = l3[oneFrameSize:]
                                    if pTrans is None:
                                        #pTrans = sub.Popen([trans, '-c', 'Config.ini'])
                                        pTrans = dockerClient.containers.run('xmar/trans360', 'trans -c /tmp/trans360/Config.ini', auto_remove=True, volumes_from='recontruct_tiled_mpv360_mmsys18', detach=True)
                                        sub.Popen(['printContainerLogs.py', pTrans.id])
                                    #for line in pTrans.logs(stream=True):
                                    #    print(line)
                                    #    poller = select.poll()
                                    #    poller.register(pTrans.logs(stream=True))
                                    #if poller is not None:
                                    #    while poller.poll(1):
                                    #        print(pTrans.logs(stream=True).readline())

                        finally:
                            p1.stdout.close()
                            p2.stdout.close()
                            p3.stdout.close()
                            pYuv.stdin.close()
                            pYuv.wait()
                            if pTrans is not None:
                                #pTrans.terminate()
                                #pTrans.wait()
                                pTrans.stop()
                                pTrans = None
                            os.remove(pathToOriginal)

                        if not os.path.exists(outDirPath):
                            os.makedirs(outDirPath)
                        shutil.move('/tmp/trans360/output1_1FlatFixed.mkv', '{}/original.mkv'.format(outDirPath))
                        shutil.move('/tmp/trans360/output1_2FlatFixed.mkv', '{}/client.mkv'.format(outDirPath))
                        shutil.move('/tmp/trans360/output1_2FlatFixed.txt', '{}/quality.txt'.format(outDirPath))
                        ParseResults(nbTileList, scenarioList, bandwidthList, nbUser, nbBandwidth, nbChunk)

