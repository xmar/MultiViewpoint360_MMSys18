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
class OptimalVerticalScenario : public Scenario
{
    public:
        OptimalVerticalScenario(int nbUser, int nbBandwidth, int nbChunk, int nbProcessedChunk, int nbLagChunk, int nbViewpoint, int nbTiles, int nbQuality, float optimalGap, int nbThread):
            Scenario(nbUser, nbBandwidth, nbChunk, nbProcessedChunk, nbLagChunk, nbViewpoint, nbTiles, nbQuality, optimalGap, nbThread) {}
        ~OptimalVerticalScenario(void) {}

        //void Run(void) override;
    protected:
        void ClearCplexVariablesImpl(int k, int u, int b) override;
        void RunImpl(int k, int u, int b) override;
    private:
};
}
