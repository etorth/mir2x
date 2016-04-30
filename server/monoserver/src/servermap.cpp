/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 04/30/2016 01:19:36
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

#include <algorithm>

#include "monster.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "servermap.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"

ServerMap::ServerMap(uint32_t nMapID)
    : m_ID(nMapID)
    , m_NullAddress(Theron::Address::Null())
    , m_RegionMonitorReady(false)
    , m_RegionMonitorResolution(10)
    , m_Metronome(nullptr)
{
    Load("./DESC.BIN");
}

bool ServerMap::Load(const char *szMapFullName)
{
    if(!m_Mir2xMap.Load(szMapFullName)){ return false; }
    // int nW = m_Mir2xMap.W();
    // int nH = m_Mir2xMap.H();

    return true;
}

void ServerMap::Operate(const MessagePack &rstMPK, const Theron::Address &stFromAddr)
{
    if(!RegionMonitorReady()){ return; }

    extern Log *g_Log;
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                for(auto &rstRecordV: m_RegionMonitorRecordV2D){
                    for(auto &rstRecord: rstRecordV){
                        if(rstRecord.Valid()){
                            Send(MessagePack(MPK_METRONOME), rstRecord.PodAddress);
                        }
                    }
                }
                break;
            }
        case MPK_READY:
            {
                AMMonitorReady stAMMR;
                std::memcpy(&stAMMR, rstMPK.Data(), sizeof(stAMMR));
                m_RegionMonitorRecordV2D[stAMMR.Y][stAMMR.X].PodAddress = stFromAddr;
                CheckRegionMonitorReady();

                if(RegionMonitorReady()){
                    Send(MessagePack(MPK_READY), m_CoreAddress);
                }
                break;
            }

        case MPK_ADDMONSTER:
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

                    Send(MessagePack(MPK_NEWMONSTOR, stAMNM),
                            RegionMonitorAddressP(stAMAM.X, stAMAM.Y));
                }

                break;
            }

        case MPK_NEWMONSTOR:
            {
                AMNewMonster stAMNM;
                std::memcpy(&stAMNM, rstMPK.Data(), sizeof(stAMNM));

                // 1. create the monstor
                auto pNewMonster = new Monster(stAMNM.GUID, stAMNM.UID, stAMNM.AddTime);
                uint64_t nKey = ((uint64_t)stAMNM.UID << 32) + stAMNM.AddTime;

                // 2. put it in the pool
                m_CharObjectM[nKey] = pNewMonster;

                // 3. add the pointer inside and forward this message to the monitor
                stAMNM.Data = (void *)pNewMonster;
                auto stAddr = RegionMonitorAddressP(stAMNM.X, stAMNM.Y);
                Send(MessagePack(MPK_NEWMONSTOR, stAMNM), stAddr);

                break;
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_WARNING, "unsupported message type: %d", rstMPK.Type());
                break;
            }
    }
}

void ServerMap::CheckRegionMonitorNeed()
{
    if(!m_Mir2xMap.Valid()){ return; }

    int nRegionMonitorW = W() / m_RegionMonitorResolution;
    int nRegionMonitorH = H() / m_RegionMonitorResolution;

    for(int nRegionMonitorY = 0; nRegionMonitorY < nRegionMonitorW; ++nRegionMonitorY){
        for(int nRegionMonitorX = 0; nRegionMonitorX < nRegionMonitorH; ++nRegionMonitorX){
            // 1. find all grids it covers
            int nGridY = nRegionMonitorY * m_RegionMonitorResolution / SYS_MAPGRIDYP;
            int nGridX = nRegionMonitorX * m_RegionMonitorResolution / SYS_MAPGRIDXP;
            int nGridH = 1 + (m_RegionMonitorResolution / SYS_MAPGRIDYP);  // make it safe
            int nGridW = 1 + (m_RegionMonitorResolution / SYS_MAPGRIDXP);  // make it safe

            // 2. check each
            for(int nY = nGridY; nY < nGridY + nGridH; ++nY){
                for(int nX = nGridX; nX < nGridX + nGridW; ++nX){
                    if(false
                            || m_Mir2xMap.CanWalk(nX, nY, 0)
                            || m_Mir2xMap.CanWalk(nX, nY, 1)
                            || m_Mir2xMap.CanWalk(nX, nY, 2)
                            || m_Mir2xMap.CanWalk(nX, nY, 3)){
                        m_RegionMonitorRecordV2D[nRegionMonitorY][nRegionMonitorX].Need = true;
                        goto __LABEL_GOTO_CHECKMONITORNEED_1;
                    }
                }
            }
__LABEL_GOTO_CHECKMONITORNEED_1:;
        }
    }
}

bool ServerMap::CheckRegionMonitorReady()
{
    for(int nY = 0; nY < H() * SYS_MAPGRIDYP / m_RegionMonitorResolution; ++nY){
        for(int nX = 0; nX < W() * SYS_MAPGRIDXP / m_RegionMonitorResolution; ++nX){
            if(m_RegionMonitorRecordV2D[nY][nX].Need
                    && m_RegionMonitorRecordV2D[nY][nX].PodAddress == Theron::Address::Null()){
                m_RegionMonitorReady = false;
                return false;
            }
        }
    }
    m_RegionMonitorReady = true;
    return true;
}
