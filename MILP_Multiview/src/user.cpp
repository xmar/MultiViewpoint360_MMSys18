#include "user.hpp"

#include <fstream>                                                               
#include <sstream>                                                               
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

using namespace IMT;

void User::Init(std::string pathToUserVisibilityFile)
{
    std::ifstream ifs(pathToUserVisibilityFile);
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
            float visibilityRatio(0);
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
                    visibilityRatio  = std::stof(val);
                }
                else if (counter == 4)
                {
                    m_switchingDecisionTime[chunkId] = std::stof(val);
                }
                else
                {
                    throw std::string("Wrong configuration file for user visibility");
                }
                ++counter;
            }
            if (counter <= 3)
            {
                throw std::string("Wrong configuration file for user visibility");
            }
            m_visibilityRatio[ToSegId(chunkId, viewpointId, tileId)] = visibilityRatio;
        }
    }
    //Sanity check
    for (int chunkId = 0; chunkId < m_nbChunk; ++chunkId)
    {
        float total(0);
        for (int viewpointId = 0; viewpointId < m_nbViewpoint; ++viewpointId)
        {
            float viewpointTotal(0);
            for (int tileId = 0; tileId < m_nbTile; ++tileId)
            {
                if (m_visibilityRatio[ToSegId(chunkId, viewpointId, tileId)] == INVALIDE_FLOAT_VALUE ||
                        m_visibilityRatio[ToSegId(chunkId, viewpointId, tileId)] < 0)
                {
                    throw std::string("Wrong configuration file for user "
                            "visibility "+ pathToUserVisibilityFile +", missing chunkId"
                            +std::to_string(chunkId)+" viewpointId"
                            + std::to_string(viewpointId) +" tileId " +
                            std::to_string(tileId)); }
                viewpointTotal += m_visibilityRatio[ToSegId(chunkId, viewpointId, tileId)];
            }
            if (std::fabs(viewpointTotal) <= std::numeric_limits<float>::epsilon() && std::fabs(viewpointTotal - 1.f)<= std::numeric_limits<float>::epsilon())
            {
                throw std::string("Wrong configuration file for user "
                        "visibility, chunkId " + std::to_string(chunkId) + 
                        " viewpointId " + std::to_string(viewpointId) + " have " +
                        "partial visibility: "+std::to_string(viewpointTotal));
            }
            total += viewpointTotal;
        }
        if ( std::fabs(total-1) > 3*std::numeric_limits<float>::epsilon() )
        {
            std::cout << total << std::endl;
            throw std::string("Wrong configuration file for user visibility, "
                    "chunkId "+std::to_string(chunkId)+" user has to watch one " +
                    "and only one viewpoint");
        }
    }
}
