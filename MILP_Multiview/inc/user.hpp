/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Class that represents a user viewing measures
 */
#pragma once

#include "common.hpp"

#include <string>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <iostream>
namespace IMT {
class User
{
    public:
        User(void): m_nbViewpoint(0), m_nbTile(0), m_nbChunk(0), m_visibilityRatio(), m_switchingDecisionTime() {}
        User(int nbViewpoint, int nbTile, int nbChunk): 
            m_nbViewpoint(nbViewpoint), m_nbTile(nbTile), m_nbChunk(nbChunk),
            m_visibilityRatio(m_nbViewpoint*m_nbTile*m_nbChunk, INVALIDE_FLOAT_VALUE),
            m_switchingDecisionTime(m_nbChunk, INVALIDE_FLOAT_VALUE) {}
        ~User(void) {}

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(m_nbViewpoint);
            ar & BOOST_SERIALIZATION_NVP(m_nbTile);
            ar & BOOST_SERIALIZATION_NVP(m_nbChunk);
            ar & BOOST_SERIALIZATION_NVP(m_visibilityRatio);
        }

        //Read the text file pathToUserVisibilityFile to fill the user tile visibility information
        void Init(std::string pathToUserVisibilityFile);

        auto const& GetVisibility(int chunkId, int viewpointId, int tileId) const
        {return m_visibilityRatio[ToSegId(chunkId, viewpointId, tileId)];}
        auto GetTileVisibility(int chunkId, int tileId) const
        {
            float v(0);
            for (int viewpointId = 0; viewpointId < m_nbViewpoint; ++viewpointId)
            {
                v += GetVisibility(chunkId, viewpointId, tileId);
            }
            return v;
        }
        ///IsAtViewpoint: Return true is user is at viewpointId during chunkId
        bool IsAtViewpoint(int chunkId, int viewpointId) const
        {
            if (chunkId < m_nbChunk && viewpointId < m_nbViewpoint)
            {
                for (int tileId = 0; tileId < m_nbTile; ++tileId)
                {
                    if (GetVisibility(chunkId, viewpointId, tileId) > 0)
                    {
                        return true;
                    }
                }
            }
            return false;

        }
        int GetWantedViewpoint(int chunkId) const
        {
            for (int viewpointId = 0; viewpointId < m_nbViewpoint; ++viewpointId)
            {
                if (IsAtViewpoint(chunkId, viewpointId))
                {
                    return viewpointId;
                }
            }
            return -1;
        }
        float GetSwitchingDecisionTime(int chunkId) const
        {return m_switchingDecisionTime[chunkId];}
        int GetNbSwitch(int k, int nbProcessedChunk) const
        {
            int count(0);
            for (int chunkId = k*nbProcessedChunk; chunkId < std::min(m_nbChunk, (k+1)*nbProcessedChunk); ++chunkId)
            {
                if (GetSwitchingDecisionTime(chunkId) >= 0)
                {
                    ++count;
                }
            }
            return count;
        }
        int GetNbSwitchUntil(int chunkId, int k, int nbProcessedChunk) const
        {
            int count(0);
            for (int chunkIdP = k*nbProcessedChunk; chunkIdP < std::min(chunkId + k*nbProcessedChunk, (k+1)*nbProcessedChunk); ++chunkIdP)
            {
                if (GetSwitchingDecisionTime(chunkIdP) >= 0)
                {
                    ++count;
                }
            }
            return count;
        }
    private:
        int m_nbViewpoint;
        int m_nbTile;
        int m_nbChunk;
        std::vector<float> m_visibilityRatio;
        std::vector<float> m_switchingDecisionTime;

        int ToSegId(int chunkId, int viewpointId, int tileId) const
        {return chunkId*m_nbTile*m_nbViewpoint+viewpointId*m_nbTile + tileId;}
};
}
