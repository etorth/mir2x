/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 06/11/2016 00:07:59
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
    stAMLOK.MapID     = m_MapID;
    stAMLOK.Direction = m_Direction;
    g_NetPodN->Send(m_SessionID, SM_LOGINOK, stAMLOK);
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
