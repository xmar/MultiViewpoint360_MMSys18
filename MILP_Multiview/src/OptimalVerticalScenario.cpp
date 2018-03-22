#include "OptimalVerticalScenario.hpp"

#include <ilcplex/ilocplex.h>
#include <cmath>

#include <boost/filesystem.hpp>                                                  
                                                                                 
using namespace IMT;

void OptimalVerticalScenario::ClearCplexVariablesImpl(int k, int u, int b)
{
}

void OptimalVerticalScenario::RunImpl(int k, int u, int b)
{
    //contraints 1f: can only download current viewpoint
    for (int chunklag = -GetNbLagChunk(); chunklag < GetNbProcessedChunk()-1; ++chunklag)
    {
        for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
        {
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
                {
                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                    {
                        //if (!m_users[u].IsAtViewpoint(std::max(k*GetNbProcessedChunk(), chunklag+1), viewpointId))
                        //{
                        //    m_d[GetDId(chunkId, chunklag, viewpointId, tileId, qualityLevel)].setUB(0);
                        //}
                        m_model.add(m_d[GetDId(chunkId, chunklag, viewpointId, tileId, qualityLevel)] <= m_y[GetYId(std::max(0, chunklag),viewpointId, m_users[u].GetNbSwitchUntil(std::max(0, chunklag), k, GetNbProcessedChunk()), u, k)] + m_y[GetYId(std::max(0, chunklag+1), viewpointId, m_users[u].GetNbSwitchUntil(std::max(0, chunklag+1), k, GetNbProcessedChunk()), u, k)]);
                    }
                }
            }
        }
    }

    //contraints on switching timestamp
    //float switchingDecisionTime = 0.5;
    //for (int chunklag = -GetNbLagChunk(); chunklag < GetNbProcessedChunk()-1; ++chunklag)
    for (int chunklag = 0; chunklag < GetNbProcessedChunk()-1; ++chunklag)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            //If the user want to switch
            if (m_users[u].IsAtViewpoint(chunklag+1+k*GetNbProcessedChunk(), viewpointId) && !m_users[u].IsAtViewpoint(chunklag+k*GetNbProcessedChunk(), viewpointId))
            {
                IloExpr sum(m_env);
                for (int chunkId = std::max(0, chunklag+1); chunkId < GetNbProcessedChunk(); ++chunkId)
                {
                    for (int tileId = 0; tileId < GetNbTile(); ++tileId)
                    {
                        for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                        {
                            sum += m_d[GetDId(chunkId, chunklag, viewpointId, tileId, qualityLevel)];
                        }
                    }
                }
                //cannot start download before the switching decision
                //m_model.add(sum <= m_bandwidths[b].GetBandwidth(chunklag+k*GetNbProcessedChunk())*(1-switchingDecisionTime+m_stall[chunklag+GetNbLagChunk()]));
                if (m_users[u].GetSwitchingDecisionTime(chunklag+k*GetNbProcessedChunk()) < 0)
                {
                    std::cerr << "Wrong switching decision time!!! " << m_users[u].GetSwitchingDecisionTime(chunklag+k*GetNbProcessedChunk()) << " " << chunklag + k*GetNbProcessedChunk() << " " << viewpointId << std::endl;
                    exit(1);
                }
                m_model.add(sum <= m_bandwidths[b].GetBandwidth(chunklag+k*GetNbProcessedChunk())*(std::max(0.f, 1-m_users[u].GetSwitchingDecisionTime(chunklag+k*GetNbProcessedChunk())-transmissionDelay)+STALL_COUNT_IN_SECOND*m_stall[chunklag+GetNbLagChunk()]));
           }
        }
    }
 

    ////Download segment in display order: ds[chunkLagId][chunkId] == 1 if segment of chunkId  was downloaded during chunk chunkLagId
    //for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
    //{
    //    for (int  chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
    //    {
    //        IloExpr sum(m_env);
    //        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
    //        {
    //            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
    //            {
    //                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
    //                {
    //                    sum += m_d[GetDId(chunkId,chunkLagId, viewpointId, tileId, qualityLevel)];
    //                }
    //            }
    //        }
    //        IloExpr sum2(m_env);
    //        for (int cLagId = chunkLagId + 1; cLagId < GetNbChunk(); ++cLagId)
    //        {
    //            for (int  c = 0; c < chunkId; ++c)
    //            {
    //                sum2 += m_ds[GetDsId(c, cLagId)];
    //            }
    //        }
    //        m_model.add(IloIfThen(m_env, m_ds[GetDsId(chunkId, chunkLagId)] == 1, sum2 == 0));
    //    }
    //}
    //////for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
    //////{
    //////    for (int  chunkId = std::max(1, chunkLagId-1); chunkId < GetNbProcessedChunk(); ++chunkId)
    //////    {
    //////        if (m_users[u].GetSwitchingDecisionTime(std::max(0, chunkId-1)+k*GetNbProcessedChunk()) >= 0)
    //////        //if (m_users[u].GetWantedViewpoint(std::max(0, chunkId-1)+k*GetNbProcessedChunk()) != m_users[u].GetWantedViewpoint(std::max(0, chunkId)+k*GetNbProcessedChunk()))
    //////        {
    //////            int counter = 0;
    //////            int counter2 = 0;
    //////            IloExpr sum(m_env);
    //////            for (int  chunkLagIdPrime = chunkLagId+1; chunkLagIdPrime < GetNbProcessedChunk(); ++chunkLagIdPrime)
    //////            {
    //////                //for (int  chunkIdPrime = std::max(chunkId-1, 0); chunkIdPrime < chunkId; ++chunkIdPrime)
    //////                //for (int  chunkIdPrime = 0; chunkIdPrime < chunkId; ++chunkIdPrime)
    //////                for (int  chunkIdPrime = 0; chunkIdPrime < chunkId; ++chunkIdPrime)
    //////                {
    //////                    for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
    //////                    {
    //////                        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
    //////                        {
    //////                            for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
    //////                            {
    //////                                //sum += m_d[GetDId(chunkIdPrime, chunkLagIdPrime, viewpointId, tileId, qualityLevel)];
    //////                                //++counter;
    //////                            }
    //////                        }
    //////                    }
    //////                    sum += m_ds[GetDsId(chunkIdPrime, chunkLagIdPrime)];
    //////                    ++counter2;
    //////                }
    //////            }
    //////            IloExpr ds_sum(m_env);
    //////            for (int  chunkIdPrime = chunkId; chunkIdPrime < GetNbProcessedChunk(); ++chunkIdPrime)
    //////            {
    //////                ds_sum += m_ds[GetDsId(chunkIdPrime, chunkLagId)];
    //////                //m_model.add(IloIfThen(m_env, m_ds[GetDsId(chunkIdPrime, chunkLagId)] == 1, sum == 0));
    //////            }
    //////            //m_model.add(sum <= (D_MAX*counter + counter2)*(1-m_ds[GetDsId(chunkId, chunkLagId)]));
    //////            //m_model.add(IloIfThen(m_env, m_ds[GetDsId(chunkId, chunkLagId)] == 1, sum == 0));
    //////            m_model.add(IloIfThen(m_env, ds_sum >= 1, sum == 0));
    //////        }
    //////    }
    //////}
}


//void OptimalVerticalScenario::Run(void)
//{
//    float DistortionMax = NOT_DOWNLOADED_DISTORTION;
//    //Init the Cplex environment
//    IloEnv env;                                                                
//    IloModel model(env);
//
//    //Create the decision variables
//    IloArray<IloArray<IloArray<IloIntVarArray>>> x(env, GetNbChunk());
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        x[chunkId] = IloArray<IloArray<IloIntVarArray>>(env, GetNbViewpoint());
//        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//        {
//            x[chunkId][viewpointId] = IloArray<IloIntVarArray>(env, GetNbTile());
//            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//            {
//                x[chunkId][viewpointId][tileId] = IloIntVarArray(env, GetNbQuality()+1, 0, 1);
//                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    x[chunkId][viewpointId][tileId][qualityLevel+1].setName(("x["+
//                                std::to_string(chunkId) + "][" +
//                                std::to_string(viewpointId) + "][" +
//                                std::to_string(tileId) + "][" +
//                                std::to_string(qualityLevel) + "]").c_str());
//                }
//            }
//        }
//    }
//    IloArray<IloIntVarArray> y(env, GetNbChunk());
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        y[chunkId] = IloIntVarArray(env, GetNbViewpoint(), 0, 1);
//    }
//    IloArray<IloArray<IloArray<IloArray<IloNumVarArray>>>> d(env, GetNbChunk());
//    //Create the decision variables
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        d[chunkId] = IloArray<IloArray<IloArray<IloNumVarArray>>>(env, GetNbChunk()+GetNbLagChunk());
//        for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//        {
//            d[chunkId][chunkLagId+GetNbLagChunk()] = IloArray<IloArray<IloNumVarArray>>(env, GetNbViewpoint());
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId] = IloArray<IloNumVarArray>(env, GetNbTile());
//                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//                {
//                    d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId] = IloNumVarArray(env, GetNbQuality(), 0, 1, ILOFLOAT);
//                    //d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId] = IloNumVarArray(env, GetNbQuality(), 0, 1, ILOINT);
//                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                    {
//                        d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel].setName(("d["+
//                                    std::to_string(chunkId) + "][" +
//                                    std::to_string(viewpointId) + "][" +
//                                    std::to_string(tileId) + "][" +
//                                    std::to_string(qualityLevel) + "]").c_str());
//
//                    }
//                }
//            }
//        }
//    }
//    IloIntVarArray stall(env, GetNbChunk()+GetNbLagChunk(), 0, 25000);
//
//    //Contraints
//    //Contraints 1a and 1b
//    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//    {
//        IloExpr sum1a(env);
//        IloExpr sum1b(env);
//        for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//        {
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//                {
//                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                    {
//                        if (int(chunkId) <= int(chunkLagId))
//                        {
//                            //sum1b += d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//                            d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel].setUB(0);
//                        }
//                        else
//                        {
//                            sum1a += m_as.GetSeg(chunkId, viewpointId, tileId, qualityLevel).GetBitrate() * d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//                        }
//                    }
//                }
//            }
//        }
//
// 
//        IloConstraint ic1a = sum1a <= m_bandwidths[b].GetBandwidth(chunkLagId)*(1+stall[chunkLagId+GetNbLagChunk()]);
//        ic1a.setName(("Const1a["+std::to_string(chunkLagId)+"]").c_str());
//        model.add(ic1a);
//       // if (chunkLagId >= 0)
//       // {
//       //     IloConstraint ic1b = sum1b == IloInt(0);
//       //     ic1b.setName(("Const1b["+std::to_string(chunkLagId)+"]").c_str());
//       //     model.add(ic1b);
//       // }
//    }
//    //contraints 1c and 1d
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//        {
//            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//            {
//                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    IloExpr sum(env);
//                    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < int(GetNbChunk()); ++chunkLagId)
//                    {
//                        sum += d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//                    }
//                    IloConstraint ic1c = sum <= IloInt(1);
//                    //IloConstraint ic1c = sum1c == x[chunkId][viewpointId][tileId][qualityLevel+1];
//                    ic1c.setName(("Const1c[" + std::to_string(chunkId) +
//                                "][" + std::to_string(viewpointId) + "][" +
//                                std::to_string(tileId) + "][" +
//                                std::to_string(qualityLevel) + "]").c_str());
//                    model.add(ic1c);
//                    IloConstraint ic1d = x[chunkId][viewpointId][tileId][qualityLevel+1] <= sum;
//                    ic1d.setName(("Const1d[" + std::to_string(chunkId) +
//                                "][" + std::to_string(viewpointId) + "][" +
//                                std::to_string(tileId) + "][" +
//                                std::to_string(qualityLevel) + "]").c_str());
//                    model.add(ic1d);
//                }
//            }
//        }
//    }
//    //contraints 1e
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//       for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//       {
//           IloExpr sum(env);
//           float visibility = m_user.GetTileVisibility(chunkId, tileId);
//           for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//           {
//                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    sum += x[chunkId][viewpointId][tileId][qualityLevel+1];
//                }
//           }
//           IloConstraint ic1e = std::ceil(visibility) == sum;
//           ic1e.setName(("Const1e[" + std::to_string(chunkId) +
//                           "][" +
//                           std::to_string(tileId) + "]").c_str());
//           model.add(ic1e);
//        }
//    }
//    //contraints 1f: can only download current viewpoint
//    for (int chunklag = -GetNbLagChunk(); chunklag < GetNbChunk()-1; ++chunklag)
//    {
//        for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//        {
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//                {
//                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                    {
//                        if (!m_user.IsAtViewpoint(std::max(0, chunklag+1), viewpointId))
//                        {
//                            d[chunkId][chunklag+GetNbLagChunk()][viewpointId][tileId][qualityLevel].setUB(0);
//                        }
//                        //model.add(d[chunkId][chunklag+GetNbLagChunk()][viewpointId][tileId][qualityLevel] <= y[std::max(0, chunklag)][viewpointId] + y[std::max(0, chunklag+1)][viewpointId]);
//                    }
//                }
//            }
//        }
//    }
//    //contraints: exactly one viewpoint displayed
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)                
//    {
//        model.add(IloSum(y[chunkId]) == 1);
//    }
//    //contraints: display only one viewpoint
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)                    
//    {
//        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//        {
//            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//            {
//                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    model.add(x[chunkId][viewpointId][tileId][qualityLevel+1] <= y[chunkId][viewpointId]);
//                }
//            }
//        }
//    }
//    //Display only one viewpoint -> the last displayed or the current wanted viewpoint
//    for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//    {
//        if (m_user.IsAtViewpoint(0, viewpointId))
//        {
//            y[0][viewpointId].setLB(1);
//        }
//        else
//        {
//            y[0][viewpointId].setUB(0);
//        }
//    }
//    for (int chunkId = 1; chunkId < GetNbChunk(); ++chunkId)
//    {
//        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//        {
//            model.add( y[chunkId][viewpointId] <= y[chunkId-1][viewpointId] + (m_user.IsAtViewpoint(chunkId, viewpointId) ? 1 : 0));
//        }
//    }
//
//    //Download segment in display order: ds[chunkLagId][chunkId] == 1 if segment of chunkId  was downloaded during chunk chunkLagId
//    IloArray<IloIntVarArray> ds(env, GetNbChunk()+GetNbLagChunk());                               
//    for (int chunkId = 0; chunkId < GetNbChunk()+GetNbLagChunk(); ++chunkId)                    
//    {                                                                            
//        ds[chunkId] = IloIntVarArray(env, GetNbChunk(), 0, 1); 
//    }
//    for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//    {
//        for (int  chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//        {
//            IloExpr sum(env);
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//                {
//                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                    {
//                        sum += d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//                    }
//                }
//            }
//            IloExpr sum2(env);
//            for (int cLagId = chunkLagId + 1; cLagId < GetNbChunk(); ++cLagId)
//            {
//                for (int  c = 0; c < chunkId; ++c)
//                {
//                    sum2 += ds[cLagId+GetNbLagChunk()][c];
//                }
//            }
//            model.add(IloIfThen(env, ds[chunkLagId+GetNbLagChunk()][chunkId] == 1, sum2 == 0));
//            model.add((ds[chunkLagId+GetNbLagChunk()][chunkId] == 0) ==  (sum == 0));
//        }
//    }
//    //for (int  chunkId = 1; chunkId < GetNbChunk(); ++chunkId)
//    //{
//    //    for (int  chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//    //    {
//    //        IloExpr sumChunkId(env);
//    //        IloExpr sumPreviousChunkId(env);
//    //        int count = 0;
//    //        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//    //        {
//    //            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//    //            {
//    //                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//    //                {
//    //                    sumChunkId += d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//    //                    for (int  nextChunkLagId = chunkLagId+1; nextChunkLagId < chunkId-1; ++nextChunkLagId)
//    //                    {
//    //                        sumPreviousChunkId += d[chunkId-1][nextChunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//    //                        count += 1;
//    //                    }
//    //                }
//    //            }
//    //        }
//    //        if (count > 0)
//    //        {
//    //            model.add( (sumChunkId == 0) || (sumPreviousChunkId == 0));
//    //        }
//    //    }
//    //}
//
//
//
//    //Objective function:
//    float beta = LAG_COST;
//    IloExpr obj(env);
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//       for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//       {
//           float tileVisibility = m_user.GetTileVisibility(chunkId, tileId);
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    if (qualityLevel == -1)
//                    {
//                        obj += x[chunkId][viewpointId][tileId][qualityLevel+1]*(DistortionMax+(m_user.IsAtViewpoint(chunkId, viewpointId) ? 0 : beta))*tileVisibility;
//                    }
//                    else
//                    {
//                        obj += x[chunkId][viewpointId][tileId][qualityLevel+1]*(m_as.GetSeg(chunkId, viewpointId, tileId, qualityLevel).GetDistortion()+(m_user.IsAtViewpoint(chunkId, viewpointId) ? 0 : beta))*tileVisibility;
//                    }
//                }
//            }
//        }
//    }
//    const float alpha = STALL_QoE_IMPACT;
//    model.add(IloMinimize(env, obj/GetNbChunk()+alpha*IloSum(stall)/GetNbChunk()));
//
//    std::cout << std::endl << "Ready to run" << std::endl;
//
//    IloCplex cplex(model);
//    //cplex.setParam(IloCplex::EpGap, m_conf->epGap);
//    cplex.setParam(IloCplex::EpGap, m_optimalGap);
//    cplex.setParam(IloCplex::TiLim, 330);
//    //cplex.setParam(IloCplex::Threads, m_conf->nbThread);
//    cplex.setParam(IloCplex::Threads, m_nbThread);
//    //cplex.setParam(IloCplex::MIPOrdType, 2);
//    cplex.setParam(IloCplex::BrDir, -1);
//    //cplex.setParam(IloCplex::Probe, 3);
//    //cplex.setParam(IloCplex::PolishAfterNode, 2000);
//    cplex.setParam(IloCplex::PolishAfterTime, 250);
//    cplex.setParam(IloCplex::MIPEmphasis, 1);
//    cplex.setParam(IloCplex::RINSHeur, 25);
//    cplex.setParam(IloCplex::DisjCuts, 2);
//    // cplex.setParam(IloCplex::NodeFileInd,3);
//    // cplex.setParam(IloCplex::WorkDir,".");
//    // cplex.setParam(IloCplex::MemoryEmphasis,true);
//    //cplex.setParam(IloCplex::DataCheck, true);
//
//    //cplex.exportModel((std::string("model.lp")).c_str());
//
//
//    //if(GetNbTile() > 6)
//    //{
//    //    IloNumVarArray startVar(env);
//    //    IloNumArray startVal(env);
//    //    std::cout << "Generate initial solution" << std::endl;
//    //    std::vector<float> bandwidth(GetNbChunk()+GetNbLagChunk(), 0);
//    //    float totalBandwidth = 0;
//    //    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//    //    {
//    //        bandwidth[chunkLagId+GetNbLagChunk()] = m_bandwidths[b].GetBandwidth(chunkLagId);
//    //        totalBandwidth += bandwidth[chunkLagId];
//    //    }
//    //    float totalVisibleLowQuality = 0;
//	//	for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)                 
//    //    {                                                                        
//    //    	for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//	//        {                                                                    
//    //	        for (int tileId = 0; tileId < GetNbTile(); ++tileId)             
//    //            {
//    //                if (m_user.GetVisibility(chunkId, viewpointId, tileId) > 0)
//    //                {
//    //                    totalVisibleLowQuality += m_as.GetSeg(chunkId, viewpointId, tileId, 0).GetBitrate();
//    //                }
//    //            }
//    //        }
//    //    }
//    //    int nbStall = 0;
//    //    while(totalBandwidth < totalVisibleLowQuality)
//    //    {
//    //        bandwidth[0] += m_bandwidths[b].GetBandwidth(-GetNbLagChunk());
//    //        totalBandwidth += m_bandwidths[b].GetBandwidth(-GetNbLagChunk());
//    //        nbStall += 1;
//    //    }
//    //    std::cout << "nbStall = " << nbStall << " " <<  totalBandwidth << " >= " << totalVisibleLowQuality << std::endl;
//    //    std::vector<float> d_chunkTile(GetNbChunk()*GetNbViewpoint()*GetNbTile(), 0);
//    //    auto ToId = [&](int chunkId, int viewpointId, int tileId) {return chunkId*GetNbViewpoint()*GetNbTile()+viewpointId*GetNbTile()+tileId;};
//    //    std::vector<int> count(GetNbChunk(), 0);
//    //    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    //    {
//    //        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//    //        {
//    //            if (m_user.GetTileVisibility(chunkId, tileId) > 0)
//    //            {
//    //                ++count[chunkId];
//    //            }
//    //        }
//    //    }
//    //    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//    //    {
//    //        std::vector<int> usedChunk(GetNbChunk(), 0);
//    //        startVar.add(stall[chunkLagId+GetNbLagChunk()]);
//    //        if (chunkLagId == -GetNbLagChunk())
//    //        {
//    //            startVal.add(0);
//    //        }
//    //        else
//    //        {
//    //            startVal.add(nbStall);
//    //        }
//    //        std::vector<float> d_start(GetNbChunk()*GetNbViewpoint()*GetNbTile(), 0);
//
//    //        for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    //        {
//    //            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//    //            {
//    //                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//    //                {
//    //                    if ( (chunkLagId <= 0 || m_user.IsAtViewpoint(chunkLagId+1, viewpointId) || m_user.IsAtViewpoint(chunkLagId, viewpointId)) && bandwidth[chunkLagId+GetNbLagChunk()] > 0 && d_chunkTile[ToId(chunkId, viewpointId, tileId)] < 1 && m_user.GetVisibility(chunkId, viewpointId, tileId) > 0)
//    //                    {
//    //                        if (chunkId > 0  && count[chunkId-1] != 0)
//    //                        {
//    //                            continue;
//    //                        }
//    //                        float remaining = (1-d_chunkTile[ToId(chunkId, viewpointId, tileId)])*m_as.GetSeg(chunkId, viewpointId, tileId, 0).GetBitrate();
//    //                        //std::cout << chunkId<< "; " << viewpointId << "; " << tileId << " R= " << remaining << " Before " << bandwidth[chunkLagId+GetNbLagChunk()];
//    //                        if (bandwidth[chunkLagId+GetNbLagChunk()] >= remaining)
//    //                        {
//    //                           // std::cout << " * ";
//    //                            bandwidth[chunkLagId+GetNbLagChunk()] -= remaining;
//    //                            d_start[ToId(chunkId, viewpointId, tileId)] = (1-d_chunkTile[ToId(chunkId, viewpointId, tileId)]);
//    //                            d_chunkTile[ToId(chunkId, viewpointId, tileId)] = 1;
//    //                            --count[chunkId];
//    //                        }
//    //                        else
//    //                        {
//    //                            //std::cout << " # ";
//    //                            float used = bandwidth[chunkLagId+GetNbLagChunk()];
//    //                            remaining -= bandwidth[chunkLagId+GetNbLagChunk()];
//    //                            bandwidth[chunkLagId+GetNbLagChunk()] = 0;
//    //                            d_start[ToId(chunkId, viewpointId, tileId)] = used / m_as.GetSeg(chunkId, viewpointId, tileId, 0).GetBitrate();
//    //                            d_start[ToId(chunkId, viewpointId, tileId)] += d_start[ToId(chunkId, viewpointId, tileId)];
//    //                        }
//    //                        //std::cout << " After " << bandwidth[chunkLagId+GetNbLagChunk()]  << "R = " << (1-d_chunkTile[ToId(chunkId, viewpointId, tileId)])*m_as.GetSeg(chunkId, viewpointId, tileId, 0).GetBitrate() << std::endl;;
//    //                        usedChunk[chunkId] = 1; 
//    //                    }
//    //                }
//    //            }
//    //        }
//    //        std::cout << chunkLagId << ": ";
//    //        for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    //        {
//    //            startVar.add(ds[chunkLagId+GetNbLagChunk()][chunkId]);
//    //            startVal.add(usedChunk[chunkId]);
//    //            if (usedChunk[chunkId] == 1)
//    //            {
//    //                std::cout << chunkId << " ";
//    //            }
//    //        }
//    //        std::cout << std::endl;
//    //    }
//    //    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    //    {
//    //        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//    //        {
//    //            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//    //            {
//    //                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
//    //                {
//    //                    if (qualityLevel == 0 && m_user.GetVisibility(chunkId, viewpointId, tileId) > 0)
//    //                    {
//    //                        startVar.add(x[chunkId][viewpointId][tileId][qualityLevel+1]);
//    //                        startVal.add(1);
//    //                    }
//    //                    else
//    //                    {
//    //                        startVar.add(x[chunkId][viewpointId][tileId][qualityLevel+1]);
//    //                        startVal.add(0);
//    //                    }
//    //                }
//    //            }
//    //            if (m_user.IsAtViewpoint(chunkId, viewpointId))
//    //            {
//    //                startVar.add(y[chunkId][viewpointId]);
//    //                startVal.add(1);
//    //            }
//    //            else
//    //            {
//    //                startVar.add(y[chunkId][viewpointId]);                       
//    //                startVal.add(0);
//    //            }
//    //        }
//    //    }
//    //    std::cout << "Inject Solution" << std::endl;
//    //    cplex.addMIPStart(startVar, startVal);
//    //    startVar.end();
//    //    startVal.end();
//    //    std::cout << "Test " << std::endl;
//    //}
//
//    std::cout << "Start solving" << std::endl;
//
//    //if(fs::exists("lastSol.txt"))
//    //{
//    //    cplex.readSolution((std::string("lastSol.txt")).c_str());
//    //}
//    cplex.solve();
//    //cplex.writeSolution((std::string("lastSol.txt")).c_str());
//
//    std::cout << "Solving done\n" << std::endl;
//
//    std::cout << "Solution status " << cplex.getStatus() << std::endl;
//    std::cout <<  "Objective value " << cplex.getObjValue() << std::endl;
//    std::cout <<  "Solution gap " << cplex.getMIPRelativeGap() << "\n" << std::endl;
//
//    m_objectiveValue = cplex.getObjValue();
//    m_solutionGap = cplex.getMIPRelativeGap();
//    m_solutionStatus = cplex.getStatus() == IloAlgorithm::Optimal ? "Optimal" : (cplex.getStatus() == IloAlgorithm::Infeasible ? "Infeasible" : ( cplex.getStatus() == IloAlgorithm::Feasible ? "Feasible": "Unknown"));
//
//
//    //Get the results
//    IloArray<IloArray<IloArray<IloNumArray>>> x_sol(env, GetNbChunk());
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        x_sol[chunkId] = IloArray<IloArray<IloNumArray>>(env, GetNbViewpoint());
//        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//        {
//            x_sol[chunkId][viewpointId] = IloArray<IloNumArray>(env, GetNbTile());
//            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//            {
//                x_sol[chunkId][viewpointId][tileId] = IloNumArray(env, GetNbQuality()+1);
//                cplex.getValues(x_sol[chunkId][viewpointId][tileId], x[chunkId][viewpointId][tileId]);
//                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    if (x_sol[chunkId][viewpointId][tileId][qualityLevel+1] > 0.5)
//                    {
//                        m_as.GetSeg(chunkId, viewpointId, tileId, qualityLevel).SetSelected();
//                    }
//                }
//		x_sol[chunkId][viewpointId][tileId].end();
//		x[chunkId][viewpointId][tileId].end();
//            }
//	    x_sol[chunkId][viewpointId].end();
//	    x[chunkId][viewpointId].end();
//        }
//	x_sol[chunkId].end();
//	x[chunkId].end();
//    }
//    x_sol.end();
//    x.end();
//    IloArray<IloNumArray> y_sol(env, GetNbChunk());
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        y_sol[chunkId] = IloNumArray(env, GetNbViewpoint());
//        cplex.getValues(y_sol[chunkId], y[chunkId]);
//        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//        {
//            if (y_sol[chunkId][viewpointId] == 1)
//            {
//                m_displayedViewpoint[chunkId]= viewpointId;
//                break;
//            }
//        }
//        y[chunkId].end();
//    }
//    y.end();
//    IloArray<IloArray<IloArray<IloArray<IloNumArray>>>> d_sol(env, GetNbChunk());
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        d_sol[chunkId] = IloArray<IloArray<IloArray<IloNumArray>>>(env, GetNbChunk()+GetNbLagChunk());
//        for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//        {
//            d_sol[chunkId][chunkLagId+GetNbLagChunk()] = IloArray<IloArray<IloNumArray>>(env, GetNbViewpoint());
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                d_sol[chunkId][chunkLagId+GetNbLagChunk()][viewpointId] = IloArray<IloNumArray>(env, GetNbTile());
//                for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//                {
//                    d_sol[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId] = IloNumArray(env, GetNbQuality());
//                    cplex.getValues(d_sol[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId], d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId]);
//                    for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//                    {
//                        SetDownloadRatio(chunkId, chunkLagId, viewpointId, tileId, qualityLevel, d_sol[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel]); 
//                    }
//		    d_sol[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId].end();
//		    d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId].end();
//                }
//		d_sol[chunkId][chunkLagId+GetNbLagChunk()][viewpointId].end();
//		d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId].end();
//            }
//	d_sol[chunkId][chunkLagId+GetNbLagChunk()].end();
//        d[chunkId][chunkLagId+GetNbLagChunk()].end();
//        }
//    d_sol[chunkId].end();
//    d[chunkId].end();
//    }
//    d_sol.end(); 
//    d.end();
//    IloNumArray stall_sol(env, GetNbChunk()+GetNbLagChunk());
//    cplex.getValues(stall_sol, stall);
//    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//    {
//        m_nbStall[chunkLagId+GetNbLagChunk()] = stall_sol[chunkLagId+GetNbLagChunk()];
//    }
//    stall_sol.end();
//    stall.end();
//
//    for (int chunkId = 0; chunkId < GetNbChunk()+GetNbLagChunk(); ++chunkId)                    
//    {                                                                            
//        IloNumArray ds_sol(env, GetNbChunk());
//        cplex.getValues(ds_sol, ds[chunkId]);
//        std::cout  << "ChunkLagId " << int(chunkId)-GetNbLagChunk() <<" ("<< m_user.GetWantedViewpoint(std::max(0, int(chunkId)-GetNbLagChunk())) <<"): ";
//        for (int c = 0; c < GetNbChunk(); ++c)
//        {
//            bool downloadVidChunk(false);
//            //if (ds_sol[c] == 1)
//            std::vector<int> viewpointList;
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                bool done = false;
//                for (int tileId = 0; tileId < GetNbTile() && !done; ++tileId)
//                {
//                    for (int qualityLevel = 0; qualityLevel < GetNbQuality() && !done; ++qualityLevel)
//                    {
//                        if (m_downloadRatio[ToDownloadRatioVecId(c, chunkId-GetNbLagChunk(),viewpointId,tileId,qualityLevel)] == INVALIDE_FLOAT_VALUE)
//                        {
//                            std::cout << "Error: invalid float" << std::endl;
//                        }
//                        if (!done && m_downloadRatio[ToDownloadRatioVecId(c, chunkId-GetNbLagChunk(),viewpointId,tileId,qualityLevel)] > 0)
//                        {
//                            done = true;
//                            viewpointList.push_back(viewpointId);
//                            downloadVidChunk = true;
//                        }
//                    }
//                }
//            }
//
//            if (downloadVidChunk || ds_sol[c] == 1)
//            {
//                std::cout << c << (ds_sol[c] == 1 ? "*" : "") << (downloadVidChunk ? "#" : "") <<"(";
//                bool first(true);
//                for(auto v: viewpointList)
//                {
//                    if (!first) std::cout << ", ";
//                    std::cout << v;
//                    first = false;
//                }
//                //std::cout.seekp(-2, std::ios_base::end);
//                std::cout << ") ";
//            }
//        }
//        std::cout << std::endl;
//        ds[chunkId].end();
//        ds_sol.end();
//    }
//    ds.end();
//    //cleanup
//    cplex.end();
//    obj.end();
//    model.end();
//    env.end();
//}
