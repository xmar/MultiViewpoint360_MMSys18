#include "OptimalHorizontalScenario.hpp"

#include <ilcplex/ilocplex.h>
#include <cmath>

using namespace IMT;

#include <iostream>
void OptimalHorizontalScenario::SetSelectAndDownloadedConstraint(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int tileId = 0; tileId < GetNbTile(); ++tileId)
            {
                for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    IloExpr sum1d(m_env);
                    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                    {
                            sum1d += m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)];
                    }
                    IloExpr sumX(m_env);
                    for (int viewpointIdPrime = 0; viewpointIdPrime < GetNbViewpoint(); ++viewpointIdPrime)
                    {
                        sumX += m_x[GetXId(chunkId, viewpointIdPrime, tileId, qualityLevel)];
                    }
                    IloConstraint ic1d = sum1d == D_MAX*sumX;
                    m_model.add(ic1d);
                }
            }
        }
   }
}

void OptimalHorizontalScenario::Presolve(int k, int u, int b)
{
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
        {
            IloExpr sum(m_env);
            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
            {
                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
                {
                    if(m_users[u].GetTileVisibility(chunkId+k*GetNbProcessedChunk(), tileId)==0)
                    {
                        m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)].setUB(0);
                        for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbProcessedChunk(); ++chunkLagId)
                        {
                            if (qualityLevel != -1)
                            {
                                m_d[GetDId(chunkId, chunkLagId, viewpointId, tileId, qualityLevel)].setUB(0);
                            }
                        }
                    }
                    if (qualityLevel == -1)
                    {
                        m_x[GetXId(chunkId, viewpointId, tileId, qualityLevel)].setUB(0);
                    }
                }
            }
        }
    }
    for (int chunkId = 0; chunkId < GetNbProcessedChunk(); ++chunkId)
    {
        for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
        {
            for (int nbSwitch = 0; nbSwitch < m_users[u].GetNbSwitch(k, GetNbProcessedChunk())+1; ++nbSwitch)
            {
                if (!m_users[u].IsAtViewpoint(chunkId+k*GetNbProcessedChunk(), viewpointId) || nbSwitch != m_users[u].GetNbSwitchUntil(chunkId, k, GetNbProcessedChunk())) 
                {
                    m_y[GetYId(chunkId,viewpointId, nbSwitch, u, k)].setUB(0);
                }
                else
                {
                    m_y[GetYId(chunkId,viewpointId, nbSwitch, u, k)].setLB(1);
                }
            }
        }
    }
}

//void OptimalHorizontalScenario::Run(void)
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
//        //if (chunkLagId >= 0)
//        //{
//        //    IloConstraint ic1b = sum1b == IloInt(0);
//        //    ic1b.setName(("Const1b["+std::to_string(chunkLagId)+"]").c_str());
//        //    model.add(ic1b);
//        //}
//    }
//    //contraints 1c and 1d
//    for (int chunkId = 0; chunkId < GetNbChunk(); ++chunkId)
//    {
//        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//        {
//            for (int qualityLevel = 0; qualityLevel < GetNbQuality(); ++qualityLevel)
//            {
//                IloExpr sumX(env);
//                for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//                {
//                    sumX += x[chunkId][viewpointId][tileId][qualityLevel+1];
//                }
//                for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//                {
//
//                    IloExpr sum(env);
//                    for (int chunkLagId = -GetNbLagChunk(); chunkLagId < GetNbChunk(); ++chunkLagId)
//                    {
//                        sum += d[chunkId][chunkLagId+GetNbLagChunk()][viewpointId][tileId][qualityLevel];
//                    }
//
//                    IloConstraint ic1c = sum <= IloInt(1);
//                    ic1c.setName(("Const1c[" + std::to_string(chunkId) +
//                                "][" + std::to_string(viewpointId) + "][" +
//                                std::to_string(tileId) + "][" +
//                                std::to_string(qualityLevel) + "]").c_str());
//                    model.add(ic1c);
//                    IloConstraint ic1d = sumX == sum; //if this tile at this quality level is selected by on any viewpoint, then it should be downloaded
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
//        for (int tileId = 0; tileId < GetNbTile(); ++tileId)
//        {
//            IloExpr sum(env);
//            for (int viewpointId = 0; viewpointId < GetNbViewpoint(); ++viewpointId)
//            {
//                for (int qualityLevel = -1; qualityLevel < GetNbQuality(); ++qualityLevel)
//                {
//                    sum += x[chunkId][viewpointId][tileId][qualityLevel+1];
//                }
//            }
//            IloConstraint ic1e = IloNum(std::ceil(m_user.GetTileVisibility(chunkId, tileId))) == sum;
//            ic1e.setName(("Const1e[" + std::to_string(chunkId) +
//                            "][" +
//                            std::to_string(tileId) + "]").c_str());
//            model.add(ic1e);
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
//            //IloExpr sum2(env);
//            //for (int cLagId = chunkLagId + 1; cLagId < GetNbChunk(); ++cLagId)
//            //{
//            //    for (int  c = 0; c < chunkId; ++c)
//            //    {
//            //        sum2 += ds[cLagId+GetNbLagChunk()][c];
//            //    }
//            //}
//            //model.add(IloIfThen(env, ds[chunkLagId+GetNbLagChunk()][chunkId] == 1, sum2 == 0));
//            model.add((ds[chunkLagId+GetNbLagChunk()][chunkId] == 0) ==  (sum == 0));
//        }
//    }
//
//    //Objective function:
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
//                        obj += x[chunkId][viewpointId][tileId][qualityLevel+1]*(DistortionMax+(m_user.IsAtViewpoint(chunkId, viewpointId) ? 0 : LAG_COST))*tileVisibility;
//                    }
//                    else
//                    {
//                        obj += x[chunkId][viewpointId][tileId][qualityLevel+1]*(m_as.GetSeg(chunkId, viewpointId, tileId, qualityLevel).GetDistortion()+(m_user.IsAtViewpoint(chunkId, viewpointId) ? 0 : LAG_COST))*tileVisibility;
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
//    cplex.setParam(IloCplex::TiLim, 600);
//    //cplex.setParam(IloCplex::Threads, m_conf->nbThread);
//    cplex.setParam(IloCplex::Threads, m_nbThread);
//    //cplex.setParam(IloCplex::MIPOrdType, 2);
//    cplex.setParam(IloCplex::BrDir, -1);
//    cplex.setParam(IloCplex::Probe, 3);
//    //cplex.setParam(IloCplex::PolishAfterNode, 2000);
//    cplex.setParam(IloCplex::PolishAfterTime, 150);
//    cplex.setParam(IloCplex::MIPEmphasis, 1);
//    cplex.setParam(IloCplex::RINSHeur, 25);
//    cplex.setParam(IloCplex::DisjCuts, 2);
//    // cplex.setParam(IloCplex::NodeFileInd,3);
//    // cplex.setParam(IloCplex::WorkDir,".");
//    // cplex.setParam(IloCplex::MemoryEmphasis,true);
//    //cplex.setParam(IloCplex::DataCheck, true);
//
//
//    //cplex.exportModel((std::string("model.lp")).c_str());
//
//    std::cout << "Start solving" << std::endl;
//
//    cplex.solve();
//
//    std::cout << "Solving done\n" << std::endl;
//
//    std::cout << "Solution status " << cplex.getStatus() << std::endl;
//    std::cout <<  "Objective value " << cplex.getObjValue() << std::endl;
//    std::cout <<  "Solution gap " << cplex.getMIPRelativeGap() << "\n" << std::endl;
//
//    m_objectiveValue = cplex.getObjValue();
//    m_solutionGap = cplex.getMIPRelativeGap();
//    m_solutionStatus = cplex.getStatus() == IloAlgorithm::Optimal ? "Optimal" : (cplex.getStatus() == IloAlgorithm::Infeasible ? "Infeasible" : "Unknown");
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
//    for (int chunkId = 0; chunkId < GetNbChunk()+GetNbLagChunk(); ++chunkId)                    
//    {                                                                            
//        ds[chunkId].end();
//    }
//    ds.end();
//
//    //cleanup
//    cplex.end();
//    obj.end();
//    model.end();
//    env.end();
//}
