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
class OptimalHorizontalScenario : public Scenario
{
    public:
        OptimalHorizontalScenario(int nbUser, int nbBandwidth, int nbChunk, int nbProcessedChunk, int nbLagChunk, int nbViewpoint, int nbTiles, int nbQuality, float optimalGap, int nbThread):
            Scenario(nbUser, nbBandwidth, nbChunk, nbProcessedChunk, nbLagChunk, nbViewpoint, nbTiles, nbQuality, optimalGap, nbThread) {}
        ~OptimalHorizontalScenario(void) {}

        //void Run(void) override;
    protected:
        void SetSelectAndDownloadedConstraint(int k, int u, int b) override;
        void Presolve(int k, int u, int b) override;
    private:
};
}
