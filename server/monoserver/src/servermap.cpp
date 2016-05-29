/*
 * =====================================================================================
 *
 *       Filename: servermap.cpp
 *        Created: 04/06/2016 08:52:57 PM
 *  Last Modified: 05/29/2016 04:49:11
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
    : Transponder()
    , m_MapID(nMapID)
    , m_RegionMonitorReady(false)
    , m_RegionW(1)
    , m_RegionH(1)
    , m_SCAddress(Theron::Address::Null())
    , m_Metronome(nullptr)
    , m_RMV2DCreated(false)
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

    return true;
}

void ServerMap::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
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
        case MPK_QUERYRMADDRESS:
            {
                On_MPK_QUERYRMADDRESS(rstMPK, rstFromAddr);
                break;
            }
        case MPK_REGIONMONITORREADY:
            {
                On_MPK_REGIONMONITORREADY(rstMPK, rstFromAddr);
                break;
            }
        // case MPK_ADDMONSTER:
        //     {
        //         On_MPK_ADDMONSTER(rstMPK, rstFromAddr);
        //         break;
        //     }
        // case MPK_NEWMONSTER:
        //     {
        //         On_MPK_NEWMONSTER(rstMPK, rstFromAddr);
        //         break;
        //     }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
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
                            || nGY >= (size_t)m_Mir2xMap.H()
                            || nGX >= (size_t)m_Mir2xMap.W()){ continue; }
                    if(false
                            || m_Mir2xMap.CanWalk(nGX, nGY, 0)
                            || m_Mir2xMap.CanWalk(nGX, nGY, 1)
                            || m_Mir2xMap.CanWalk(nGX, nGY, 2)
                            || m_Mir2xMap.CanWalk(nGX, nGY, 3)){
                        m_RegionMonitorRecordV2D[nRMY][nRMX].Need = true;
                        goto __LABEL_GOTO_SERVERMAP_CHECKREGIONMONITORNEED_DONE_1;
                    }
                }
            }
__LABEL_GOTO_SERVERMAP_CHECKREGIONMONITORNEED_DONE_1:;
        }
    }
}

bool ServerMap::CheckRegionMonitorReady()
{
    bool bReady = true;
    for(size_t nY = 0; nY < m_RegionMonitorRecordV2D.size(); ++nY){
        for(size_t nX = 0; nX < m_RegionMonitorRecordV2D[0].size(); ++nX){
            if(!m_RegionMonitorRecordV2D[nY][nX].Ready()){
                bReady = false;
                goto __LABEL_GOTO_SERVERMAP_CHECKREGIONMONITORREADY_DONE_1;
            }
        }
    }

__LABEL_GOTO_SERVERMAP_CHECKREGIONMONITORREADY_DONE_1:
    m_RegionMonitorReady = bReady;
    return bReady;
}

// this function can only invoke one time
void ServerMap::CreateRegionMonterV2D()
{
    if(m_RMV2DCreated){ return; }

    for(size_t nGY = 0; nGY < m_RegionMonitorRecordV2D.size(); ++nGY){
        for(size_t nGX = 0; nGX < m_RegionMonitorRecordV2D[0].size(); ++nGX){
            // 1. won't apply
            if(!m_RegionMonitorRecordV2D[nGY][nGX].Need){ continue; }

            // 2. won't need further initialization
            if(m_RegionMonitorRecordV2D[nGY][nGX].Ready()){ continue; }

            // ok now it applies and need further initialization
            
            // 3. clear the previous one
            delete m_RegionMonitorRecordV2D[nGY][nGX].Data;

            // 4. create the new one
            auto pNewMonitor = new RegionMonitor(GetAddress(),
                    m_MapID, nGX * m_RegionW, nGY * m_RegionH, m_RegionW, m_RegionH);

            m_RegionMonitorRecordV2D[nGY][nGX].Data       = pNewMonitor;
            m_RegionMonitorRecordV2D[nGY][nGX].Need       = true;
            m_RegionMonitorRecordV2D[nGY][nGX].Inform     = false;
            m_RegionMonitorRecordV2D[nGY][nGX].RMReady    = false;
            m_RegionMonitorRecordV2D[nGY][nGX].PodAddress = pNewMonitor->Activate();
        }
    }

    for(size_t nGY = 0; nGY < m_RegionMonitorRecordV2D.size(); ++nGY){
        for(size_t nGX = 0; nGX < m_RegionMonitorRecordV2D[0].size(); ++nGX){
            // 1. won't apply
            if(!m_RegionMonitorRecordV2D[nGY][nGX].Need){ continue; }

            // 2. no need for further initialization
            if(m_RegionMonitorRecordV2D[nGY][nGX].Ready()){ continue; }

            // 3. pending, half-inited
            if(m_RegionMonitorRecordV2D[nGY][nGX].Pending()){ continue; }

            // ok now we need to send neighbor list

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

            auto fnOnR = [this, nGX, nGY](const MessagePack &rstMPK, const Theron::Address &){
                if(rstMPK.Type() == MPK_OK && m_RegionMonitorRecordV2D[nGY][nGX].Pending()){
                    m_RegionMonitorRecordV2D[nGY][nGX].RMReady = true;
                    CheckRegionMonitorReady();
                    return;
                }

                // ooops we have errors
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "init region monitor failed");
                g_MonoServer->Restart();
            };

            m_ActorPod->Forward({MPK_NEIGHBOR, (const uint8_t *)&stStringAddress[0],
                    stStringAddress.size()}, m_RegionMonitorRecordV2D[nGY][nGX].PodAddress, fnOnR);
            m_RegionMonitorRecordV2D[nGY][nGX].Inform = true;
        }
    }

    m_RMV2DCreated = true;
}
