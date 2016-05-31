/*
 * =====================================================================================
 *
 *       Filename: playerop.cpp
 *        Created: 05/11/2016 17:37:54
 *  Last Modified: 05/30/2016 18:09:33
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

void Player::On_MPK_BINDSESSION(const MessagePack &rstMPK, const Theron::Address &)
{
    Bind(*((uint32_t *)rstMPK.Data()));
    extern NetPodN *g_NetPodN;

    SMLoginOK stAMLOK;
    stAMLOK.GUID      = m_GUID;
    stAMLOK.JobID     = m_JobID;
    stAMLOK.Level     = m_Level;
    stAMLOK.MapX      = m_CurrX;
    stAMLOK.MapY      = m_CurrY;
    stAMLOK.Direction = m_Direction;
    g_NetPodN->Send(m_SessionID, SM_LOGINOK, stAMLOK);
}

void Player::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
{
    m_RMAddress = rstFromAddr;
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
