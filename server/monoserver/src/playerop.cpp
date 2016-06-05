/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 06/05/2016 14:59:09
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

void Player::On_MPK_BINDSESSION(const MessagePack &rstMPK, const Theron::Address &)
{
    Bind(*((uint32_t *)rstMPK.Data()));
    extern NetPodN *g_NetPodN;

    SMLoginOK stAMLOK;
    stAMLOK.GUID      = m_GUID;
    stAMLOK.JobID     = m_JobID;
    stAMLOK.Level     = m_Level;
    stAMLOK.X         = m_CurrX;
    stAMLOK.Y         = m_CurrY;
    stAMLOK.Direction = m_Direction;
    g_NetPodN->Send(m_SessionID, SM_LOGINOK, stAMLOK);
}

void Player::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    // 1. set the RM address, it's the source of information
    m_RMAddress = rstFromAddr;

    // 2. try to set SC address
    if(!m_SCAddress){
        auto fnOnR = [this](const MessagePack &rstMPK, const Theron::Address &){
            if(rstMPK.Type() == MPK_ADDRESS){
                m_SCAddress = Theron::Address((char *)(rstMPK.Data()));
                m_SCAddressQuery = QUERY_OK;
                return;
            }

            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "failed to get SC address from RM");
            g_MonoServer->Restart();
        };

        m_SCAddressQuery = QUERY_PENDING;
        m_ActorPod->Forward(MPK_QUERYSCADDRESS, m_RMAddress, fnOnR);
    }

    // 3. try to set map address
    if(!m_MapAddress){
        if(!m_MapID){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid map id for activated player");
            g_MonoServer->Restart();
        }

        auto fnOnR = [this](const MessagePack &rstMPK, const Theron::Address &){
            if(rstMPK.Type() == MPK_ADDRESS){
                m_MapAddress = Theron::Address((char *)(rstMPK.Data()));
                m_MapAddressQuery = QUERY_OK;
                return;
            }

            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "failed to get map address from RM");
            g_MonoServer->Restart();
        };
        m_ActorPod->Forward({MPK_QUERYMAPADDRESS, m_MapID}, m_RMAddress, fnOnR);
    }
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

void Player::On_MPK_MOTIONSTATE(const MessagePack &rstMPK, const Theron::Address &)
{
    AMMotionState stAMMS;
    std::memcpy(&stAMMS, rstMPK.Data(), sizeof(stAMMS));

    // prepare the server message to send
    SMMotionState stSMMS;
    stSMMS.Type    = stAMMS.Type;
    stSMMS.UID     = stAMMS.UID;
    stSMMS.AddTime = stAMMS.AddTime;
    stSMMS.State   = stAMMS.State;
    stSMMS.Speed   = stAMMS.Speed;
    stSMMS.X       = stAMMS.X;
    stSMMS.Y       = stAMMS.Y;

    if(true
            && std::abs(m_CurrX - stAMMS.X) <= SYS_MAPVISIBLEW
            && std::abs(m_CurrY - stAMMS.Y) <= SYS_MAPVISIBLEH){
        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(m_SessionID, SM_MOTIONSTATE, stSMMS);
    }
}
