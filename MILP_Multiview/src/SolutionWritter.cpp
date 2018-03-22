#include "SolutionWritter.hpp"

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>

using namespace IMT;

//Add function to allow for range loop on SolutionType()


static std::string solTypeToString(SolutionType st)
{
    switch (st)
    {
        case SolutionType::Optimal:
            return "Opt";
        case SolutionType::Vertical:
            return "Vert";
        case SolutionType::Horizontal:
            return "Hori";
    }
}

void SolutionWritter::Write(void) const
{
    std::cout << "Generate output for bandwidth" << std::endl;
    
    for (auto& gap: m_gap)
    {
    for (auto& nbLagChunk: m_nbLagChunk)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                std::ofstream ofs(m_outputPathId + "_bandwidth_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".txt");
                ofs << "bandwidth ";
                for (auto& solType: m_solType)
                {
                    ofs << solTypeToString(solType) << "_objVal ";
                    ofs << solTypeToString(solType) << "_solGap ";
                    ofs << solTypeToString(solType) << "_distVis ";
                    ofs << solTypeToString(solType) << "_nbStall ";
                    ofs << solTypeToString(solType) << "_nbSwitchLag ";
                    ofs << solTypeToString(solType) << "_nbSkipSwitch ";
                    for (int quality = 0; quality <= nbQuality; ++quality)
                    {
                        ofs << solTypeToString(solType) << "_q"<<quality<<"_download ";
                        ofs << solTypeToString(solType) << "_q"<<quality<<"_displayed ";
                    }
                    ofs << solTypeToString(solType) << "_bandwidthUsage ";
                    ofs << solTypeToString(solType) << "_bandwidthWasted ";
                }
                ofs.seekp(-1, std::ios_base::end);
                ofs << "\n";

                for (auto& bandwidth: m_avBandwidth)
                {
                    ofs << bandwidth << " ";
                    for (auto& solType: m_solType)
                    {
                        float val(-1);
                        float solGap(-1);
                        float distVisi(-1);
                        float nbStall(-1);
                        float nbLag(-1);
                        float nbSkipSwitch(-1);
                        std::vector<float> const* downloadQuality(nullptr);
                        std::vector<float> const* displayedQuality(nullptr);
                        float usage(-1);
                        float wasted(-1);
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    val = sol.m_objectiveSolution;
                                    solGap = sol.m_solutionGap;
                                    distVisi = sol.m_visibleDistortion;
                                    nbStall = sol.m_nbStall;
                                    nbLag = sol.m_nbSwitchingLag;
                                    nbSkipSwitch = sol.m_nbSkipSwitch;
                                    downloadQuality = &sol.m_qualitySelected;
                                    displayedQuality = &sol.m_qualityVisible;
                                    usage = sol.m_networkUsage;
                                    wasted = sol.m_networkWasted;
                                }
                            }
                        }
                        ofs << val  << " ";
                        ofs << solGap  << " ";
                        ofs << distVisi << " ";
                        ofs << nbStall  << " ";
                        ofs << nbLag  << " ";
                        ofs << nbSkipSwitch  << " ";
                        for (int quality = 0; quality <= nbQuality; ++quality)
                        {
                            if (downloadQuality != nullptr)
                            {
                                ofs << downloadQuality->at(quality) << " ";
                                ofs << displayedQuality->at(quality) << " ";
                            }
                            else
                            {
                                ofs << "-1 ";
                            }
                        }
                        ofs << usage << " ";
                        ofs << wasted << " ";
                    }
                    ofs.seekp(-1, std::ios_base::end);
                    ofs << "\n";
                }
            }
        }
    }
    }
    std::cout << "Generate output for lag" << std::endl;
    
    for (auto& gap: m_gap)
    {
    for (auto& bandwidth: m_avBandwidth)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                std::ofstream ofs(m_outputPathId + "_lag_Bandwidth"+std::to_string(bandwidth)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".txt");
                ofs << "lag ";
                for (auto& solType: m_solType)
                {
                    ofs << solTypeToString(solType) << "_objVal ";
                    ofs << solTypeToString(solType) << "_solGap ";
                    ofs << solTypeToString(solType) << "_distVis ";
                    ofs << solTypeToString(solType) << "_nbStall ";
                    ofs << solTypeToString(solType) << "_nbSwitchLag ";
                    ofs << solTypeToString(solType) << "_nbSkipSwitch ";
                    for (int quality = 0; quality <= nbQuality; ++quality)
                    {
                        ofs << solTypeToString(solType) << "_q"<<quality<<"_download ";
                        ofs << solTypeToString(solType) << "_q"<<quality<<"_displayed ";
                    }
                    ofs << solTypeToString(solType) << "_bandwidthUsage ";
                    ofs << solTypeToString(solType) << "_bandwidthWasted ";
                }
                ofs.seekp(-1, std::ios_base::end);
                ofs << "\n";

                for (auto& nbLagChunk: m_nbLagChunk)
                {
                    ofs << nbLagChunk << " ";
                    for (auto& solType: m_solType)
                    {
                        float val(-1);
                        float solGap(-1);
                        float distVisi(-1);
                        float nbStall(-1);
                        float nbLag(-1);
                        float nbSkipSwitch(-1);
                        std::vector<float> const* downloadQuality(nullptr);
                        std::vector<float> const* displayedQuality(nullptr);
                        float usage(-1);
                        float wasted(-1);
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    val = sol.m_objectiveSolution;
                                    solGap = sol.m_solutionGap;
                                    distVisi = sol.m_visibleDistortion;
                                    nbStall = sol.m_nbStall;
                                    nbLag = sol.m_nbSwitchingLag;
                                    nbSkipSwitch = sol.m_nbSkipSwitch;
                                    downloadQuality = &sol.m_qualitySelected;
                                    displayedQuality = &sol.m_qualityVisible;
                                    usage = sol.m_networkUsage;
                                    wasted = sol.m_networkWasted;
                                }
                            }
                        }
                        ofs << val  << " ";
                        ofs << solGap  << " ";
                        ofs << distVisi  << " ";
                        ofs << nbStall  << " ";
                        ofs << nbLag  << " ";
                        ofs << nbSkipSwitch  << " ";
                        for (int quality = 0; quality <= nbQuality; ++quality)
                        {
                            if (downloadQuality != nullptr)
                            {
                                ofs << downloadQuality->at(quality) << " ";
                                ofs << displayedQuality->at(quality) << " ";
                            }
                            else
                            {
                                ofs << "-1 ";
                            }
                        }
                        ofs << usage << " ";
                        ofs << wasted << " ";
                    }
                    ofs.seekp(-1, std::ios_base::end);
                    ofs << "\n";
                }
            }
        }
    }
    }
    std::cout << "Generate switching lag cdf" << std::endl;
    
    for (auto& gap: m_gap)
    {
    for (auto& nbLagChunk: m_nbLagChunk)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                std::ofstream ofs(m_outputPathId + "_switchingLag_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".txt");
                ofs << "cdf ";
                for (auto& solType: m_solType)
                {
                    for (auto& bandwidth: m_avBandwidth)
                    {
                        ofs << solTypeToString(solType) << "_"<<bandwidth << "Mbps ";
                        ofs << solTypeToString(solType) << "_"<<bandwidth << "Mbps_inNbChunk ";
                    }
                }
                ofs.seekp(-1, std::ios_base::end);
                ofs << "\n";

                std::vector<std::vector<float>> lags;
                std::vector<std::vector<float>> lags_inChunkNb;
                //std::vector<std::vector<float>> pi;
                int counter(0);
                for (auto& solType: m_solType)
                {
                    for (auto& bandwidth: m_avBandwidth)
                    {
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    lags.emplace_back(sol.m_switchingLag.size());
                                    lags_inChunkNb.emplace_back(sol.m_lags_inChunkNb.size());
                                    std::copy(sol.m_switchingLag.cbegin(), sol.m_switchingLag.cend(), lags[counter].begin());
                                    std::sort(lags[counter].begin(), lags[counter].end());
                                    std::copy(sol.m_lags_inChunkNb.cbegin(), sol.m_lags_inChunkNb.cend(), lags_inChunkNb[counter].begin());
                                    std::sort(lags_inChunkNb[counter].begin(), lags_inChunkNb[counter].end());
                                    //pi.emplace_back();
                                    //for (int i = 1; i <= lags[counter].size(); ++i)
                                    //{
                                    //    pi[counter].push_back(100.f*(i-0.5)/lags[counter].size());
                                    //}
                                }
                            }
                        }
                        if (lags.size() == counter)
                        {
                            lags.emplace_back();
                            lags_inChunkNb.emplace_back();
                        }
                        ++counter;

                    }
                }
                for(int n = 0; n <= 100; ++n)
                {
                    ofs << n << " ";
                    int counter(0);
                    for (auto& solType: m_solType)
                    {
                        for (auto& bandwidth: m_avBandwidth)
                        {
                            if (lags[counter].size() == 0)
                            {
                                ofs << -1 << " ";
                            }
                            else
                            {
                                float i = lags[counter].size()*n/100.f+0.5;
                                int k = std::floor(i);
                                float f = i-k;
                                float val(0.f);
                                if (n <= 100*0.5/lags[counter].size())
                                {
                                    val = lags[counter][0];
                                }
                                else if(n >= 100*(lags[counter].size()-0.5)/lags[counter].size())
                                {
                                    val = lags[counter][lags[counter].size()-1];
                                }
                                else
                                {
                                    val = lags[counter][k-1]*(1-f)+f*lags[counter][k];
                                }
                                ofs << val << " ";
                            }
                            if (lags_inChunkNb[counter].size() == 0)
                            {
                                ofs << -1 << " ";
                            }
                            else
                            {
                                float i = lags_inChunkNb[counter].size()*n/100.f+0.5;
                                int k = std::floor(i);
                                float f = i-k;
                                float val(0.f);
                                if (n <= 100*0.5/lags_inChunkNb[counter].size())
                                {
                                    val = lags_inChunkNb[counter][0];
                                }
                                else if(n >= 100*(lags_inChunkNb[counter].size()-0.5)/lags_inChunkNb[counter].size())
                                {
                                    val = lags_inChunkNb[counter][lags_inChunkNb[counter].size()-1];
                                }
                                else
                                {
                                    val = lags_inChunkNb[counter][k-1]*(1-f)+f*lags_inChunkNb[counter][k];
                                }
                                ofs << val << " ";
                            }

                            ++counter;
                        }
                    }
                    ofs.seekp(-1, std::ios_base::end);
                    ofs << "\n";
                }
            }
        }
    }
    }

    std::cout << "Generate stalls cdf" << std::endl;
    for (auto& gap: m_gap)
    {
    for (auto& nbLagChunk: m_nbLagChunk)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                std::ofstream ofs(m_outputPathId + "_stall_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".txt");
                std::ofstream ofs2(m_outputPathId + "_stallAggr_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".txt");
                ofs << "cdf ";
                ofs2 << "cdf ";
                for (auto& solType: m_solType)
                {
                    for (auto& bandwidth: m_avBandwidth)
                    {
                        ofs << solTypeToString(solType) << "_"<<bandwidth << "Mbps ";
                    }
                    ofs2 << solTypeToString(solType) << " ";
                }
                ofs.seekp(-1, std::ios_base::end);
                ofs << "\n";
                ofs2.seekp(-1, std::ios_base::end);
                ofs2 << "\n";

                std::vector<std::vector<float>>stalls;
                std::vector<std::vector<float>>aggrStalls;
                //std::vector<std::vector<float>> pi;
                int counter(0);
                int counterSolType(0);
                for (auto& solType: m_solType)
                {
                    aggrStalls.emplace_back();
                    for (auto& bandwidth: m_avBandwidth)
                    {
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    stalls.emplace_back(sol.m_stalls.size());
                                    std::copy(sol.m_stalls.cbegin(), sol.m_stalls.cend(), stalls[counter].begin());
                                    std::sort(stalls[counter].begin(), stalls[counter].end());
                                    if (bandwidth >= 5)
                                    {
                                        for (auto& s: sol.m_stalls)
                                        {
                                            aggrStalls[counterSolType].emplace_back(s);
                                        }
                                    }
                                    //pi.emplace_back();
                                    //for (int i = 1; i <= lags[counter].size(); ++i)
                                    //{
                                    //    pi[counter].push_back(100.f*(i-0.5)/lags[counter].size());
                                    //}
                                }
                            }
                        }
                        if (stalls.size() == counter)
                        {
                            stalls.emplace_back();
                        }
                        ++counter;

                    }
                    std::sort(aggrStalls[counterSolType].begin(), aggrStalls[counterSolType].end());
                    ++counterSolType;
                }
                for(int n = 0; n <= 100; ++n)
                {
                    ofs << n << " ";
                    ofs2 << n << " ";
                    int counter(0);
                    int counterSol(0);
                    for (auto& solType: m_solType)
                    {
                        for (auto& bandwidth: m_avBandwidth)
                        {
                            if (stalls[counter].size() == 0)
                            {
                                ofs << -1 << " ";
                            }
                            else
                            {
                                float i = stalls[counter].size()*n/100.f+0.5;
                                int k = std::floor(i);
                                float f = i-k;
                                float val(0.f);
                                if (n <= 100*0.5/stalls[counter].size())
                                {
                                    val = stalls[counter][0];
                                }
                                else if(n >= 100*(stalls[counter].size()-0.5)/stalls[counter].size())
                                {
                                    val = stalls[counter][stalls[counter].size()-1];
                                }
                                else
                                {
                                    val = stalls[counter][k-1]*(1-f)+f*stalls[counter][k];
                                }
                                ofs << val << " ";
                            }
                            ++counter;
                        }
                        if (aggrStalls[counterSol].size() == 0)
                        {
                            ofs2 << -1 << " ";
                        }
                        else
                        {
                            float i = aggrStalls[counterSol].size()*n/100.f+0.5;
                            int k = std::floor(i);
                            float f = i-k;
                            float val(0.f);
                            if (n <= 100*0.5/aggrStalls[counterSol].size())
                            {
                                val = aggrStalls[counterSol][0];
                            }
                            else if(n >= 100*(aggrStalls[counterSol].size()-0.5)/aggrStalls[counterSol].size())
                            {
                                val = aggrStalls[counterSol][aggrStalls[counterSol].size()-1];
                            }
                            else
                            {
                                val = aggrStalls[counterSol][k-1]*(1-f)+f*aggrStalls[counterSol][k];
                            }
                            ofs2 << val << " ";
                        }
                        ++counterSol;
                    }
                    ofs.seekp(-1, std::ios_base::end);
                    ofs << "\n";
                    ofs2.seekp(-1, std::ios_base::end);
                    ofs2 << "\n";
                }
            }
        }
    }
    }
    std::cout << "Generate median gap for bandwidth" << std::endl;
    
    for (auto& gap: m_gap)
    {
    for (auto& nbLagChunk: m_nbLagChunk)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                std::ofstream ofs(m_outputPathId + "_medianGap_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".txt");
                ofs << "bandwidth ";
                for (auto& solType: m_solType)
                {
                    ofs << solTypeToString(solType) << " ";
                    ofs << solTypeToString(solType) << "_25 ";
                    ofs << solTypeToString(solType) << "_75 ";
                }
                ofs.seekp(-1, std::ios_base::end);
                ofs << "\n";

                for (auto& bandwidth: m_avBandwidth)
                {
                    ofs << bandwidth << " ";
                    std::vector<float> opt;
                    std::vector<float> hori;
                    std::vector<float> vert;
                    for (auto& solType: m_solType)
                    {
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    if (solType == SolutionType::Optimal)
                                    {
                                        opt = std::vector<float>(sol.m_distVisi_psnr.size());
                                        std::copy(sol.m_distVisi_psnr.cbegin(), sol.m_distVisi_psnr.cend(), opt.begin());
                                    }
                                    else if (solType == SolutionType::Horizontal)
                                    {
                                        hori = std::vector<float>(sol.m_distVisi_psnr.size());
                                        std::copy(sol.m_distVisi_psnr.cbegin(), sol.m_distVisi_psnr.cend(), hori.begin());
                                    }
                                    else
                                    {
                                        vert = std::vector<float>(sol.m_distVisi_psnr.size());
                                        std::copy(sol.m_distVisi_psnr.cbegin(), sol.m_distVisi_psnr.cend(), vert.begin());
                                    }
                                }
                            }
                        }
                        
                    }
                    if (hori.size() == vert.size() && hori.size() == opt.size() && hori.size() != 0)
                    {
                        float i = opt.size()*50/100.f+0.5;
                        int k = std::floor(i);
                        float f = i-k;
                        float i25 = opt.size()*25/100.f+0.5;
                        int k25 = std::floor(i25);
                        float f25 = i25-k25;
                        float i75 = opt.size()*75/100.f+0.5;
                        int k75 = std::floor(i75);
                        float f75 = i75-k75;
                        for (int u = 0; u < opt.size(); ++u)
                        {
                            hori[u] -= opt[u];
                            vert[u] -= opt[u];
                        }
                        std::sort(opt.begin(), opt.end());
                        std::sort(hori.begin(), hori.end());
                        std::sort(vert.begin(), vert.end());
                        float o = opt[k-1]*(1-f)+f*opt[k];
                        float h = hori[k-1]*(1-f)+f*hori[k];
                        float v = vert[k-1]*(1-f)+f*vert[k];
                        float o25 = opt[k25-1]*(1-f25)+f25*opt[k25];
                        float h25 = hori[k25-1]*(1-f25)+f25*hori[k25];
                        float v25 = vert[k25-1]*(1-f25)+f25*vert[k25];
                        float o75 = opt[k75-1]*(1-f75)+f75*opt[k75];
                        float h75 = hori[k75-1]*(1-f75)+f75*hori[k75];
                        float v75 = vert[k75-1]*(1-f75)+f75*vert[k75];
                        ofs << o << " " << o25 << " " << o75 << " ";
                        ofs << v << " " << v25 << " " << v75 << " ";
                        ofs << h << " " << h25 << " " << h75 << " ";
                    }
                    else
                    {
                        ofs << -1 << " " << -1 << " ";
                    }
                    ofs.seekp(-1, std::ios_base::end);
                    ofs << "\n";
                }
            }
        }
    }
    }
    

    std::cout << "Generate lag ratio for bandwidth" << std::endl;

    for (auto& gap: m_gap)
    {
    for (auto& nbLagChunk: m_nbLagChunk)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                std::ofstream ofs(m_outputPathId + "_lagRatio_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+".tex");
                
                std::vector<float> noDelay;
                std::vector<float> oneSecond;
                std::vector<float> twoSecond;
                std::vector<float> tenSecond;
                std::vector<float> twentySecond;
                std::vector<float> more;
                for (auto& bandwidth: m_avBandwidth)
                {
                    if (bandwidth < 5)
                    {
                        continue;
                    }
                    int counter(0);
                    for (auto& solType: m_solType)
                    {
                        noDelay.emplace_back(0);
                        oneSecond.emplace_back(0);
                        twoSecond.emplace_back(0);
                        tenSecond.emplace_back(0);
                        twentySecond.emplace_back(0);
                        more.emplace_back(0);
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    for (auto& s: sol.m_lags_inChunkNb)
                                    {
                                        if (s < 0)
                                        {
                                            //mean no switch happen so do nothing
                                        }
                                        else if (s == 0)
                                        {
                                            ++noDelay[counter];
                                        }
                                        else if (s <= 1)
                                        {
                                            ++oneSecond[counter];
                                        }
                                        else if (s <= 2)
                                        {
                                            ++twoSecond[counter];
                                        }
                                        else if (s <= 10)
                                        {
                                            ++tenSecond[counter];
                                        }
                                        else if (s <= 20)
                                        {
                                            ++twentySecond[counter];
                                        }
                                        else
                                        {
                                            ++more[counter];
                                        }
                                    }
                                }
                            }
                        }
                        ++counter;
                    }
                }
                int counter(0);
                if (noDelay.size() > 0)
                {
                    for (auto& solType: m_solType)
                    {
                        int sum = noDelay[counter] + oneSecond[counter] + twoSecond[counter] + tenSecond[counter] + twentySecond[counter] + more[counter];
                        if (sum != 0)
                        {
                            noDelay[counter] /= sum;
                            oneSecond[counter] /= sum;
                            twoSecond[counter] /= sum;
                            tenSecond[counter] /= sum;
                            twentySecond[counter] /= sum;
                            more[counter] /= sum;
                        }
                        std::vector<std::string> stackName({"NoDelay", "OneS", "TwoS", "TenS", "TwentyS", "More"});
                        int c(0);
                        for (auto& sName: stackName)
                        {
                            ofs << "\\def\\" << solTypeToString(solType) << sName << "{" ;
                            switch (c) {
                            case 0:
                            ofs << noDelay[counter] << "}\n"; break;
                            case 1:
                            ofs << oneSecond[counter] << "}\n"; break;
                            case 2:
                            ofs << twoSecond[counter] << "}\n"; break;
                            case 3:
                            ofs << tenSecond[counter] << "}\n"; break;
                            case 4:
                            ofs << twentySecond[counter] << "}\n"; break;
                            case 5:
                            ofs << more[counter] << "}\n"; break;
                            }
                            ++c;
                        }
                        ++counter;
                    }
                }
            }
        }
    }
    }

    std::cout << "Generate selected segments" << std::endl;
    for (auto& gap: m_gap)
    {
    for (auto& nbLagChunk: m_nbLagChunk)
    {
        for (auto& nbQuality: m_nbQuality)
        {
            for (auto& nbTile: m_nbTile)
            {
                for (auto& bandwidth: m_avBandwidth)
                {
                    for (auto& solType: m_solType)
                    {
                        std::ofstream ofs(m_outputPathId + "_selectedSegment_Lag"+std::to_string(nbLagChunk)+"_qual"+std::to_string(nbQuality)+"_tile"+std::to_string(nbTile)+"_gap"+std::to_string(gap)+"_"+solTypeToString(solType)+"_"+std::to_string(bandwidth)+"Mbps"+".txt");

                        std::vector<std::vector<int>> const* displayedViewpoint;                          
                        std::vector<std::vector<std::vector<int>>> const* selectedTileQuality;
                        int nbUser = -1;
                        int nbBandwidth = -1;
                        for (auto& solTuple: m_sol)
                        {
                            Solution const& sol = std::get<1>(solTuple);
                            MetaData const& meta = std::get<0>(solTuple);
                            if (meta.m_avBandwidth == bandwidth && meta.m_nbTile == nbTile && meta.m_nbQuality == nbQuality && meta.m_nbLagChunk == nbLagChunk&& meta.m_gap == gap)
                            {
                                if (meta.m_solType == solType)
                                {
                                    displayedViewpoint = &sol.m_displayedViewpoint;
                                    selectedTileQuality = &sol.m_selectedTileQuality;
                                    nbUser = sol.m_nbUser;
                                    nbBandwidth = sol.m_nbBandwidth;
                                }
                            }
                        }
                        if (nbUser > 0)
                        {
                            ofs << nbUser << " " << nbBandwidth << " " << nbTile << "\n";
                            ofs << "chunkId ";
                            for (int u = 0; u < int(nbUser); ++u)
                            {
                                for (int b = 0; b < int(nbBandwidth); ++b)
                                {
                                    ofs << u << "_" << b << "_viewpoint ";
                                    for (int t = 0; t < int(nbTile); ++t)
                                    {
                                        ofs << u << "_" << b << "_" << t << " ";
                                    }
                                }
                            }
                            ofs.seekp(-1, std::ios_base::end);
                            ofs << "\n";
                            for (int chunkId = 0;  chunkId < (*displayedViewpoint)[0].size(); ++chunkId)
                            {
                                ofs << chunkId << " ";
                                for (int u = 0; u < nbUser; ++u)
                                {
                                    for (int b = 0; b < nbBandwidth; ++b)
                                    {
                                        ofs << (*displayedViewpoint)[u*nbBandwidth+b][chunkId] << " ";
                                        for (int t = 0; t < nbTile; ++t)
                                        {
                                            ofs << (*selectedTileQuality)[u*nbBandwidth+b][chunkId][t] << " ";
                                        }
                                    }
                                }
                                ofs.seekp(-1, std::ios_base::end);                   
                                ofs << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
    }

}
