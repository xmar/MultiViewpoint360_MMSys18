/**
 * IMT Atlantique                                      
 * Author: Xavier CORBILLON                                                      
 *                                                                               
 * Virtual class that represents the scenario interface
 */
#pragma once

#include "adaptationSet.hpp"
#include "user.hpp"
#include "bandwidth.hpp"
#include "SolutionWritter.hpp"
#include "serializationTuple.hpp"

#include <string>
#include <vector>
#include <map>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include <ilcplex/ilocplex.h>

namespace IMT {
class Scenario
{
    public:
	struct SolutionInfo
	{
        SolutionInfo(void):
            m_objectiveValue(INVALIDE_FLOAT_VALUE), m_solutionGap(INVALIDE_FLOAT_VALUE),
            m_solutionStatus("Not Solved Yet"), m_nbStall(), m_displayedViewpoint(), m_downloadRatio() {};
        SolutionInfo(int nbProcessedChunk, int nbLagChunk, int nbViewpoint, int nbTiles, int nbQuality):
            m_objectiveValue(INVALIDE_FLOAT_VALUE), m_solutionGap(INVALIDE_FLOAT_VALUE),
            m_solutionStatus("Not Solved Yet"), m_nbStall(nbProcessedChunk+nbLagChunk,0),
            m_displayedViewpoint(nbProcessedChunk, nbViewpoint+1),
            m_downloadRatio(nbProcessedChunk*(nbProcessedChunk+nbLagChunk)*nbViewpoint*nbTiles*nbQuality+(nbProcessedChunk+nbLagChunk)*nbViewpoint*nbTiles*nbQuality+ nbViewpoint*nbTiles*nbQuality + nbTiles*nbQuality + nbQuality, INVALIDE_FLOAT_VALUE) {}
        float m_objectiveValue;
        float m_solutionGap;
        std::string m_solutionStatus;
        std::vector<float> m_nbStall;
        std::vector<int> m_displayedViewpoint;
        std::vector<float> m_downloadRatio;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(m_objectiveValue);
            ar & BOOST_SERIALIZATION_NVP(m_solutionGap);
            ar & BOOST_SERIALIZATION_NVP(m_solutionStatus);
            ar & BOOST_SERIALIZATION_NVP(m_nbStall);
            ar & BOOST_SERIALIZATION_NVP(m_displayedViewpoint);
            ar & BOOST_SERIALIZATION_NVP(m_downloadRatio);
        } 
	};
        Scenario(int nbUsers, int nbBandwidth, int nbChunk, int nbProcessedChunk, int nbLagChunk, int nbViewpoint, int nbTiles, int nbQuality, float optimalGap, int nbThread):
            m_as(nbViewpoint, nbTiles, nbQuality, nbChunk),
            m_users(nbUsers, User(nbViewpoint, nbTiles, nbChunk)), m_bandwidths(nbBandwidth, Bandwidth(nbChunk, nbLagChunk)),
            m_optimalGap(optimalGap), m_nbThread(nbThread), 
            m_solInfo(),
            m_nbChunk(nbChunk), m_nbProcessedChunk(nbProcessedChunk), m_nbLagChunk(nbLagChunk), m_nbViewpoint(nbViewpoint), m_nbTiles(nbTiles),
            m_nbQuality(nbQuality) 
            //m_downloadRatio()
            {
                for (int chunkOffset = 0; chunkOffset < nbChunk; chunkOffset += nbProcessedChunk)
                {
                    for (int u = 0; u < m_users.size(); ++u)
                    {
                        for(int b = 0; b < m_bandwidths.size(); ++b)
                        {
                            m_solInfo.emplace_back(m_nbProcessedChunk, m_nbLagChunk, m_nbViewpoint, m_nbTiles, m_nbQuality);
                        }
                    }
                }
            }
        virtual ~Scenario(void) {}

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            ar & BOOST_SERIALIZATION_NVP(m_users);
            ar & BOOST_SERIALIZATION_NVP(m_as);
            ar & BOOST_SERIALIZATION_NVP(m_bandwidths);
            ar & BOOST_SERIALIZATION_NVP(m_optimalGap);
            ar & BOOST_SERIALIZATION_NVP(m_solInfo);
            ar & BOOST_SERIALIZATION_NVP(m_nbChunk);
            ar & BOOST_SERIALIZATION_NVP(m_nbLagChunk);
            ar & BOOST_SERIALIZATION_NVP(m_nbViewpoint);
            ar & BOOST_SERIALIZATION_NVP(m_nbTiles);
            ar & BOOST_SERIALIZATION_NVP(m_nbQuality);
        }

        void Init(const std::string& pathToAdaptationSetConf, const std::vector<std::string>& pathToUserVisibilityConf, const std::vector<std::string>& pathToBandwidthConf)
        {
            m_as.Init(pathToAdaptationSetConf);
            for (int u = 0; u < m_users.size(); ++u)
            {
                m_users[u].Init(pathToUserVisibilityConf[u]);
            }
            for (int b = 0; b < m_bandwidths.size(); ++b)
            {
                m_bandwidths[b].Init(pathToBandwidthConf[b]);
            }
        }

        void Run(std::string outputScenarioId, std::string printableScenarioName, double scenariosAverageDuration, int scenarioId, int nbScenarios);
        Solution PrintResults(void) const;

        int GetNbChunk(void) const {return m_nbChunk;}
        int GetNbProcessedChunk(void) const {return m_nbProcessedChunk;}
        int GetNbLagChunk(void) const {return m_nbLagChunk;}
        int GetNbViewpoint(void) const {return m_nbViewpoint;}
        int GetNbTile(void) const {return m_nbTiles;}
        int GetNbQuality(void) const {return m_nbQuality;}
        float GetTotalDownloadRatio(int chunkId, int viewpointId, int tilesId, int qualityId, int k) const
        {
            float total(0);
            for (int lagChunkId = -m_nbLagChunk; lagChunkId < int(m_nbChunk); ++lagChunkId)
            {
                total += m_solInfo[k].m_downloadRatio.at(ToDownloadRatioVecId(chunkId, lagChunkId, viewpointId, tilesId, qualityId));
            }
            return total;
        }
    protected:
        AdaptationSet m_as;
        std::vector<User> m_users;
        std::vector<Bandwidth> m_bandwidths;
        float m_optimalGap;
        int m_nbThread;
        std::vector<SolutionInfo> m_solInfo; 
        //variables related to the Cplex model
        IloEnv m_env;
        IloModel m_model;
        IloIntVarArray m_x; //selected representation
        IloIntVarArray m_y; //selected viewpoint
        IloNumVarArray m_d; //downloaded segments
        IloIntVarArray m_ds; //downloaded chunk
        IloIntVarArray m_stall; //stall duration
        IloIntVarArray m_stallEvent; //assess a stall event
        IloNumVarArray m_varr; //qualityVariance
        IloNumVarArray m_displayedQuality; //qualityVariance
        IloNumVarArray m_stallQad; //qualityVariance


        int GetSolId(int ProcessId, int userId, int bandwidthId) const {return ProcessId*m_users.size()*m_bandwidths.size() + userId*m_bandwidths.size() + bandwidthId;}
        int GetXId(int chunkId, int viewpointId, int tilesId, int qualityId) const
        {return chunkId*m_nbViewpoint*m_nbTiles*(1+m_nbQuality)+viewpointId*m_nbTiles*(1+m_nbQuality)+tilesId*(1+m_nbQuality)+qualityId+1;}
        int GetYId(int chunkId, int viewpointId, int switchId, int u, int k) const {return chunkId*m_nbViewpoint*(m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1)+viewpointId*(m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1)+switchId;}
        int GetDId(int chunkId, int lagChunkId, int viewpointId, int tilesId, int qualityId) const 
        {return ToDownloadRatioVecId(chunkId, lagChunkId, viewpointId, tilesId, qualityId);}
        int GetDsId(int chunkId, int lagChunkId) const 
        {return chunkId*(m_nbProcessedChunk+m_nbLagChunk)+ lagChunkId+m_nbLagChunk;}
        void SetDownloadRatio(int chunkId, int lagChunkId, int viewpointId, int tilesId, int qualityId, int k, float downloadRatio)
        {m_solInfo[k].m_downloadRatio[ToDownloadRatioVecId(chunkId, lagChunkId, viewpointId, tilesId, qualityId)] = downloadRatio;}
        void SetBandwidthConstraint(int k, int u, int b);
        void SetTransmissionDelayConstraint(int k, int u, int b);
        void SetDownloadOnceConstraint(int k, int u, int b);
        void SetDownloadDeadlineConstraint(int k, int u, int b);
        virtual void SetSelectAndDownloadedConstraint(int k, int u, int b);
        void SetSelectOneIfVisibleConstraint(int k, int u, int b);
        void SetDisplayOneAndOnlyOneViewpointContraint(int k, int u, int b);
        void SetSelectIfDisplayedViewpointConstraint(int k, int u, int b);
        void SetViewpointSwitchingConstraint(int k, int u, int b);
        void SetDownloadedChunkIndicator(int k, int u, int b);
        void SetInOrderSelection(int k, int u, int b);
        virtual void Presolve(int k, int u, int b);
        void SetObjective(int k, int u, int b);
        void GetResults(IloCplex& cplex, int k, int u, int b);
    //private:
        int m_nbChunk;
        int m_nbProcessedChunk;
        int m_nbLagChunk;
        int m_nbViewpoint;
        int m_nbTiles;
        int m_nbQuality;
        //std::map<std::tuple<int, int, int, int ,int>,int> m_downloadRatio;

        int ToDownloadRatioVecId(int chunkId, int lagChunkId, int viewpointId, int tilesId, int qualityId) const
        {return chunkId*(m_nbProcessedChunk+m_nbLagChunk)*m_nbViewpoint*m_nbTiles*m_nbQuality+(lagChunkId+m_nbLagChunk)*m_nbViewpoint*m_nbTiles*m_nbQuality+
            viewpointId*m_nbTiles*m_nbQuality + tilesId*m_nbQuality + qualityId;}

        void InitCplexVariables(int k, int u, int b);
        void ClearCplexVariables(IloCplex& cplex, int k, int u, int b);
        virtual void ClearCplexVariablesImpl(int k, int u, int b) {};
        virtual void RunImpl(int k, int u, int b) {};
        int GetNbK(void) const { return std::min(7, GetNbChunk()/GetNbProcessedChunk());};
};
}
