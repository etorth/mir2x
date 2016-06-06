/*
 * =====================================================================================
 *
 *       Filename: monsterop.cpp
 *        Created: 05/03/2016 21:49:38
 *  Last Modified: 06/05/2016 22:29:21
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
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "monoserver.hpp"

void Monster::On_MPK_HI(const MessagePack &, const Theron::Address &rstFromAddr)
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
            g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid map id for activated monster");
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

void Monster::On_MPK_METRONOME(const MessagePack &, const Theron::Address &)
{
    Update();
}
