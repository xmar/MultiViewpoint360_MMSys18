/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Class that store the bandwidth of the different chunks
 */
#pragma once

#include "common.hpp"

#include <vector>
#include <string>
#include <boost/serialization/vector.hpp>

namespace IMT {
class Bandwidth
{
    public:
        Bandwidth(void): m_nbVideoChunk(0), m_nbLagChunk(0), m_chunkBandwidth() {}
        //for video chunk start with id 0, first download chunk start at id -nbLagChunk;
        Bandwidth(int nbVideoChunk, int nbLagChunk): m_nbVideoChunk(nbVideoChunk),
            m_nbLagChunk(nbLagChunk), m_chunkBandwidth(nbVideoChunk+nbLagChunk) {}
        ~Bandwidth(void) = default;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(m_nbVideoChunk);
            ar & BOOST_SERIALIZATION_NVP(m_nbLagChunk);
            ar & BOOST_SERIALIZATION_NVP(m_chunkBandwidth);
        }

        void Init(std::string pathToBandwidthFile);

        auto const& GetBandwidth(int chunkId) const {return m_chunkBandwidth[ChunkIdToVectId(chunkId)];}
    private:
        int m_nbVideoChunk;
        int m_nbLagChunk;
        std::vector<float> m_chunkBandwidth;

        int ChunkIdToVectId(int chunkId) const {return chunkId + m_nbLagChunk;}
        int VectIdToChunkId(int vectId) const {return vectId - m_nbLagChunk;}
};
}
