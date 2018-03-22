/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Class to write the solutions to output files
 */
#pragma once

#include "common.hpp"

#include <vector>
#include <set>
#include <string>
#include <tuple>

namespace IMT {
enum class SolutionType {Optimal, Vertical, Horizontal};
static const std::vector<SolutionType> v_SolutionType( {SolutionType::Optimal, SolutionType::Horizontal, SolutionType::Vertical} );

struct MetaData
{
    MetaData(void): m_solType(SolutionType::Optimal), m_nbTile(0), m_nbViewpoint(0), m_nbChunk(0), m_nbLagChunk(0),
        m_nbQuality(0), m_gap(0), m_avBandwidth(0){}
    MetaData(SolutionType solType, int nbTile, int nbViewpoint, int nbChunk,
            int nbLagChunk, int nbQuality, float optGap, float avBandwidth):
        m_solType(solType), m_nbTile(nbTile), m_nbViewpoint(nbViewpoint), m_nbChunk(nbChunk), m_nbLagChunk(nbLagChunk),
        m_nbQuality(nbQuality), m_gap(optGap),
        m_avBandwidth(avBandwidth){}
    SolutionType m_solType;
    int m_nbTile;
    int m_nbViewpoint;
    int m_nbChunk;
    int m_nbLagChunk;
    int m_nbQuality;
    float m_gap;
    float m_avBandwidth;
};
struct Solution
{
    Solution(void): m_objectiveSolution(-1), m_solutionGap(-1), m_visibleDistortion(-1), m_nbStall(0), m_nbSwitchingLag(0), m_switchingLag(), m_nbSkipSwitch(0), m_networkUsage(0), m_networkWasted(0), m_qualitySelected(), m_qualityVisible() {}
    Solution(int nbUser, int nbBandwidth, float objectiveSolution, float solutionGap, float visibleDistortion, std::vector<float> distVisi_psnr, float nbStall, std::vector<float> stalls, float nbSwitchingLag, std::vector<float> switchingLag, std::vector<float> lags_inChunkNb, float nbSkipSwitch, float networkUsage, float networkWasted, std::vector<float> qualitySelected, std::vector<float> qualityVisible, std::vector<std::vector<int>> displayedViewpoint, std::vector<std::vector<std::vector<int>>> selectedTileQuality):
        m_objectiveSolution(objectiveSolution), m_solutionGap(solutionGap), m_visibleDistortion(visibleDistortion), m_distVisi_psnr(std::move(distVisi_psnr)),
        m_nbStall(nbStall), m_stalls(std::move(stalls)), m_nbSwitchingLag(nbSwitchingLag), m_switchingLag(std::move(switchingLag)), m_lags_inChunkNb(std::move(lags_inChunkNb)), m_nbSkipSwitch(nbSkipSwitch), m_networkUsage(networkUsage), m_networkWasted(networkWasted), m_qualitySelected(std::move(qualitySelected)),
        m_qualityVisible(std::move(qualityVisible)), m_nbUser(nbUser), m_nbBandwidth(nbBandwidth), m_displayedViewpoint(displayedViewpoint), m_selectedTileQuality(selectedTileQuality) {}
    Solution(Solution&&)=default;
    ~Solution(void) = default;
    float m_objectiveSolution;
    float m_solutionGap;
    float m_visibleDistortion;
    std::vector<float> m_distVisi_psnr;
    float m_nbStall;
    std::vector<float> m_stalls;
    float m_nbSwitchingLag;
    std::vector<float> m_switchingLag;
    std::vector<float> m_lags_inChunkNb;
    float m_nbSkipSwitch;
    float m_networkUsage;
    float m_networkWasted;
    std::vector<float> m_qualitySelected;
    std::vector<float> m_qualityVisible;
    int m_nbUser;
    int m_nbBandwidth;
    std::vector<std::vector<int>> m_displayedViewpoint;
    std::vector<std::vector<std::vector<int>>> m_selectedTileQuality;
};

class SolutionWritter
{
    public:
        SolutionWritter(std::string outputPathId): m_outputPathId(outputPathId), m_sol(), m_solType(), m_nbTile(), m_nbLagChunk(),
           m_nbQuality(), m_avBandwidth(), m_gap() {}
        ~SolutionWritter(void) = default;

        void AddSolution(MetaData meta, Solution sol)
        {
            m_solType.insert(meta.m_solType);
            m_nbTile.insert(meta.m_nbTile);
            m_nbLagChunk.insert(meta.m_nbLagChunk);
            m_nbQuality.insert(meta.m_nbQuality);
            m_avBandwidth.insert(meta.m_avBandwidth);
            m_gap.insert(meta.m_gap);
            m_sol.emplace_back(std::make_tuple(std::move(meta),std::move(sol)));
        }

        void Write(void) const;
    private:
        std::string m_outputPathId;
        std::vector<std::tuple<MetaData, Solution>> m_sol;
        std::set<SolutionType> m_solType;
        std::set<int> m_nbTile;
        std::set<int> m_nbLagChunk;
        std::set<int> m_nbQuality;
        std::set<float> m_avBandwidth;
        std::set<float> m_gap;
};
}
