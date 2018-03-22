#include "adaptationSet.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

using namespace IMT;

AdaptationSet::AdaptationSet(int nbViewpoint, int nbTile, int nbQuality, int nbChunk):
            m_nbViewpoint(nbViewpoint), m_nbTile(nbTile), m_nbQuality(nbQuality),
            m_nbChunk(nbChunk), m_segments(m_nbChunk*m_nbQuality*m_nbTile*m_nbViewpoint)//,
            //m_orderedSegments(m_nbChunk,
            //        std::vector<std::vector<std::vector<std::shared_ptr<Segment>>>>(m_nbViewpoint,
            //            std::vector<std::vector<std::shared_ptr<Segment>>>(m_nbTile, std::vector<std::shared_ptr<Segment>>(m_nbQuality, nullptr) ) ) )
{}


void AdaptationSet::Init(std::string pathToAdaptationSetConfig)
{
    /**
     * Parse the text configuration file of the adaptation set
     */
    std::ifstream ifs(pathToAdaptationSetConfig);
    std::string line;
    std::string val;
    while (std::getline(ifs, line))
    {
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
        if (line[0] != '#')
        {
            int counter = 0;
            int chunkId(0);
            int viewpointId(0);
            int tileId(0);
            int qualityId(0);
            float distortion(0);
            float bitrate(0);
            std::istringstream ss(line);
            while (std::getline(ss, val, ','))
            {
                if (counter == 0)
                {
                    chunkId = std::stoi(val);
                }
                else if (counter == 1)
                {
                    viewpointId  = std::stoi(val);
                }
                else if (counter == 2)
                {
                    tileId  = std::stoi(val);
                }
                else if (counter == 3)
                {
                    qualityId  = std::stoi(val);
                }   
                else if (counter == 4)
                {
                    //distortion  = std::sqrt(std::stof(val));
                    distortion  = std::stof(val);
                }   
                else if (counter == 5)
                {
                    bitrate  = std::stof(val);
                }   
                else
                {
                    throw std::string("Wrong configuration file for adaptation set");
                }
                ++counter;
            }
            if (counter <= 5)
            {
                throw std::string("Wrong configuration file for adaptation set");
            }
            //m_orderedSegments[chunkId][viewpointId][tileId][qualityId] =
            //    std::make_shared<Segment>(chunkId, viewpointId, tileId, qualityId, distortion, bitrate);
            m_segments[ToSegId(chunkId, viewpointId, tileId, qualityId)] = 
            //    m_orderedSegments[chunkId][viewpointId][tileId][qualityId];
            //    std::make_shared<Segment>(chunkId, viewpointId, tileId, qualityId, distortion, bitrate);
                Segment(chunkId, viewpointId, tileId, qualityId, distortion, bitrate);
        }
    }
    for (int chunkId = 0; chunkId < m_nbChunk; ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < m_nbViewpoint; ++viewpointId)
        {
            for (int tileId = 0; tileId < m_nbTile; ++tileId)
            {
                for (int qualityId = 0; qualityId < m_nbQuality; ++qualityId)
                {
                    if (m_segments[ToSegId(chunkId, viewpointId, tileId, qualityId)].GetBitrate() < 0)
                    {
                        std::string("Wrong configuration file for adaptation set " + pathToAdaptationSetConfig +
                                ", missing chunkId "+ std::to_string(chunkId) +
                                " viewpointId " + std::to_string(viewpointId) +
                                " tileId "+ std::to_string(tileId) + " qualityId " + std::to_string(qualityId));
                    }
                }
            }
        }
    } 
}
