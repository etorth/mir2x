/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 03/30/2017 14:44:44
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

void Player::On_MPK_BINDSESSION(const MessagePack &rstMPK, const Theron::Address &)
{
    Bind(*((uint32_t *)rstMPK.Data()));
    extern NetPodN *g_NetPodN;
    extern MemoryPN *g_MemoryPN;

    auto pMem = g_MemoryPN->Get<SMLoginOK>();
    pMem->GUID      = m_GUID;
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

void Player::On_MPK_HI(const MessagePack &, const Theron::Address &)
{
}

void Player::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    extern NetPodN *g_NetPodN;
    extern MemoryPN *g_MemoryPN;
    extern MonoServer *g_MonoServer;

    auto pMem = g_MemoryPN->Get<SMPing>();
    pMem->Tick = g_MonoServer->GetTimeTick();

    g_NetPodN->Send(m_SessionID, SM_PING, (uint8_t *)(pMem), sizeof(SMPing), [pMem](){ g_MemoryPN->Free(pMem); });
    // AMPullCOInfo stAMPCOI;
    // stAMPCOI.SessionID = m_SessionID;

    // m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, m_Map->GetAddress());
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

void Player::On_MPK_ACTIONSTATE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMActionState stAMAS;
    std::memcpy(&stAMAS, rstMPK.Data(), sizeof(stAMAS));

    if((std::abs(stAMAS.X - m_CurrX) <= SYS_MAPVISIBLEW) && (std::abs(stAMAS.Y - m_CurrY) <= SYS_MAPVISIBLEH)){
        extern MemoryPN *g_MemoryPN;
        auto pMem = g_MemoryPN->Get<SMActionState>();

        pMem->UID   = stAMAS.UID;
        pMem->X     = stAMAS.X;
        pMem->Y     = stAMAS.Y;
        pMem->MapID = stAMAS.MapID;

        pMem->Speed     = stAMAS.Speed;
        pMem->Action    = stAMAS.Action;
        pMem->Direction = stAMAS.Direction;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(m_SessionID, SM_ACTIONSTATE, (uint8_t *)pMem, sizeof(SMActionState), [pMem](){ g_MemoryPN->Free(pMem); });
    }
}

void Player::On_MPK_UPDATECOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMUpdateCOInfo stAMUCOI;
    std::memcpy(&stAMUCOI, rstMPK.Data(), sizeof(stAMUCOI));
    ReportCORecord(stAMUCOI.SessionID);
}

void Player::On_MPK_PULLCOINFO(const MessagePack &rstMPK, const Theron::Address &)
{
    AMPullCOInfo stAMPCOI;
    std::memcpy(&stAMPCOI, rstMPK.Data(), sizeof(stAMPCOI));
    if(stAMPCOI.SessionID != m_SessionID){
        ReportCORecord(stAMPCOI.SessionID);
    }
}
