/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Class that represents a segment from the adaptation set
 */
#pragma once

#include "common.hpp"
#include <tuple>
#include <boost/serialization/vector.hpp>

namespace IMT {
class Segment
{
    public:
        Segment(void): m_chunkId(0), m_viewpointId(0), m_tileId(0), m_qualityLevel(0), m_distortion(-1), m_bitrate(-1), m_selected(false) {}
        Segment(int chunkId, int viewpointId, int tileId, int qualityLevel,
                float distortion, float bitrate): m_chunkId(chunkId),
            m_viewpointId(viewpointId), m_tileId(tileId),
            m_qualityLevel(qualityLevel), m_distortion(distortion),
            m_bitrate(bitrate), m_selected(false) {}
        ~Segment(void){}

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(m_chunkId);
            ar & BOOST_SERIALIZATION_NVP(m_viewpointId);
            ar & BOOST_SERIALIZATION_NVP(m_tileId);
            ar & BOOST_SERIALIZATION_NVP(m_qualityLevel);
            ar & BOOST_SERIALIZATION_NVP(m_distortion);
            ar & BOOST_SERIALIZATION_NVP(m_bitrate);
            ar & BOOST_SERIALIZATION_NVP(m_selected);
        }

        auto SegmentId(void) const {return std::make_tuple(m_chunkId, m_viewpointId, m_tileId, m_qualityLevel);}
        auto const& GetDistortion(void) const {return m_distortion;}
        auto const& GetBitrate(void) const {return m_bitrate;}
        void SetSelected(void) {m_selected = true;}
        auto const& IsSelected(void) const {return m_selected;}
    private:
        int m_chunkId;
        int m_viewpointId;
        int m_tileId;
        int m_qualityLevel;
        float m_distortion;
        float m_bitrate;
        bool m_selected;
};
}
