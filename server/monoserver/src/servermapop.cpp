/*
 * =====================================================================================
 *
 *       Filename: servermapop.cpp
 *        Created: 05/03/2016 20:21:32
 *  Last Modified: 06/11/2016 02:36:13
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

#include "monster.hpp"
#include "sysconst.hpp"
#include "actorpod.hpp"
#include "metronome.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"

void ServerMap::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    m_SCAddress = rstFromAddr;
    if(!m_Metronome){
        m_Metronome = new Metronome(100);
    }
    m_Metronome->Activate(m_ActorPod->GetAddress());
}

void ServerMap::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    for(auto &rstRecordV: m_RMRecordV2D){
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

    m_RMRecordV2D[stAMMR.LocY][stAMMR.LocX].RMReady = true;
    CheckRegionMonitorReady();
}

// void ServerMap::On_MPK_ADDMONSTER(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
// {
//     AMAddMonster stAMAM;
//     std::memcpy(&stAMAM, rstMPK.Data(), sizeof(stAMAM));
//
//     // point is outside map boundary
//     if(!ValidP(stAMAM.X, stAMAM.Y) && stAMAM.Strict){
//         extern MonoServer *g_MonoServer;
//         g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid monster adding request");
//         m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
//
//         return;
//     }
//
//     // ok point is inside map boundary
//     Monster *pNewMonster = new Monster(stAMAM.GUID, stAMAM.UID, stAMAM.AddTime);
//     AMNewMonster stAMNM;
//
//     stAMNM.X = stAMAM.X;
//     stAMNM.Y = stAMAM.Y;
//     stAMNM.R = stAMAM.R;
//
//     stAMNM.Data    = pNewMonster;
//     stAMNM.GUID    = stAMAM.GUID;
//     stAMNM.UID     = stAMAM.UID;
//     stAMNM.AddTime = stAMAM.AddTime;
//
//     if(!RegionMonitorReady()){
//         CreateRegionMonterV2D();
//
//         if(!RegionMonitorReady()){
//             extern MonoServer *g_MonoServer;
//             g_MonoServer->AddLog(LOGTYPE_WARNING, "create region monitors for server map failed");
//             g_MonoServer->Restart();
//         }
//     }
//
//     auto stAddr = RegionMonitorAddressP(stAMAM.X, stAMAM.Y);
//     if(stAddr == Theron::Address::Null()){
//         extern MonoServer *g_MonoServer;
//         g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid location for new monstor");
//         m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
//         delete pNewMonster;
//
//         return;
//     }
//
//     // ok valid RM, we are to try to add it inside
//     auto fnROP = [this, rstFromAddr, nMPKID = rstMPK.ID()](
//             const MessagePack &rstRMPK, const Theron::Address &){
//         switch(rstRMPK.Type()){
//             case MPK_OK:
//                 {
//                     m_ActorPod->Forward(MPK_OK, rstFromAddr, nMPKID);
//                     break;
//                 }
//             default:
//                 {
//                     m_ActorPod->Forward(MPK_ERROR, rstFromAddr, nMPKID);
//                     break;
//                 }
//         }
//     };
//     m_ActorPod->Forward({MPK_NEWMONSTER, stAMNM}, stAddr, fnROP);
// }

// void ServerMap::On_MPK_NEWMONSTER(const MessagePack &rstMPK, const Theron::Address &)
// {
//     AMNewMonster stAMNM;
//     std::memcpy(&stAMNM, rstMPK.Data(), sizeof(stAMNM)); 
//     // 1. create the monstor
//     auto pNewMonster = new Monster(stAMNM.GUID, stAMNM.UID, stAMNM.AddTime);
//     uint64_t nKey = ((uint64_t)stAMNM.UID << 32) + stAMNM.AddTime;
//
//     // 2. put it in the pool
//     m_CharObjectM[nKey] = pNewMonster;
//
//     // 3. add the pointer inside and forward this message to the monitor
//     stAMNM.Data = (void *)pNewMonster;
//     auto stAddr = RegionMonitorAddressP(stAMNM.X, stAMNM.Y);
//     m_ActorPod->Forward({MPK_NEWMONSTER, stAMNM}, stAddr);
// }
//

// query the RM address, respondse
// 1. none          didn't ask for response
// 2. MPK_ADDRESS   everything is ready
// 4. MPK_ERROR     failed
// 5. MPK_PENDING   the RM record is not ready yet, ask later
//
void ServerMap::On_MPK_QUERYRMADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    AMQueryRMAddress stAMQRMA;
    std::memcpy(&stAMQRMA, rstMPK.Data(), sizeof(stAMQRMA));

    // 1. check map id
    if(stAMQRMA.MapID != m_MapID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "query rm address with another map id, ask sc directly");
        g_MonoServer->Restart();
    }

    // 2. check boundary
    if(!((stAMQRMA.MapX >= 0 && stAMQRMA.MapX < W() * SYS_MAPGRIDXP) && (stAMQRMA.MapY >= 0 && stAMQRMA.MapY < H() * SYS_MAPGRIDYP))){
        // TODO: out of bounday, should I kill current process or just return ERROR???
        //       RM and CO has no idea of map size, so this check could happen...
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    int nRMX = stAMQRMA.MapX / SYS_MAPGRIDXP / m_RegionW;
    int nRMY = stAMQRMA.MapY / SYS_MAPGRIDYP / m_RegionH;

    if(!((nRMY >= 0 && nRMY < (int)m_RMRecordV2D.size()) && (nRMX >= 0 && nRMX < (int)m_RMRecordV2D[0].size()))){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    // 3. pending state
    if(!(RegionMonitorReady() || CheckRegionMonitorReady())){
        // TODO: why I didn't put it in the ctor?
        // when create the RM's in CreateRegionMonterV2D(), I need to access
        // m_ActorPod->GetAddress() and pass it as one parameter to RM, but
        // inside the ctor the ServerMap is not activated yet, it's activated
        // by ServiceCore outside
        //
        CreateRegionMonterV2D();
        m_ActorPod->Forward(MPK_PENDING, rstFromAddr, rstMPK.ID());
        return;
    }

    // 3. all set we can anwser this query
    if(!m_RMRecordV2D[nRMY][nRMX].Valid()){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    auto &rstRMAddr = m_RMRecordV2D[nRMY][nRMX].PodAddress;
    if(!rstRMAddr){
        m_ActorPod->Forward(MPK_ERROR, rstFromAddr, rstMPK.ID());
        return;
    }

    std::string szRMAddr = rstRMAddr.AsString();
    m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)szRMAddr.c_str(), 1 + szRMAddr.size()}, rstFromAddr, rstMPK.ID());
}

// I was tring to use a routing protocol to dispatch this state locally, then I found it's
// impossible for following condition:
//
//  +-+-+-+-+
//  |A|X| | |    in this map each grid is a RM, and A is isolated from B, but
//  +-+-+-+-+    A and B are in the same view region, means A and B should be
//  |X|X| | |    visible to each other, then any motion update of A should be
//  +-+-+-+-+    reported to B, however, with one-hop routing it's impossible
//  | | |B| |
//  +-+-+-+-+
//
//  so I decide to report motion state directly to ServerMap and this class will dispatch
//  the state update to proper RMs
void ServerMap::On_MPK_MOTIONSTATE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMMotionState stAMMS;
    std::memcpy(&stAMMS, rstMPK.Data(), sizeof(stAMMS));

    int nRMX0 = (stAMMS.X - SYS_MAPVISIBLEW) / m_RegionW;
    int nRMX1 = (stAMMS.X + SYS_MAPVISIBLEW) / m_RegionW;
    int nRMY0 = (stAMMS.Y - SYS_MAPVISIBLEH) / m_RegionH;
    int nRMY1 = (stAMMS.Y + SYS_MAPVISIBLEH) / m_RegionH;

    for(int nY = nRMY0; nY <= nRMY1; ++nY){
        for(int nX = nRMX0; nX < nRMX1; ++nX){
            if(true
                    && nY >= 0
                    && nY <  (int)m_RMRecordV2D.size()      // boundary condition
                    && nX >= 0
                    && nX <  (int)m_RMRecordV2D[0].size()
                    && m_RMRecordV2D[nY][nX].Valid()){      // RM should be valid
                m_ActorPod->Forward({MPK_MOTIONSTATE, stAMMS}, m_RMRecordV2D[nY][nX].PodAddress);
            }
        }
    }
}

void ServerMap::On_MPK_QUERYSCADDRESS(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    if(!m_SCAddress){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "activated server map works without SC address");
        g_MonoServer->Restart();
    }

    std::string szAddr = m_SCAddress.AsString();
    m_ActorPod->Forward({MPK_ADDRESS, (const uint8_t *)(szAddr.c_str()), 1 + szAddr.size()}, rstFromAddr, rstMPK.ID());
}

void ServerMap::On_MPK_UPDATECOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMUpdateCOInfo stAMUCOI;
    std::memcpy(&stAMUCOI, rstMPK.Data(), sizeof(stAMUCOI));

    // 1. argument check here
    if(stAMUCOI.UID && stAMUCOI.AddTime && stAMUCOI.MapID && stAMUCOI.SessionID && stAMUCOI.MapID == m_MapID
                && stAMUCOI.X >= 0 && stAMUCOI.X < W() * SYS_MAPGRIDXP && stAMUCOI.Y >= 0 && stAMUCOI.Y < H() * SYS_MAPGRIDYP){
        int nRMX = stAMUCOI.X / SYS_MAPGRIDXP / m_RegionW;
        int nRMY = stAMUCOI.Y / SYS_MAPGRIDYP / m_RegionH;

        int nMinRMX = nRMX - SYS_MAPVISIBLEW / SYS_MAPGRIDXP;
        int nMaxRMX = nRMX + SYS_MAPVISIBLEW / SYS_MAPGRIDXP;
        int nMinRMY = nRMY - SYS_MAPVISIBLEH / SYS_MAPGRIDYP;
        int nMaxRMY = nRMY + SYS_MAPVISIBLEH / SYS_MAPGRIDYP;

        for(int nCurrRMY = nMinRMY; nCurrRMY <= nMaxRMY; ++nCurrRMY){
            for(int nCurrRMX = nMinRMX; nCurrRMX <= nMaxRMX; ++nCurrRMX){
                if(nCurrRMY >= 0 && nCurrRMY < (int)m_RMRecordV2D.size() && nCurrRMX >= 0 && nCurrRMX < (int)m_RMRecordV2D[0].size()){
                    if(m_RMRecordV2D[nCurrRMY][nCurrRMX].Valid()){
                        m_ActorPod->Forward({MPK_UPDATECOINFO, stAMUCOI}, m_RMRecordV2D[nCurrRMY][nCurrRMX].PodAddress);
                    }
                }
            }
        }

        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid argument in message package");
    g_MonoServer->Restart();
}
