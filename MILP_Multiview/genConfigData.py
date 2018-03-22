#!/usr/bin/env python3

import numpy as np
import random

if __name__ == '__main__':

    nbViewpoint = 3
    nbTileList = [1, 3*2, 6*4]
    #nbTileList = [1]
    #nbQuality = 4
    nbQuality = 3
    #nbChunk = 4*60
    nbChunk = 256
    #nbChunk = 60
    nbBandwidth = 1
    nbUser = 4
    nbProcessedChunk = 32
    #nbLagChunkList = [2,10]
    #nbLagChunkList = [2,3,4]
    nbLagChunkList = [2]

    optimalGap=0.03
    nbThread=4

    #averageLowBitrate = 5
    #averageHighBitrate = 35
    averageBitrateList = [5, 8, 16]
    avgBitrateList = [[5.00638565625, 8.00672046875, 16.01394303125], [5.0235795, 8.02069896875, 16.019751999999997], [5.0842264375, 8.08175678125, 16.080042812500004]]
    varBitrateList = [[0.05197598260550684, 0.13243587169603027, 0.5569402424963116], [0.013006378470749997, 0.043633303918936515, 0.3272058487585], [0.012634444530058589, 0.04158807113638965, 0.3401763092415898]]
    #averageLowQuality = 220
    #averageHighQuality = 5
    averageQualityList = [2.8642533333333335, 2.5503899999999997, 2.1635133333333334]
    varQualityList = [0.6041990998222223, 0.3490154629, 0.13785629982222222]
    #averageBandwidthList = [4, 7, 16, 24, 32, 48, 60]
    #averageBandwidthList = [4, 7, 10, 15, 20]
    #averageBandwidthList = [5, 10, 15, 20, 25, 30, 35, 40]
    #averageBandwidthList = [3, 4, 7, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60]
    #averageBandwidthList = [3, 4, 5, 7, 10, 15, 20, 25 ,30]
    averageBandwidthList = [3, 5, 7, 10, 15]
    #averageBandwidthList = [20, 25, 30, 35, 40, 45, 50, 55, 60]
    #averageBandwidthList = [4, 7, 10]
    #for i in range(15, 61):
    #    averageBandwidthList.append(i)

    outputUser = dict()
    outputAdaptationSet = dict()
    for nbTile in nbTileList:
        outputUser[nbTile] = dict()
        for userId in range(nbUser):
            #if userId != 0:
            #    outputUser[nbTile][userId] = 'scenarios/user_{nbViewpoint}_{nbTile}_{nbChunk}_{userId}.txt'.format(nbViewpoint=nbViewpoint, nbTile=nbTile, nbChunk=nbChunk, userId=userId)
            outputUser[nbTile][userId] = 'scenarios/user_{nbViewpoint}_{nbTile}_{nbChunk}_{userId}.txt'.format(nbViewpoint=nbViewpoint, nbTile=nbTile, nbChunk=nbChunk, userId=userId)
            #else:
            #    outputUser[nbTile][userId] = 'scenarios/user_{nbViewpoint}_{nbTile}_{nbChunk}.txt'.format(nbViewpoint=nbViewpoint, nbTile=nbTile, nbChunk=nbChunk)
        outputAdaptationSet[nbTile] = 'scenarios/adaptationSet_{nbViewpoint}_{nbTile}_{nbChunk}_{nbQuality}.txt'.format(nbViewpoint=nbViewpoint, nbTile=nbTile, nbChunk=nbChunk, nbQuality=nbQuality)

    outputBandwidth = dict()
    for averageBandwidth in averageBandwidthList:
        outputBandwidth[averageBandwidth] = dict()
        for nbLagChunk in nbLagChunkList:
            outputBandwidth[averageBandwidth][nbLagChunk] = dict()
            for bandwidthId in range(nbBandwidth):
                outputBandwidth[averageBandwidth][nbLagChunk][bandwidthId] = 'scenarios/bandwdithFake_{nbChunk}_{nbLagChunk}_{averageBandwidth}Mbps_id{bandwidthId}.txt'.format(nbChunk=nbChunk, nbLagChunk=nbLagChunk, averageBandwidth=averageBandwidth, bandwidthId=bandwidthId)

    random.seed(42)
    np.random.seed(42)

    #Configuration file
    with open('Config.ini', 'w') as o:
        o.write('[Global]\nNbViewpoint={nbViewpoint}\nNbScenario={nbScenario}\noptimalGap={optimalGap}\nnbThread={nbThread}\n'.format(nbViewpoint=nbViewpoint, nbScenario=len(averageBandwidthList)*len(nbTileList)*len(nbLagChunkList), optimalGap=optimalGap, nbThread=nbThread))
        counter = 0
        #for scenarioId in reversed(range(len(averageBandwidthList))):
        for scenarioId in range(len(averageBandwidthList)):
            for nbTileId in range(len(nbTileList)):
                for nbLagChunkId in range(len(nbLagChunkList)):
                    nbTile = nbTileList[nbTileId]
                    nbLagChunk = nbLagChunkList[nbLagChunkId]
                    o.write('Scenario{id}={name}\n'.format(id=counter, name='Test_{}_{}_{}_{}Mbps'.format(nbChunk, nbLagChunk, nbTile, averageBandwidthList[scenarioId])))
                    counter += 1
        o.write('\n')
        #for scenarioId in reversed(range(len(averageBandwidthList))):
        for scenarioId in range(len(averageBandwidthList)):
             for nbTile in nbTileList:
                for nbLagChunk in nbLagChunkList:
                    bandwidthConf = ""
                    for bandwidthId in range(nbBandwidth):
                        bandwidthConf += '{}../{}'.format(';' if bandwidthConf != "" else '', outputBandwidth[averageBandwidthList[scenarioId]][nbLagChunk][bandwidthId])
                    userConf = ""
                    for userId in range(nbUser):
                        userConf += '{}../{}'.format(';' if userConf != "" else '', outputUser[nbTile][userId])
                    o.write('[{name}]\nNbTile={nbTile}\nNbQuality={nbQuality}\nNbChunk={nbChunk}\nNbProcessedChunk={nbProcessedChunk}\nNbLagDownloadChunk={nbLagChunk}\nAdaptationSetConf=../{asName}\nBandwidthConf={bName}\nUserConf={uName}\n'.format(name='Test_{}_{}_{}_{}Mbps'.format(nbChunk, nbLagChunk, nbTile, averageBandwidthList[scenarioId]), nbTile=nbTile, nbChunk=nbChunk, nbProcessedChunk=nbProcessedChunk, nbQuality=nbQuality, nbLagChunk=nbLagChunk, asName=outputAdaptationSet[nbTile], uName=userConf, bName=bandwidthConf))
                    o.write('horizontalOptimal={}\n'.format('true'))# if averageBandwidthList[scenarioId] > 0.9*nbViewpoint*averageLowBitrate else 'false'))
                    o.write('optimal={}\n'.format('true'))
                    o.write('verticalOptimal={}\n'.format('true'))
                    o.write('avgBandwidth={}\n'.format(averageBandwidthList[scenarioId]))
                    o.write('\n')

    #Bandwidth file
    #bandwidthList = [max(0.1*averageBandwidthList[0], np.random.normal(averageBandwidthList[0], 0.15*averageBandwidthList[0])) for i in range(-nbLagChunk, nbChunk)]
    for averageBandwidth in averageBandwidthList:
        for nbLagChunk in nbLagChunkList:
            for bandwidthId in range(nbBandwidth):
                with open(outputBandwidth[averageBandwidth][nbLagChunk][bandwidthId], 'w') as ob:
                    ob.write('#chunId,bandwidth\n')
                    for chunId in range(-nbLagChunk, nbChunk):
                        ob.write('{},{}\n'.format(chunId, np.random.normal(averageBandwidth, 0.05*averageBandwidth)))
                        #ob.write('{},{}\n'.format(chunId, bandwidthList[chunId]*averageBandwidth/averageBandwidthList[0]))

    ##User:
    #for userId in range(nbUser):
    #    viewpointList = list()
    #    switchingTime = list()
    #    lastViewpoint = None
    #    for chunId in range(nbChunk):
    #        if lastViewpoint is None:
    #            currentViewpoint = random.randint(0,nbViewpoint-1)
    #        else:
    #            vList = [i for i in range(0, nbViewpoint)]
    #            pList = list()
    #            for v in vList:
    #                if v == lastViewpoint:
    #                    pList.append(35)
    #                else:
    #                    pList.append(1)
    #            pList = np.array(pList)/sum(pList)
    #            currentViewpoint = np.random.choice(vList, p=pList)
    #        if lastViewpoint != currentViewpoint:
    #            if lastViewpoint is not None:
    #                switchingTime.append(np.random.uniform())
    #            print('Switch at',chunId, 'from',lastViewpoint,'to',currentViewpoint)
    #        else:
    #            switchingTime.append(-1)
    #        lastViewpoint = currentViewpoint
    #        viewpointList.append(currentViewpoint)
    #    switchingTime.append(-1)
    #    visibility = list()
    #    for chunId in range(nbChunk):
    #        visiList = list()
    #        nbTile = 24
    #        totalVisi = 0
    #        for tileId in range(nbTile):
    #            if tileId != nbTile-1:
    #                visiList.append(random.randint(0,1000-totalVisi)/1000)
    #                totalVisi += int(visiList[tileId]*1000)
    #            else:
    #                visiList.append(1-totalVisi/1000)
    #        random.shuffle(visiList)
    #        visibility.append(visiList)

    #    for nbTile in nbTileList:
    #        if userId != 0:
    #            with open(outputUser[nbTile][userId], 'w') as ou:
    #                ou.write('#chunkId,viewpointId,tileId,visibilityRatio,switchingDecisionTime\n')
    #                for chunId in range(nbChunk):
    #                    currentViewpoint = viewpointList[chunId]
    #                    for viewpointId in range(nbViewpoint):
    #                        for tileId in range(nbTile):
    #                            if viewpointId != currentViewpoint:
    #                                ou.write('{},{},{},{},{}\n'.format(chunId, viewpointId, tileId, 0, switchingTime[chunId]))
    #                            else:
    #                                if nbTile == 24:
    #                                    visi = visibility[chunId][tileId]
    #                                elif nbTile == 6:
    #                                    if tileId == 5:
    #                                        visi = round(10000*sum(visibility[chunId][tileId*6:(tileId+1)*6]))/10000
    #                                    else:
    #                                        visi = round(1000*sum(visibility[chunId][tileId*6:(tileId+1)*6]))/1000
    #                                elif nbTile == 1:
    #                                    visi = round(1000*sum(visibility[chunId])/1000)
    #                                else:
    #                                    raise 'NOT SUPPORTED TILE NUMBER'
    #                                ou.write('{},{},{},{}, {}\n'.format(chunId, viewpointId, tileId, visi, switchingTime[chunId]))

    #AdaptationSet
    #for nbTile in nbTileList:
    #    with open(outputAdaptationSet[nbTile], 'w') as oas:
    #        oas.write('#chunkId,viewpointId,tileId,qualityId,distortion,bitrate\n')
    #        for chunId in range(nbChunk):
    #            for viewpointId in range(nbViewpoint):
    #                for tileId in range(nbTile):
    #                    for qualityId in range(nbQuality):
    #                        #avBitrate = (qualityId*(averageHighBitrate-averageLowBitrate)/(nbQuality-1) + averageLowBitrate)/nbTile
    #                        #avBitrate = averageBitrateList[qualityId]/nbTile
    #                        #avDistortion = (qualityId*(averageHighQuality-averageLowQuality)/(nbQuality-1) + averageLowQuality)**2
    #                        #bitrate = np.random.normal(avBitrate, 0.05*avBitrate/nbTile)
    #                        bitrate = np.random.normal(avgBitrateList[nbTileList.index(nbTile)][qualityId], varBitrateList[nbTileList.index(nbTile)][qualityId])/nbTile
    #                        #distortion = np.random.normal(avDistortion, avDistortion*0.005)
    #                        distortion = np.random.normal(averageQualityList[qualityId], varQualityList[qualityId])
    #                        #bitrate = max(bitrate, 0.05*avBitrate/nbTile)
    #                        distortion = min(255*255, max(0, distortion))
    #                        oas.write('{},{},{},{},{},{}\n'.format(chunId, viewpointId, tileId, qualityId, distortion, bitrate))

