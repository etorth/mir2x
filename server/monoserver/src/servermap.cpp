/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 05/07/2016 04:08:23
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
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "servermap.hpp"
#include "charobject.hpp"
#include "monoserver.hpp"
#include "rotatecoord.hpp"
#include "regionmonitor.hpp"

ServerMap::ServerMap(uint32_t nMapID)
    : m_ID(nMapID)
    , m_NullAddress(Theron::Address::Null())
    , m_RegionMonitorReady(false)
    , m_RegionW(1)
    , m_RegionH(1)
    , m_Metronome(nullptr)
{
    Load("./DESC.BIN");

    int nGridW = (W() + m_RegionW - 1) / m_RegionW;
    int nGridH = (H() + m_RegionH - 1) / m_RegionH;

    m_RegionMonitorRecordV2D.resize(nGridH);
    for(auto &rstRecordV: m_RegionMonitorRecordV2D){
        rstRecordV.resize(nGridW);
    }

    CheckRegionMonitorNeed();
}

bool ServerMap::Load(const char *szMapFullName)
{
    if(!m_Mir2xMap.Load(szMapFullName)){ return false; }
    // int nW = m_Mir2xMap.W();
    // int nH = m_Mir2xMap.H();

    return true;
}

void ServerMap::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    extern Log *g_Log;
    switch(rstMPK.Type()){
        case MPK_HI:
            {
                On_MPK_HI(rstMPK, rstFromAddr);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
                break;
            }
        case MPK_REGIONMONITORREADY:
            {
                On_MPK_REGIONMONITORREADY(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ADDMONSTER:
            {
                On_MPK_ADDMONSTER(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NEWMONSTER:
            {
                On_MPK_NEWMONSTER(rstMPK, rstFromAddr);
                break;
            }
        default:
            {
                g_Log->AddLog(LOGTYPE_WARNING,
                        "unsupported message type: (%d:%s)", rstMPK.Type(), rstMPK.Name());
                break;
            }
    }
}

void ServerMap::CheckRegionMonitorNeed()
{
    if(!m_Mir2xMap.Valid()){ return; }

    for(size_t nRMY = 0; nRMY < m_RegionMonitorRecordV2D.size(); ++nRMY){
        for(size_t nRMX = 0; nRMX < m_RegionMonitorRecordV2D[0].size(); ++nRMX){

            for(size_t nY = 0; nY < m_RegionH; ++nY){
                for(size_t nX = 0; nX < m_RegionW; ++nX){
                    size_t nGX = nX + nRMX * m_RegionW;
                    size_t nGY = nY + nRMY * m_RegionH;
                    if(false
                            || m_Mir2xMap.CanWalk(nGX, nGY, 0)
                            || m_Mir2xMap.CanWalk(nGX, nGY, 1)
                            || m_Mir2xMap.CanWalk(nGX, nGY, 2)
                            || m_Mir2xMap.CanWalk(nGX, nGY, 3)){
                        m_RegionMonitorRecordV2D[nRMY][nRMX].Need = true;
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
    for(size_t nY = 0; nY < m_RegionMonitorRecordV2D.size(); ++nY){
        for(size_t nX = 0; nX < m_RegionMonitorRecordV2D[0].size(); ++nX){
            if(!m_RegionMonitorRecordV2D[nY][nX].Ready()){
                m_RegionMonitorReady = false;
                return false;
            }
        }
    }
    m_RegionMonitorReady = true;
    return true;
}

void ServerMap::CreateRegionMonterV2D()
{
    if(RegionMonitorReady()){ return; }
    if(CheckRegionMonitorReady()){ return; }

    for(size_t nGY = 0; nGY < m_RegionMonitorRecordV2D.size(); ++nGY){
        for(size_t nGX = 0; nGX < m_RegionMonitorRecordV2D[0].size(); ++nGX){
            if(m_RegionMonitorRecordV2D[nGY][nGX].Ready()){ continue; }

            auto pNewMonitor = new RegionMonitor(GetAddress());
            auto stAddress   = pNewMonitor->Activate();

            m_RegionMonitorRecordV2D[nGY][nGX].Data = pNewMonitor;
            m_RegionMonitorRecordV2D[nGY][nGX].PodAddress = stAddress;
        }
    }

    for(size_t nGY = 0; nGY < m_RegionMonitorRecordV2D.size(); ++nGY){
        for(size_t nGX = 0; nGX < m_RegionMonitorRecordV2D[0].size(); ++nGX){
            if(!m_RegionMonitorRecordV2D[nGY][nGX].Need){ continue; }
            if(!m_RegionMonitorRecordV2D[nGY][nGX].Ready()){
                // should be error
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "init region monitor failed");
                m_RegionMonitorReady = false;
                return;
            }

            // 1. send boundary information
            AMRegion stAMRegion;
            stAMRegion.X = m_RegionW * SYS_MAPGRIDXP * nGX;
            stAMRegion.Y = m_RegionH * SYS_MAPGRIDYP * nGY;
            stAMRegion.W = m_RegionW * SYS_MAPGRIDXP;
            stAMRegion.H = m_RegionH * SYS_MAPGRIDYP;

            stAMRegion.LocX = nGX;
            stAMRegion.LocY = nGY;

            stAMRegion.MapID = ID();

            m_ActorPod->Forward({MPK_INITREGIONMONITOR,
                    stAMRegion}, m_RegionMonitorRecordV2D[nGY][nGX].PodAddress);

            // 2. send neighbor information
            std::vector<char> stStringAddress;
            for(int nDY = -1; nDY <= 1; ++nDY){
                for(int nDX = -1; nDX <= 1; ++nDX){
                    int nRMX = nGX + nDX;
                    int nRMY = nGY + nDY;

                    std::string szAddr;
                    if(true
                            && (nDX || nDY)
                            && (nRMY >= 0 && nRMY < (int)m_RegionMonitorRecordV2D.size())
                            && (nRMX >= 0 && nRMX < (int)m_RegionMonitorRecordV2D[0].size())
                            && m_RegionMonitorRecordV2D[nRMY][nRMX].Valid()){
                        szAddr = m_RegionMonitorRecordV2D[nRMY][nRMX].PodAddress.AsString();
                    }

                    stStringAddress.insert(stStringAddress.end(), szAddr.begin(), szAddr.end());
                    stStringAddress.push_back('\0');
                }
            }

            m_ActorPod->Forward({MPK_NEIGHBOR, (const uint8_t *)&stStringAddress[0],
                        stStringAddress.size()}, m_RegionMonitorRecordV2D[nGY][nGX].PodAddress);
        }
    }
    m_RegionMonitorReady = true;
}
