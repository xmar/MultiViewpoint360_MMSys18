/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Class to manage an optimal selection scenario
 */
#pragma once

#include "scenario.hpp"

#include <string>

namespace IMT {
class OptimalScenario : public Scenario
{
    public:
        OptimalScenario(int nbUser, int nbBandwidth, int nbChunk, int nbProcessedChunk, int nbLagChunk, int nbViewpoint, int nbTiles, int nbQuality, float optimalGap, int nbThread):
            Scenario(nbUser, nbBandwidth, nbChunk, nbProcessedChunk, nbLagChunk, nbViewpoint, nbTiles, nbQuality, optimalGap, nbThread) {}
        ~OptimalScenario(void) {}

        //void Run(void) override;
    private:
};
}
