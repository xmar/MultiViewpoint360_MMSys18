/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Class that represents an adaptation set
 */
#pragma once

#include "segment.hpp"

#include <string>
#include <vector>
//#include <memory>
#include <boost/serialization/vector.hpp>

namespace IMT {
class AdaptationSet
{
    public:
        AdaptationSet(int nbViewpoint, int nbTile, int nbQuality, int nbChunk);
        ~AdaptationSet(void) {}

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(m_nbViewpoint);
            ar & BOOST_SERIALIZATION_NVP(m_nbTile);
            ar & BOOST_SERIALIZATION_NVP(m_nbQuality);
            ar & BOOST_SERIALIZATION_NVP(m_nbChunk);
            ar & BOOST_SERIALIZATION_NVP(m_segments);
        }
 

        //Read the text file pathToAdaptationSetConfig file to fill the adaptation set segment information
        void Init(std::string pathToAdaptationSetConfig);
        auto& GetSeg(int chunkId, int viewpointId, int tileId, int qualityLevel)
        //auto const& GetSeg(int chunkId, int viewpointId, int tileId, int qualityLevel) const
           {return m_segments[ToSegId(chunkId, viewpointId, tileId, qualityLevel)];}
        auto const& GetSeg(int chunkId, int viewpointId, int tileId, int qualityLevel) const
           {return m_segments[ToSegId(chunkId, viewpointId, tileId, qualityLevel)];}

        auto const& GetSegs(void) const {return m_segments;}
    private:
        int m_nbViewpoint;
        int m_nbTile;
        int m_nbQuality;
        int m_nbChunk;
        std::vector<Segment> m_segments;
        //std::vector<std::shared_ptr<Segment>> m_segments;
        ////m_orderedSegments[chunk][viewpoint][tile][quality] -> seg
        //std::vector<std::vector<std::vector<std::vector<std::shared_ptr<Segment>>>>> m_orderedSegments;

        int ToSegId(int chunkId, int viewpointId, int tileId, int qualityLevel) const
            {return chunkId*m_nbViewpoint*m_nbTile*m_nbQuality + viewpointId*m_nbTile*m_nbQuality + tileId*m_nbQuality + qualityLevel;}
};
}
