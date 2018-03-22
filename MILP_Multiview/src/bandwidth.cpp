#include "bandwidth.hpp"

#include <fstream>                                                               
#include <sstream>                                                               
#include <algorithm>

using namespace IMT;

void Bandwidth::Init(std::string pathToBandwidthFile)
{
    std::ifstream ifs(pathToBandwidthFile);
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
            float bandwidth(0);
            std::istringstream ss(line);
            while (std::getline(ss, val, ','))
            {
                if (counter == 0)
                {
                    chunkId = std::stoi(val);
                }
                else if (counter == 1)
                {
                    bandwidth  = std::stof(val);
                }
                else
                {
                    throw std::string("Wrong configuration file for bandwidth");
                }
                ++counter;
            }
            if (counter <= 1)
            {
                throw std::string("Wrong configuration file for bandwidth");
            }
            m_chunkBandwidth[ChunkIdToVectId(chunkId)] = bandwidth;
        }
    }
    for (int chunkId = -m_nbLagChunk; chunkId < m_nbVideoChunk; ++chunkId)
    {
        if (m_chunkBandwidth[ChunkIdToVectId(chunkId)] == INVALIDE_FLOAT_VALUE || m_chunkBandwidth[ChunkIdToVectId(chunkId)] <= 0)
        {
            throw std::string("Wrong configuration file for bandwidth: missing chunk "+std::to_string(chunkId));
        }
    }
}
