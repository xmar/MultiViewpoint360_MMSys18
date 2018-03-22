#include "scenario.hpp"

#include <iostream>
#include <cmath>
#include <limits>
#include <chrono>


#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#define INPUT_ARCHIVE(variableName, inputFile) boost::archive::binary_iarchive variableName(inputFile);
#define OUTPUT_ARCHIVE(variableName, outputFile) boost::archive::binary_oarchive variableName(outputFile);


using namespace IMT;

Solution Scenario::PrintResults(void) const
{
    int nbSimu = GetNbK();
    std::vector<float> wastedBandwidth(nbSimu*m_users.size()*m_bandwidths.size(), 0.f);
    std::vector<float> nbLag(nbSimu*m_users.size()*m_bandwidths.size(), 0);
    std::vector<float> distVisi(nbSimu*m_users.size()*m_bandwidths.size(), 0);
    std::vector<float> averageBandwidthUsage(nbSimu*m_users.size()*m_bandwidths.size(), 0);
    std::vector<float> totalBandwidth(nbSimu*m_users.size()*m_bandwidths.size(),0);
    std::vector<std::vector<float>> qs(nbSimu*m_users.size()*m_bandwidths.size());
    std::vector<std::vector<float>> qv(nbSimu*m_users.size()*m_bandwidths.size());
    std::vector<float> nbStall(nbSimu*m_bandwidths.size()*m_users.size(), 0);

    std::vector<std::vector<int>> displayedViewpoint;
    std::vector<std::vector<std::vector<int>>> selectedTileQuality;
    float avg_skipSwitch(0);

    for (int u = 0; u < m_users.size(); ++u)
    {
    for (int b = 0; b < m_bandwidths.size(); ++b)
    {
        displayedViewpoint.emplace_back(std::vector<int>(nbSimu*GetNbProcessedChunk(), -1));
        selectedTileQuality.emplace_back(std::vector<std::vector<int>>(nbSimu*GetNbProcessedChunk(), std::vector<int>(GetNbTile(), -1)));
    for (int k = 0; k < nbSimu; ++k)
    {
        std::vector<int> qualitySelected(GetNbQuality()+1, 0);
        int nbSelection(0);
        std::vector<float> qualitySelectedProportionnal(GetNbQuality()+1, 0);
        std::vector<float> qualityBitrate(GetNbQuality()+1, 0); 
        std::vector<float> qualityDistortion(GetNbQuality()+1, 0); 
        qualityDistortion[0] = NOT_DOWNLOADED_DISTORTION*GetNbChunk()*GetNbTile()*GetNbViewpoint();
        std::vector<float> networkUsage(GetNbChunk()+GetNbLagChunk(), 0);
        int nbWantedSwitch(0);
        int nbRealSwitch(0);
        int lastViewpoint = m_users[u].GetWantedViewpoint(k*GetNbProcessedChunk());
        int lastDisplayedViewpoint = m_solInfo[GetSolId(k, u, b)].m_displayedViewpoint[0];

        for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
        {
            int viewpointId = m_solInfo[GetSolId(k, u, b)].m_displayedViewpoint[chunkId];
            displayedViewpoint[u*m_bandwidths.size()+b][chunkId+k*GetNbProcessedChunk()] = viewpointId;
            if (viewpointId >= GetNbViewpoint())
            {
                std::cerr << "Error, no selected viewpoint for chunk " << chunkId << std::endl;
                exit(1);
            }
            if (!m_users[u].IsAtViewpoint(chunkId+k*GetNbProcessedChunk(), viewpointId))
            {
                nbLag[GetSolId(k, u, b)] += 1;
            }
            if (m_users[u].GetWantedViewpoint(chunkId+k*GetNbProcessedChunk()) != lastViewpoint)
            {
                ++nbWantedSwitch;
                lastViewpoint = m_users[u].GetWantedViewpoint(chunkId+k*GetNbProcessedChunk());
            }
            if (viewpointId != lastDisplayedViewpoint)
            {
                ++nbRealSwitch;
                lastDisplayedViewpoint = viewpointId;
            }
            for (int vId = 0; vId < GetNbViewpoint(); ++vId)
            {
                if (viewpointId != vId)
                {
                    for (int tileId = 0; tileId < GetNbTile(); ++tileId)                    
                    {
                        for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                        {
                            for(int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                            {
                                wastedBandwidth[GetSolId(k, u, b)] += m_solInfo[GetSolId(k, u, b)].m_downloadRatio.at(ToDownloadRatioVecId(chunkId, chunkLagId, vId, tileId, qualityLevel))*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), vId, tileId, qualityLevel).GetBitrate();
                                networkUsage[chunkLagId+GetNbLagChunk()] += m_solInfo[GetSolId(k, u, b)].m_downloadRatio.at(ToDownloadRatioVecId(chunkId, chunkLagId, vId, tileId, qualityLevel))*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), vId, tileId, qualityLevel).GetBitrate();
                            }
                        }
                    }
                }
            }

            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                float tileVisibility = m_users[u].GetTileVisibility(chunkId+k*GetNbProcessedChunk(), tileId);
                bool selected = false;
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    for(int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                    //for(int chunkLagId = -GetNbLagChunk(); chunkLagId < int(chunkId); ++chunkLagId)
                    {
                        networkUsage[chunkLagId+GetNbLagChunk()] += m_solInfo[GetSolId(k, u, b)].m_downloadRatio.at(ToDownloadRatioVecId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel))*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetBitrate();

                        if ( chunkLagId >= int(chunkId) && m_solInfo[GetSolId(k, u, b)].m_downloadRatio.at(ToDownloadRatioVecId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)) > 0)
                        {
                            std::cout << "Error: download chunk " << chunkId << " after deadline" << std::endl;
                        }
                    }
                    qualityBitrate[qualityLevel+1] += m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetBitrate();
                    qualityDistortion[qualityLevel+1] += m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetDistortion();
                    if (m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).IsSelected())
                    {
                        selected = true;
                        //std::cout << "Chunk " << chunkId << ": client selected " << viewpointId << " " << tileId << " " << qualityLevel << std::endl;
                        ++qualitySelected[qualityLevel+1];
                        qualitySelectedProportionnal[qualityLevel+1] += tileVisibility;
                        distVisi[GetSolId(k, u, b)] += tileVisibility*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetDistortion();
                        ++nbSelection;

                        selectedTileQuality[u*m_bandwidths.size()+b][chunkId+k*GetNbProcessedChunk()][tileId] = qualityLevel;
                    }
                    else
                    {
                        for(int chunkLagId = -GetNbLagChunk(); chunkLagId < int(chunkId); ++chunkLagId)
                        {
                            wastedBandwidth[GetSolId(k, u, b)] += m_solInfo[GetSolId(k, u, b)].m_downloadRatio.at(ToDownloadRatioVecId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel))*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetBitrate();
                        }
                    }
                    //std::cout << "Total " << chunkId << " "  << viewpointId << " " << tileId << " " << qualityLevel << ": " << GetTotalDownloadRatio(chunkId, viewpointId, tileId, qualityLevel) << std::endl;
                }
                if (!selected && tileVisibility > 0)
                {
                    ++qualitySelected[0];
                    qualitySelectedProportionnal[0] += tileVisibility;
                    ++nbSelection;
                }
            }
        }
        distVisi[GetSolId(k, u, b)] /= GetNbProcessedChunk();
        for(int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk()-1; ++chunkLagId)
        {
            averageBandwidthUsage[GetSolId(k, u, b)] += networkUsage[chunkLagId+GetNbLagChunk()];
            totalBandwidth[GetSolId(k, u, b)] += m_bandwidths[b].GetBandwidth(chunkLagId+k*GetNbProcessedChunk())*(1+m_solInfo[GetSolId(k, u, b)].m_nbStall[chunkLagId+GetNbLagChunk()]);
        }
        //std::cout << averageBandwidthUsage << " ";
        averageBandwidthUsage[GetSolId(k, u, b)] /= totalBandwidth[GetSolId(k, u, b)];
        //std::cout << averageBandwidthUsage << std::endl;

        std::cout << "\n### For processing " << k+1 << "/" << nbSimu << " user " << u+1 << "/" << m_users.size() << " bandwidth " << b+1 << "/" << m_bandwidths.size() << std::endl;
        std::cout << "Solution status " << m_solInfo[GetSolId(k, u, b)].m_solutionStatus << " configured gap " << m_optimalGap << std::endl;
        std::cout <<  "Objective value " << m_solInfo[GetSolId(k, u, b)].m_objectiveValue << " PSNR = " << 10*std::log10(255*255/(m_solInfo[GetSolId(k, u, b)].m_objectiveValue)) << " dB" << std::endl;
        //std::cout <<  "Objective value " << m_solInfo[GetSolId(k, u, b)].m_objectiveValue << " PSNR = " << 10*log(255*255/(m_solInfo[GetSolId(k, u, b)].m_objectiveValue)) << " dB" << std::endl;
        std::cout <<  "Solution gap " << m_solInfo[GetSolId(k, u, b)].m_solutionGap * 100 << "%\n" << std::endl;

        std::cout <<  "Average network usage: " << averageBandwidthUsage[GetSolId(k, u, b)] * 100 << "%" << std::endl;
        wastedBandwidth[GetSolId(k, u, b)] = wastedBandwidth[GetSolId(k, u, b)] / (averageBandwidthUsage[GetSolId(k, u, b)]*totalBandwidth[GetSolId(k, u, b)]);
        std::cout <<  "Average wasted network resources: " << wastedBandwidth[GetSolId(k, u, b)] * 100 << "%" << std::endl;
        for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
        {                                                                            
            nbStall[GetSolId(k, u, b)] += m_solInfo[GetSolId(k, u, b)].m_nbStall[chunkLagId+GetNbLagChunk()];
        }
        //std::cout <<  "Distortion visible: " << distVisi[k] << " PSNR = " << 10*log(255*255/(distVisi[k])) << " dB" << std::endl;
        std::cout <<  "Distortion visible: " << distVisi[GetSolId(k, u, b)] << " PSNR = " << 10*std::log10(255*255/(distVisi[GetSolId(k, u, b)])) << " dB" << std::endl;
        std::cout <<  "Nb stall: " << nbStall[GetSolId(k, u, b)] << std::endl;
        std::cout <<  "Nb switching lag: " << int(nbLag[GetSolId(k, u, b)]) << std::endl;
        std::cout <<  "Nb wanted switch: " << nbWantedSwitch << std::endl;
        std::cout <<  "Nb effective switch: " << nbRealSwitch << std::endl;
        avg_skipSwitch += (nbWantedSwitch-nbRealSwitch)/GetNbProcessedChunk();
        float val = -1;
        if (nbWantedSwitch != 0)
        {
            //nbLag[GetSolId(k, u, b)] /= nbWantedSwitch;
            val = nbLag[GetSolId(k, u, b)] / nbWantedSwitch;
        }
        else
        {
            nbLag[GetSolId(k, u, b)] = -1;
        }
        std::cout <<  "Average switching lag per wanted switch: " << val << "\n" << std::endl;

        for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
        {
            qs[GetSolId(k, u, b)].push_back(qualitySelected[qualityLevel+1]*100.f/(nbSelection));
            qv[GetSolId(k, u, b)].push_back(qualitySelectedProportionnal[qualityLevel+1]*100.f/(GetNbProcessedChunk()));
            std::cout << "Quality " << qualityLevel << ": selected " << qualitySelected[qualityLevel+1]*100.f/(nbSelection)
                << "\% of the tiles; proportion of visible viewports " << qualitySelectedProportionnal[qualityLevel+1]*100.f/(GetNbProcessedChunk())
                << "\% ("<< qualityBitrate[qualityLevel+1]/(GetNbChunk()*GetNbTile()*GetNbViewpoint()) <<"; " << qualityDistortion[qualityLevel+1]/(GetNbProcessedChunk()*GetNbTile()*GetNbViewpoint()) <<")" << std::endl;
        }
    }
    }
    }
    float avg_objectiveValue(0.f);
    float avg_solutionGap(0.f);
    float avg_wastedBandwidth(0.f);
    float avg_nbLag(0);
    std::vector<float> lags;
    std::vector<float> lags_inChunkNb;
    float avg_distVisi(0);
    std::vector<float> distVisi_psnr;
    float avg_averageBandwidthUsage(0);
    float avg_totalBandwidth(0);
    std::vector<float> avg_qs(GetNbQuality()+1, 0);
    std::vector<float> avg_qv(GetNbQuality()+1, 0);
    float avg_nbStall(0);
    std::vector<float> stalls;

    int counter(0);
    for (int u = 0; u < m_users.size(); ++u)
    {
    for (int b = 0; b < m_bandwidths.size(); ++b)
    {
    for (int k = 0; k < nbSimu; ++k)
    {
        avg_objectiveValue += m_solInfo[GetSolId(k, u, b)].m_objectiveValue/(nbSimu*m_users.size()*m_bandwidths.size());
        avg_solutionGap += m_solInfo[GetSolId(k, u, b)].m_solutionGap/(nbSimu*m_users.size()*m_bandwidths.size());
        avg_wastedBandwidth += wastedBandwidth[GetSolId(k, u, b)]/(nbSimu*m_users.size()*m_bandwidths.size());
        float defaultLag = 0;
        int nbSwitch(0);
        for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
        {
            auto sdt = m_users[u].GetSwitchingDecisionTime(chunkId+k*GetNbProcessedChunk());
            if (sdt >= 0)
            {
                defaultLag += 1 - sdt;
                ++nbSwitch;
            }
        }
        if (nbLag[GetSolId(k, u, b)] >= 0)
        {
            std::cout << nbLag[GetSolId(k, u, b)] << " " << (defaultLag + nbLag[GetSolId(k, u, b)])/nbSwitch << " " << nbLag[GetSolId(k, u, b)]/nbSwitch << " " << nbSwitch << std::endl;
            avg_nbLag += nbLag[GetSolId(k, u, b)];
            defaultLag += nbLag[GetSolId(k, u, b)];
            ++counter;
        }
        if (defaultLag > 0)
        {
            lags.push_back(defaultLag/nbSwitch);
            lags_inChunkNb.push_back(nbLag[GetSolId(k, u, b)]/nbSwitch);
        }
        avg_distVisi += distVisi[GetSolId(k, u, b)]/(nbSimu*m_users.size()*m_bandwidths.size());
        distVisi_psnr.emplace_back(10*std::log10(255*255/distVisi[GetSolId(k, u, b)]));
        avg_averageBandwidthUsage += averageBandwidthUsage[GetSolId(k, u, b)]/(nbSimu*m_users.size()*m_bandwidths.size());
        avg_totalBandwidth += totalBandwidth[GetSolId(k, u, b)] /(nbSimu*m_users.size()*m_bandwidths.size());
        avg_nbStall += nbStall[GetSolId(k, u, b)]/(nbSimu*m_users.size()*m_bandwidths.size());
        stalls.emplace_back(nbStall[GetSolId(k, u, b)]/GetNbProcessedChunk());
        for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
        {
            avg_qs[qualityLevel+1] += qs[GetSolId(k, u, b)][qualityLevel+1]/(nbSimu*m_users.size()*m_bandwidths.size());
            avg_qv[qualityLevel+1] += qv[GetSolId(k, u, b)][qualityLevel+1]/(nbSimu*m_users.size()*m_bandwidths.size());
        }
    }
    }
    }
    if (counter != 0)
    {
        avg_nbLag /= counter;
    }
    std::cout << "\n### Average results" << std::endl;
    //std::cout <<  "Objective value " << avg_objectiveValue << " PSNR = " << 10*log(255*255/(avg_objectiveValue)) << " dB" << std::endl;
    std::cout <<  "Objective value " << avg_objectiveValue << " PSNR = " << 10*std::log10(255*255/(avg_objectiveValue)) << " dB" << std::endl;
    std::cout <<  "Solution gap " << avg_solutionGap * 100 << "%\n" << std::endl;

    std::cout <<  "Average network usage: " << avg_averageBandwidthUsage * 100 << "%" << std::endl;
    std::cout <<  "Average wasted network resources: " << avg_wastedBandwidth* 100 << "%" << std::endl;
    //std::cout <<  "Distortion visible: " << avg_distVisi << " PSNR = " << 10*log(255*255/(avg_distVisi)) << " dB" << std::endl;
    std::cout <<  "Distortion visible: " << avg_distVisi << " PSNR = " << 10*std::log10(255*255/(avg_distVisi)) << " dB" << std::endl;
    std::cout <<  "Nb stall: " << avg_nbStall << std::endl;
    std::cout <<  "Average nb switching lag per wanted switch: " << avg_nbLag << "\n" << std::endl;

    for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
    {
        std::cout << "Quality " << qualityLevel << ": selected " << avg_qs[qualityLevel+1]
            << "\% of the tiles; proportion of visible viewports " << avg_qv[qualityLevel+1] << std::endl;
    }

    return Solution(m_users.size(), m_bandwidths.size(),avg_objectiveValue, avg_solutionGap, avg_distVisi, std::move(distVisi_psnr), avg_nbStall, std::move(stalls), avg_nbLag, std::move(lags), std::move(lags_inChunkNb), avg_skipSwitch, avg_averageBandwidthUsage*100.f, avg_wastedBandwidth*100.f, std::move(avg_qs), std::move(avg_qv), std::move(displayedViewpoint), std::move(selectedTileQuality));
}

void Scenario::Run(std::string outputScenarioId, std::string printableScenarioName, double scenariosAverageDuration, int scenarioId, int nbScenarios)
{
    int remainingScenarios = nbScenarios-scenarioId;
    double averageDuration = scenariosAverageDuration/(m_users.size()*m_bandwidths.size()*(GetNbK()));
    int counter(1);
    for (int u = 0; u < m_users.size(); ++u)
    {
    for (int b = 0; b < m_bandwidths.size(); ++b)
    {
    for (int k = 0; k < GetNbK(); ++k)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        auto outSaveName = outputScenarioId+"_"+std::to_string(u)+"_"+std::to_string(b)+"_"+std::to_string(k)+".save.bin.gz";
        std::cout << printableScenarioName << std::endl;
        std::cout << "Total spent time: " << print_time<long>(scenariosAverageDuration*scenarioId+averageDuration*(u*m_bandwidths.size()*(GetNbK())+b*(GetNbK())+k)) << std::endl;
        std::cout << "Estimated remaining time: " << print_time<long>(remainingScenarios*scenariosAverageDuration) << std::endl;
        std::cout << "Ajusted estimated remaining time: " << print_time<long>((remainingScenarios-1)*scenariosAverageDuration-(counter-1)*averageDuration+(m_users.size()*m_bandwidths.size()*(GetNbK())-counter-1)*averageDuration) << std::endl;
        std::cout << "Start processing " << k + 1 << "/" << GetNbK() << " user " << u+1 << "/" << m_users.size() << " bandwidth " << b+1 << "/" << m_bandwidths.size() << " " << 100*float(u*m_bandwidths.size()*(GetNbK())+b*(GetNbK())+k)/((GetNbK())*m_bandwidths.size()*m_users.size()) << "\% of this scenario" << std::endl;
		if (boost::filesystem::exists(outSaveName))
		{
			std::cout << "Load stored results" << std::endl;
        	std::ifstream ifs(outSaveName, std::ios_base::in | std::ios_base::binary);
        	boost::iostreams::filtering_istream in;                                
        	in.push(boost::iostreams::gzip_decompressor());                        
        	in.push(ifs);                                                          
        	INPUT_ARCHIVE (ia,in);                                                 
        	ia >> m_solInfo[GetSolId(k, u, b)];
		}
		else
		{
			std::cout << "Run processing" << std::endl;
        	InitCplexVariables(k, u, b);
        	//Set constraints
        	std::cout << "Set contraints: " << std::flush;
        	SetBandwidthConstraint(k, u, b);
        	//std::cout << "*"  << std::flush;
        	//SetTransmissionDelayConstraint(k);
        	std::cout << "*"  << std::flush;
        	SetDownloadOnceConstraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetDownloadDeadlineConstraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetSelectAndDownloadedConstraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetSelectOneIfVisibleConstraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetDisplayOneAndOnlyOneViewpointContraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetSelectIfDisplayedViewpointConstraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetViewpointSwitchingConstraint(k, u, b);
        	std::cout << "*" << std::flush;
        	SetDownloadedChunkIndicator(k, u, b);
        	std::cout << "*" << std::flush;
        	SetInOrderSelection(k, u, b);
        	std::cout << "*" << std::flush;
        	RunImpl(k, u, b);
        	std::cout << "*" << std::endl;
        	//Presolve
        	std::cout << "Static Presolve" << std::endl;
        	Presolve(k, u, b);
        	//Objective
        	std::cout << "Set objective" << std::endl;
        	SetObjective(k, u, b);
        	//Run CPLEX
        	std::cout << std::endl << "Ready to run" << std::endl;

        	IloCplex cplex(m_model);
        	//cplex.setParam(IloCplex::EpGap, m_conf->epGap);
        	cplex.setParam(IloCplex::EpGap, m_optimalGap);
        	cplex.setParam(IloCplex::TiLim, 600);
        	//cplex.setParam(IloCplex::Threads, m_conf->nbThread);
        	cplex.setParam(IloCplex::Threads, m_nbThread);
        	//cplex.setParam(IloCplex::MIPOrdType, 2);
        	cplex.setParam(IloCplex::BrDir, -1);
        	cplex.setParam(IloCplex::Probe, 3);
        	//cplex.setParam(IloCplex::PolishAfterNode, 2000);
        	cplex.setParam(IloCplex::PolishAfterTime, 350);
        	//cplex.setParam(IloCplex::MIPEmphasis, 1);
        	cplex.setParam(IloCplex::RINSHeur, 25);
        	cplex.setParam(IloCplex::DisjCuts, 2);
        	// cplex.setParam(IloCplex::NodeFileInd,3);
        	// cplex.setParam(IloCplex::WorkDir,".");
        	// cplex.setParam(IloCplex::MemoryEmphasis,true);
        	//cplex.setParam(IloCplex::DataCheck, true);

        	//cplex.exportModel((std::string("model.lp")).c_str());

        	std::cout << "Start solving" << std::endl;

        	cplex.solve();

        	std::cout << "Solving done\n" << std::endl;

        	std::cout << "Solution status " << cplex.getStatus() << std::endl;
        	std::cout <<  "Objective value " << cplex.getObjValue() << std::endl;
        	std::cout <<  "Solution gap " << cplex.getMIPRelativeGap() << "\n" << std::endl;
        	//GetResults
        	std::cout << "Store results" << std::endl;
        	GetResults(cplex, k, u, b);
        	//
        	std::cout << "Start Cleanup" << std::endl;
        	ClearCplexVariablesImpl(k, u, b);
        	ClearCplexVariables(cplex, k, u, b);
        	std::cout << "Cleanup done" << std::endl;
			std::ofstream ofs(outSaveName, std::ios_base::out | std::ios_base::binary);
        	boost::iostreams::filtering_ostream out;                         
        	out.push( boost::iostreams::gzip_compressor());                  
        	out.push( ofs );                                                 
        	OUTPUT_ARCHIVE (oa,out);                                         
        	oa << m_solInfo[GetSolId(k, u, b)];    
		}
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime ).count()/1000.f;
        averageDuration = (averageDuration*(counter-1) + duration)/(counter);
        ++counter;
    }
    }
    }
}

void Scenario::InitCplexVariables(int k, int u, int b)
{
    m_env = IloEnv();
    m_model = IloModel(m_env);
    m_x = IloIntVarArray(m_env, m_nbProcessedChunk*m_nbViewpoint*m_nbTiles*(1+m_nbQuality), 0, 1);
    m_y = IloIntVarArray(m_env, m_nbProcessedChunk*m_nbViewpoint*(m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1), 0, 1);
    m_stall = IloIntVarArray(m_env, GetNbProcessedChunk()+GetNbLagChunk(), 0, 250);
    m_stallQad = IloNumVarArray(m_env, GetNbProcessedChunk()+GetNbLagChunk(), 0, 250*250, ILOFLOAT);
    //m_d = IloNumVarArray(m_env, m_nbChunk*(m_nbChunk+m_nbLagChunk)*m_nbViewpoint*m_nbTiles*m_nbQuality, 0, 1, ILOFLOAT);
    m_d = IloNumVarArray(m_env, m_nbProcessedChunk*(m_nbProcessedChunk+m_nbLagChunk)*m_nbViewpoint*m_nbTiles*m_nbQuality, 0, D_MAX, ILOFLOAT);
    m_ds = IloIntVarArray(m_env, m_nbProcessedChunk*(m_nbProcessedChunk+m_nbLagChunk), 0, 1);
    m_varr = IloNumVarArray(m_env, GetNbProcessedChunk()-1, 0, 100, ILOFLOAT);
    m_displayedQuality = IloNumVarArray(m_env, GetNbProcessedChunk(), 0, NOT_DOWNLOADED_DISTORTION, ILOFLOAT); //qualityVariance
    m_stallEvent = IloIntVarArray(m_env, GetNbProcessedChunk()+GetNbLagChunk(), 0, 1);
}

void Scenario::ClearCplexVariables(IloCplex& cplex, int k, int u, int b)
{
    m_stallEvent.end();
    m_stallQad.end();
    m_displayedQuality.end();
    m_varr.end();
    m_ds.end();
    m_d.end();
    m_stall.end();
    m_y.end();
    m_x.end();
    cplex.end();
    m_model.end();
    m_env.end();
}

void Scenario::SetBandwidthConstraint(int k, int u, int b)
{
    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    {
        IloExpr sum1a(m_env);
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
                    {
                            sum1a += m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetBitrate() * (m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)]/float(D_MAX));
                    }
                }
            }
        }
        IloConstraint ic1a = sum1a <= m_bandwidths[b].GetBandwidth(chunkLagId+k*GetNbProcessedChunk())*(1+STALL_COUNT_IN_SECOND*m_stall[chunkLagId+GetNbLagChunk()]);
        //ic1a.setName(("ConstBandwidth["+std::to_string(chunkLagId)+"]").c_str());
        m_model.add(ic1a);
    }
}

void Scenario::SetTransmissionDelayConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        IloExpr sum(m_env);
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                 for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                 {
                    sum += m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetBitrate() * (m_d[GetDId(chunkId, chunkId-1, viewpointId, tileId, qualityLevel)]/float(D_MAX));
                 }
            }
        }
        IloConstraint ic = sum <= m_bandwidths[b].GetBandwidth(chunkId-1+k*GetNbProcessedChunk())*(1-transmissionDelay+STALL_COUNT_IN_SECOND*m_stall[chunkId-1+GetNbLagChunk()]);
        m_model.add(ic);
    }
}

void Scenario::SetDownloadOnceConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    IloExpr sum1b(m_env);
                    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                    {
                            sum1b += m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)];
                    }
                    IloConstraint ic1b = sum1b <= D_MAX;
                    m_model.add(ic1b);

                }
            }
        }
   }
}

void Scenario::SetDownloadDeadlineConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    for (int chunkLagId = chunkId; chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                    {
                            m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)].setUB(0);
                    }
                }
            }
        }
   }
}

void Scenario::SetSelectAndDownloadedConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    IloExpr sum1d(m_env);
                    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                    {
                            sum1d += m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)];
                    }
                    IloConstraint ic1d = sum1d == D_MAX*m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)];
                    m_model.add(ic1d);
                }
            }
        }
   }
}

void Scenario::SetSelectOneIfVisibleConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
        {
            IloExpr sum(m_env);
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    sum += m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)];
                }
            }
            IloConstraint ic1e = IloNum(std::ceil(m_users[u].GetTileVisibility(chunkId+k*GetNbProcessedChunk(), tileId))) <= sum;
            m_model.add(ic1e);
        }
    }
}

void Scenario::SetDisplayOneAndOnlyOneViewpointContraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        IloExpr sum(m_env);
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
            {
                sum += m_y[GetYId(chunkId, viewpointId, nbSwitch, u, k)];
            }
        }
        IloConstraint ic1f = sum == IloNum(1);
        m_model.add(ic1f);
    }
}

void Scenario::SetSelectIfDisplayedViewpointConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
        {
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                IloExpr sum(m_env);
                for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
                {
                    sum += m_y[GetYId(chunkId, viewpointId, nbSwitch, u, k)];
                }
                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    IloConstraint ic1g = m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)] <= sum;
                    m_model.add(ic1g);
                }
            }
        }
    }
}

void Scenario::SetViewpointSwitchingConstraint(int k, int u, int b)
{
    for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
    {
        for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
        {
            if (nbSwitch == 0 && m_users[u].IsAtViewpoint(k*GetNbProcessedChunk(), viewpointId))
            {
                m_y[GetYId(0, viewpointId, nbSwitch, u, k)].setLB(1);
            }
            else
            {
                m_y[GetYId(0, viewpointId, nbSwitch, u, k)].setUB(0);
            }
        }
    }
    for (int chunkId = 1; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
            {
                if (nbSwitch == m_users[u].GetNbSwitchUntil(chunkId, k, GetNbProcessedChunk()))
                {
                    m_model.add( m_y[GetYId(chunkId, viewpointId, nbSwitch, u, k)] <= m_y[GetYId(chunkId-1, viewpointId, m_users[u].GetNbSwitchUntil(chunkId-1, k, GetNbProcessedChunk()), u, k)] + (m_users[u].IsAtViewpoint(chunkId+k*GetNbProcessedChunk(), viewpointId) ? 1 : 0));
                }
                else
                {
                    m_y[GetYId(chunkId, viewpointId, nbSwitch, u, k)].setUB(0);
                }
            }
        }
    }
}

void Scenario::SetDownloadedChunkIndicator(int k, int u, int b)
{
    for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    {
        for (int  chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
        {
            IloExpr sum(m_env);
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
                {
                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                    {
                        sum += m_d[GetDId(chunkId,chunkLagId, viewpointId, tileId, qualityLevel)];
                    }
                }
            }
            m_model.add((m_ds[GetDsId(chunkId, chunkLagId)] == 0) ==  (sum == 0));
	    //m_model.add( sum <= D_MAX*m_ds[GetDsId(chunkId, chunkLagId)] );
	    //m_model.add( std::numeric_limits<double>::min()*m_ds[GetDsId(chunkId, chunkLagId)] <= sum );
        }
    }
    //DEBUG add new test constrains
    
    for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    {
         //m_model.add(250*m_stallEvent[chunkLagId + GetNbLagChunk()] >= m_stall[chunkLagId + GetNbLagChunk()]);
         m_model.add( (m_stallEvent[chunkLagId + GetNbLagChunk()] == 0) == (m_stall[chunkLagId + GetNbLagChunk()] == 0) );
    }
    //for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    //{
    //     m_model.add(m_stallQad[chunkLagId + GetNbLagChunk()] >= m_stall[chunkLagId + GetNbLagChunk()]*m_stall[chunkLagId + GetNbLagChunk()]);
    //}


    //for (int  chunkId = 0; chunkId < GetNbProcessedChunk()-1; ++chunkId)
    //{
    //     m_model.add( m_varr[chunkId] >= m_displayedQuality[chunkId]-m_displayedQuality[chunkId+1] );
    //     m_model.add( m_varr[chunkId] >= m_displayedQuality[chunkId+1]-m_displayedQuality[chunkId] );
    //}
    //for (int  chunkId = 0; chunkId < GetNbProcessedChunk()-1; ++chunkId)
    //{
    //   IloExpr sumi(m_env);
    //   IloExpr sumiPlusOne(m_env);
    //   for (int tileId = 0; tileId < GetNbTile(); ++tileId)
    //   {
    //       float tileVisibility = m_users[u].GetTileVisibility(chunkId+k*GetNbProcessedChunk(), tileId);
    //        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
    //        {
    //            for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
    //            {
    //                if (qualityLevel == -1)
    //                {
    //                    sumi += m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)]*NOT_DOWNLOADED_DISTORTION*tileVisibility;
    //                    sumiPlusOne += m_x[GetXId(chunkId+1, viewpointId, tileId, qualityLevel)]*NOT_DOWNLOADED_DISTORTION*tileVisibility;
    //                }
    //                else
    //                {
    //                    sumi += m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)]*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetDistortion()*tileVisibility;
    //                    sumiPlusOne +=  m_x[GetXId(chunkId+1, viewpointId, tileId, qualityLevel)]*m_as.GetSeg(chunkId+1+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetDistortion()*tileVisibility;
    //                }
    //            }
    //        }
    //   }
    //   m_model.add( m_varr[chunkId] >= sumi - sumiPlusOne );
    //   m_model.add( m_varr[chunkId] >= sumiPlusOne - sumi );
    //}
}

void Scenario::SetInOrderSelection(int k, int u, int b)
{
    //for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    //{
    //    for (int  chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    //    {
    //        int counter = 0;
    //        int counter2 = 0;
    //        IloExpr sum(m_env);
    //        for (int  chunkLagIdPrime = chunkLagId+1; chunkLagIdPrime < GetNbChunk(); ++chunkLagIdPrime)
    //        {
    //            //for (int  chunkIdPrime = std::max(chunkId-1, 0); chunkIdPrime < chunkId; ++chunkIdPrime)
    //            for (int  chunkIdPrime = 0; chunkIdPrime < chunkId; ++chunkIdPrime)
    //            {
    //                for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
    //                {
    //                    for (int tileId = 0; tileId < GetNbTile(); ++tileId)
    //                    {
    //                        for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
    //                        {
    //                            //sum += m_d[GetDId(chunkIdPrime, chunkLagIdPrime, viewpointId, tileId, qualityLevel)];
    //                            //++counter;
    //                        }
    //                    }
    //                }
    //                sum += m_ds[GetDsId(chunkIdPrime, chunkLagIdPrime)];
    //                ++counter2;
    //            }
    //        }
    //        //m_model.add(sum <= (D_MAX*counter + counter2)*(1-m_ds[GetDsId(chunkId, chunkLagId)]));
    //        m_model.add(IloIfThen(m_env, m_ds[GetDsId(chunkId, chunkLagId)] == 1, sum == 0));
    //    }
    //}
}


void Scenario::SetObjective(int k, int u, int b)
{
    //IloExpr distortion(m_env);
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
       IloExpr distortion(m_env);
       for (int tileId = 0; tileId < GetNbTile(); ++tileId)
       {
           float tileVisibility = m_users[u].GetTileVisibility(chunkId+k*GetNbProcessedChunk(), tileId);
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    if (qualityLevel == -1)
                    {
                        distortion += m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)]*NOT_DOWNLOADED_DISTORTION*tileVisibility;
                    }
                    else
                    {
                        distortion += m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)]*m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).GetDistortion()*tileVisibility;
                    }
                }
            }
       }
       m_model.add( m_displayedQuality[chunkId] == distortion );
    }
    IloExpr switchingLag(m_env);
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
            {
                switchingLag += (m_users[u].IsAtViewpoint(chunkId+k*GetNbProcessedChunk(), viewpointId) ? 0 : 1)*m_y[GetYId(chunkId, viewpointId, nbSwitch, u, k)];
            }
        }
    }
    //model.add(IloMinimize(env, obj/GetNbChunk()));
    const float alpha = STALL_QoE_IMPACT;
    const float beta = LAG_COST;
    const float test = 10;
    //m_model.add(IloMinimize(m_env, test*IloSum(m_varr)/(GetNbProcessedChunk()-1)+ distortion/GetNbProcessedChunk()+alpha*(IloSum(m_stall)+10*IloSum(m_stallEvent)/alpha)/GetNbProcessedChunk()+beta*switchingLag/GetNbProcessedChunk()));
    //m_model.add(IloMinimize(m_env, IloSum(m_displayedQuality)/GetNbProcessedChunk()+alpha*STALL_COUNT_IN_SECOND*IloSum(m_stallQad)/GetNbProcessedChunk()+beta*switchingLag/GetNbProcessedChunk()));
    //m_model.add(IloMinimize(m_env, test*IloSum(m_varr)/(GetNbProcessedChunk()-1) + IloSum(m_displayedQuality)/GetNbProcessedChunk()+alpha*STALL_COUNT_IN_SECOND*IloSum(m_stall)/GetNbProcessedChunk()+beta*switchingLag/GetNbProcessedChunk()));
    m_model.add(IloMinimize(m_env, IloSum(m_displayedQuality)/GetNbProcessedChunk()+alpha*(STALL_COUNT_IN_SECOND*IloSum(m_stall)+IloSum(m_stallEvent))/GetNbProcessedChunk()+beta*switchingLag/GetNbProcessedChunk()));
}

void Scenario::GetResults(IloCplex& cplex, int k, int u, int b)
{
    m_solInfo[GetSolId(k, u, b)].m_objectiveValue = cplex.getObjValue();
    m_solInfo[GetSolId(k, u, b)].m_solutionGap = cplex.getMIPRelativeGap();
    m_solInfo[GetSolId(k, u, b)].m_solutionStatus = cplex.getStatus() == IloAlgorithm::Optimal ? "Optimal" : (cplex.getStatus() == IloAlgorithm::Infeasible ? "Infeasible" : "Unknown");


    //Get the results
    IloNumArray x_sol(m_env, m_nbProcessedChunk*m_nbViewpoint*m_nbTiles*(1+m_nbQuality));
    cplex.getValues(x_sol, m_x);
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    if (x_sol[GetXId(chunkId, viewpointId, tileId, qualityLevel)] > 0.5)
                    {
                        m_as.GetSeg(chunkId+k*GetNbProcessedChunk(), viewpointId, tileId, qualityLevel).SetSelected();
                    }
                }
            }
        }
    }
    x_sol.end();
    IloNumArray y_sol(m_env, m_nbProcessedChunk*m_nbViewpoint*(m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1));
    cplex.getValues(y_sol, m_y);
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
            {
                if (y_sol[GetYId(chunkId, viewpointId, nbSwitch, u, k)] > 0.5)
                {
                    m_solInfo[GetSolId(k, u, b)].m_displayedViewpoint[chunkId]= viewpointId;
                    break;
                }
            }
        }
    }
    y_sol.end();
    IloNumArray d_sol(m_env, m_nbChunk*(m_nbProcessedChunk+m_nbLagChunk)*m_nbViewpoint*m_nbTiles*m_nbQuality);
    cplex.getValues(d_sol, m_d);
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
        {
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
                {
                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                    {
                        SetDownloadRatio(chunkId, chunkLagId, viewpointId, tileId, qualityLevel, k, d_sol[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)]/float(D_MAX)); 
                    }
                }
            }
        }
    }
    d_sol.end();
    IloNumArray stall_sol(m_env, GetNbProcessedChunk()+GetNbLagChunk());
    cplex.getValues(stall_sol, m_stall);
    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    {
        m_solInfo[GetSolId(k, u, b)].m_nbStall[chunkLagId+GetNbLagChunk()] = stall_sol[chunkLagId+GetNbLagChunk()]*STALL_COUNT_IN_SECOND;
    }
    stall_sol.end();

    IloNumArray ds_sol(m_env, GetNbProcessedChunk()*(GetNbProcessedChunk()+GetNbLagChunk()));
    cplex.getValues(ds_sol, m_ds);
    for (int chunkId = 0; chunkId < GetNbProcessedChunk()+GetNbLagChunk(); ++chunkId)                    
    {                                                                            
        std::cout  << "ChunkLagId " << int(chunkId)-GetNbLagChunk() << " (" << chunkId-GetNbLagChunk()+k*GetNbProcessedChunk() << ")" << ((chunkId-GetNbLagChunk() > 0) ? " displ " +std::to_string(m_solInfo[GetSolId(k, u, b)].m_displayedViewpoint[chunkId-GetNbLagChunk()]): "") << ((chunkId-GetNbLagChunk() > 0 && m_users[u].GetSwitchingDecisionTime(chunkId-1-GetNbLagChunk()+k*GetNbProcessedChunk()) >= 0) ? " sw_req" : "")<<": ";
        std::cout  << "["  << std::max(0, chunkId-GetNbLagChunk()) << "/" << m_users[u].GetNbSwitchUntil(std::max(0, chunkId-GetNbLagChunk()), k, GetNbProcessedChunk()) << "/" << m_users[u].GetNbSwitch(k, GetNbProcessedChunk()) << "] ";
        //std::cout  << "ChunkLagId " << int(chunkId)-GetNbLagChunk() << " (" << chunkId-GetNbLagChunk()+k*GetNbProcessedChunk() << ")" << ((chunkId-GetNbLagChunk() > 0 && m_users[u].GetWantedViewpoint(std::max(0, chunkId-1-GetNbLagChunk())+k*GetNbProcessedChunk()) != m_users[u].GetWantedViewpoint(std::max(0, chunkId-GetNbLagChunk())+k*GetNbProcessedChunk())) ? " sw_req" : "")<<": ";
        for (int c = 0; c < GetNbProcessedChunk(); ++c)
        {
            if (ds_sol[GetDsId(c, chunkId-GetNbLagChunk())] > 0.5)
            {
                std::cout << c << " ";
            }
        }
        std::cout << std::endl;
    }
    ds_sol.end(); 
}

void Scenario::Presolve(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
        {
            IloExpr sum(m_env);
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    if(m_users[u].GetTileVisibility(chunkId+k*GetNbProcessedChunk(), tileId)==0)
                    {
                        m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)].setUB(0);
                        for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                        {
                            if (qualityLevel != -1)
                            {
                                m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)].setUB(0);
                            }
                        }
                        
                    }
                    if (qualityLevel == -1)
                    {
                        m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)].setUB(0);
                    }
                }
            }
        }
    }
}
