/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 04/07/2017 11:39:46
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
    extern NetPodN *g_NetPodN;
    extern MemoryPN *g_MemoryPN;

    auto pMem = g_MemoryPN->Get<SMLoginOK>();
    pMem->DBID      = m_DBID;
    pMem->JobID     = m_JobID;
    pMem->Level     = m_Level;
    pMem->X         = m_CurrX;
    pMem->Y         = m_CurrY;
    pMem->MapID     = m_Map->ID();
    pMem->Direction = m_Direction;

    g_NetPodN->Send(m_SessionID, SM_LOGINOK, (uint8_t *)(pMem), sizeof(SMLoginOK), [pMem](){ g_MemoryPN->Free(pMem); });

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
        g_MemoryPN->Free(stAMNP.Data);
    }
}

void Player::On_MPK_ACTION(const MessagePack &rstMPK, const Theron::Address &)
{
    AMAction stAMA;
    std::memcpy(&stAMA, rstMPK.Data(), sizeof(stAMA));

    if((std::abs(stAMA.X - m_CurrX) <= SYS_MAPVISIBLEW) && (std::abs(stAMA.Y - m_CurrY) <= SYS_MAPVISIBLEH)){
        extern MemoryPN *g_MemoryPN;
        auto pMem = g_MemoryPN->Get<SMAction>();

        pMem->UID   = stAMA.UID;
        pMem->X     = stAMA.X;
        pMem->Y     = stAMA.Y;
        pMem->MapID = stAMA.MapID;

        pMem->Action      = stAMA.Action;
        pMem->ActionParam = stAMA.ActionParam;
        pMem->Speed       = stAMA.Speed;
        pMem->Direction   = stAMA.Direction;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(m_SessionID, SM_ACTION, (uint8_t *)pMem, sizeof(SMAction), [pMem](){ g_MemoryPN->Free(pMem); });
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
