/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 04/27/2017 15:20:10
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

#include "netpod.hpp"
#include "player.hpp"
#include "memorypn.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"

void Player::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    extern NetPodN *g_NetPodN;
    extern MemoryPN *g_MemoryPN;
    extern MonoServer *g_MonoServer;

    auto pMem = g_MemoryPN->Get<SMPing>();
    pMem->Tick = g_MonoServer->GetTimeTick();

    g_NetPodN->Send(m_SessionID, SM_PING, (uint8_t *)(pMem), sizeof(SMPing), [pMem](){ g_MemoryPN->Free(pMem); });
}

void Player::On_MPK_BINDSESSION(const MessagePack &rstMPK, const Theron::Address &)
{
    Bind(*((uint32_t *)rstMPK.Data()));

    SMLoginOK stSMLOK;
    stSMLOK.UID       = UID();
    stSMLOK.DBID      = DBID();
    stSMLOK.MapID     = m_Map->ID();
    stSMLOK.X         = m_CurrX;
    stSMLOK.Y         = m_CurrY;
    stSMLOK.Male      = true;
    stSMLOK.Direction = m_Direction;
    stSMLOK.JobID     = m_JobID;
    stSMLOK.Level     = m_Level;

    extern NetPodN *g_NetPodN;
    g_NetPodN->Send(m_SessionID, SM_LOGINOK, stSMLOK);

    if(ActorPodValid() && m_Map->ActorPodValid()){
        AMPullCOInfo stAMPCOI;
        stAMPCOI.SessionID = m_SessionID;
        m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, m_Map->GetAddress());
    }
}

void Player::On_MPK_NETPACKAGE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMNetPackage stAMNP;
    std::memcpy(&stAMNP, rstMPK.Data(), sizeof(AMNetPackage));

    OperateNet(stAMNP.Type, stAMNP.Data, stAMNP.DataLen);

    if(stAMNP.Data){
        extern MemoryPN *g_MemoryPN;
        g_MemoryPN->Free(const_cast<uint8_t *>(stAMNP.Data));
    }
}

void Player::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if((std::abs(stAMA.X - m_CurrX) <= SYS_MAPVISIBLEW) && (std::abs(stAMA.Y - m_CurrY) <= SYS_MAPVISIBLEH)){
        SMAction stSMA;

        stSMA.UID   = stAMA.UID;
        stSMA.MapID = stAMA.MapID;

        stSMA.Action      = stAMA.Action;
        stSMA.ActionParam = stAMA.ActionParam;
        stSMA.Speed       = stAMA.Speed;
        stSMA.Direction   = stAMA.Direction;

        stSMA.X    = stAMA.X;
        stSMA.Y    = stAMA.Y;
        stSMA.EndX = stAMA.EndX;
        stSMA.EndY = stAMA.EndY;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(m_SessionID, SM_ACTION, stSMA);
    }
}

void Player::On_MPK_PULLCOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));
    if(stAMPCOI.SessionID != m_SessionID){
        ReportCORecord(stAMPCOI.SessionID);
    }
}
