/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 03/22/2017 18:52:04
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
    auto pMem = new SMLoginOK();

    pMem->GUID      = m_GUID;
    pMem->JobID     = m_JobID;
    pMem->Level     = m_Level;
    pMem->X         = m_CurrX;
    pMem->Y         = m_CurrY;
    pMem->MapID     = m_Map->ID();
    pMem->Direction = m_Direction;

    g_NetPodN->Send(m_SessionID, SM_LOGINOK, (uint8_t *)pMem, sizeof(SMLoginOK), [pMem](){ delete pMem; });

    if(ActorPodValid() && m_Map->ActorPodValid()){
        AMUpdateCOInfo stAMUCOI;
        stAMUCOI.UID       = UID();
        stAMUCOI.AddTime   = AddTime();
        stAMUCOI.X         = X();
        stAMUCOI.Y         = Y();
        stAMUCOI.MapID     = m_Map->ID();
        stAMUCOI.SessionID = m_SessionID;

        m_ActorPod->Forward({MPK_UPDATECOINFO, stAMUCOI}, m_Map->GetAddress());
    }
}

void Player::On_MPK_HI(const MessagePack &, const Theron::Address &)
{
}

void Player::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
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

    // prepare the server message to send
    extern MemoryPN *g_MemoryPN;
    auto pMem = (SMActionState *)g_MemoryPN->Get(sizeof(SMActionState));

    pMem->UID     = stAMAS.UID;
    pMem->AddTime = stAMAS.AddTime;

    pMem->X     = stAMAS.X;
    pMem->Y     = stAMAS.Y;
    pMem->R     = stAMAS.R;
    pMem->MapID = stAMAS.MapID;

    pMem->Action    = stAMAS.Action;
    pMem->Direction = stAMAS.Direction;

    pMem->Speed  = stAMAS.Speed;

    if(std::abs(m_CurrX - stAMAS.X) <= SYS_MAPVISIBLEW && std::abs(m_CurrY - stAMAS.Y) <= SYS_MAPVISIBLEH){
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
