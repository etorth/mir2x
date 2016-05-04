/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 05/03/2016 20:30:49
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "actorpod.hpp"
#include "servermap.hpp"

void ServerMap::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecordV: m_RegionMonitorRecordV2D){
        for(auto &rstRecord: rstRecordV){
            if(rstRecord.Valid()){
                m_ActorPod->Forward(MPK_METRONOME, rstRecord.PodAddress);
            }
        }
    }
}

void ServerMap::On_MPK_REGIONMONITORREADY(const MessagePack &rstMPK, const Theron::Address &)
{
    AMRegionMonitorReady stAMMR;
    std::memcpy(&stAMMR, rstMPK.Data(), sizeof(stAMMR));
    m_RegionMonitorRecordV2D[stAMMR.LocY][stAMMR.LocX].CanRun = true;
    CheckRegionMonitorReady();

    // if(RegionMonitorReady()){
    //     m_ActorPod->Forward(MPK_READY, m_CoreAddress);
    // }
}

void ServerMap::On_MPK_ADDMONSTER(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAddMonster stAMAM;
    std::memcpy(&stAMAM, rstMPK.Data(), sizeof(stAMAM));

    if(!ValidP(stAMAM.X, stAMAM.Y) && stAMAM.Strict){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid monster adding request");
    }else{
        Monster *pNewMonster = 
            new Monster(stAMAM.MonsterIndex, stAMAM.UID, stAMAM.AddTime);
        AMNewMonster stAMNM;
        stAMNM.Data = pNewMonster;

        if(!RegionMonitorReady()){
            CreateRegionMonterV2D();

            if(!RegionMonitorReady()){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "create region monitors for server map failed");
                g_MonoServer->Restart();
            }
        }

        auto stAddr = RegionMonitorAddressP(stAMAM.X, stAMAM.Y);
        if(stAddr == Theron::Address::Null()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid location for new monstor");
            m_ActorPod->Forward(MPK_ERROR, stFromAddr,rstMPK.ID());
        }else{
            auto fnROP = [this, stFromAddr, nID = rstMPK.ID()](
                    const MessagePack &rstRMPK, const Theron::Address &){
                switch(rstRMPK.Type()){
                    case MPK_OK:
                        {
                            m_ActorPod->Forward(MPK_OK, stFromAddr, nID);
                            break;
                        }
                    default:
                        {
                            m_ActorPod->Forward(MPK_ERROR, stFromAddr, nID);
                            break;
                        }
                }
            };
            m_ActorPod->Forward({MPK_NEWMONSTOR, stAMNM}, stAddr, fnROP);
        }
    }
}
