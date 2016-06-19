/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 06/19/2016 11:53:32
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

    auto pMem = (SMLoginOK *)g_MemoryPN->Get(sizeof(SMLoginOK));

    pMem->GUID      = m_GUID;
    pMem->JobID     = m_JobID;
    pMem->Level     = m_Level;
    pMem->X         = m_CurrX;
    pMem->Y         = m_CurrY;
    pMem->MapID     = m_MapID;
    pMem->Direction = m_Direction;

    g_NetPodN->Send(m_SessionID, SM_LOGINOK, (uint8_t *)pMem, sizeof(SMLoginOK), [pMem](){ g_MemoryPN->Free(pMem); });

    // install a trigger to collect neighbors every 1s
    auto fnDoUpdate = [this](){
        if(QueryMapAddress() == QUERY_OK){
            if(m_MapAddress){
                AMUpdateCOInfo stAMUCOI;
                stAMUCOI.UID       = UID();
                stAMUCOI.AddTime   = AddTime();
                stAMUCOI.X         = X();
                stAMUCOI.Y         = Y();
                stAMUCOI.MapID     = m_MapID;
                stAMUCOI.SessionID = m_SessionID;

                m_ActorPod->Forward({MPK_UPDATECOINFO, stAMUCOI}, m_MapAddress);
                return;
            }
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "unexpected internal logic error");
            g_MonoServer->Restart();
        }
    };

    extern MonoServer *g_MonoServer;
    uint32_t nLastUpdateTick = g_MonoServer->GetTimeTick();
    auto fnUpdateNeighbor = [this, nLastUpdateTick, fnDoUpdate]() mutable -> bool{
        // 1. check time and do update
        uint32_t nCurrentTick = g_MonoServer->GetTimeTick();
        if(nCurrentTick - nLastUpdateTick >= 2000){
            nLastUpdateTick = nCurrentTick;
            fnDoUpdate();
        }

        // 2. never return true
        return false;
    };

    m_StateHook.Install(fnUpdateNeighbor);
}

void Player::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    m_RMAddress = rstFromAddr;

    QuerySCAddress();
    QueryMapAddress();
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

    pMem->Action = stAMAS.Action;
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
    if(stAMUCOI.UID && stAMUCOI.AddTime && stAMUCOI.MapID && stAMUCOI.SessionID
            && stAMUCOI.MapID == m_MapID && stAMUCOI.X >= 0 && stAMUCOI.Y >= 0 && stAMUCOI.UID != UID() && stAMUCOI.AddTime != AddTime()){
        ReportCORecord(stAMUCOI.SessionID);
    }
}
