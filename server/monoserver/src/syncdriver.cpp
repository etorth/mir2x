/*
 * =====================================================================================
 *
 *       Filename: syncdriver.cpp
 *        Created: 06/09/2016 17:32:50
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

#include <cinttypes>
#include "serverenv.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"

// send without waiting for response
// also for SyncDriver we have no registed response handler
//
// return value:
//      0. no error
//      1. send failed
int SyncDriver::Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "(Driver: 0X%0*" PRIXPTR ", Name: SyncDriver, UID: NA) -> (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                (int)(sizeof(this) * 2), (uintptr_t)(this), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    if(!rstAddr){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(Driver: 0X%0*" PRIXPTR ", Name: SyncDriver, UID: NA) -> (Type: %s, ID: 0, Resp: %" PRIu32 ") : Try to send message to an emtpy address",
                (int)(sizeof(this) * 2), (uintptr_t)(this), MessagePack(rstMB.Type()).Name(), nRespond);
        return 1;
    }

    if(rstAddr == GetAddress()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(Driver: 0X%0*" PRIXPTR ", Name: SyncDriver, UID: NA) -> (Type: %s, ID: 0, Resp: %" PRIu32 ") : Try to send message to itself",
                (int)(sizeof(this) * 2), (uintptr_t)(this), MessagePack(rstMB.Type()).Name(), nRespond);
        return 1;
    }

    extern Theron::Framework *g_Framework;
    return g_Framework->Send<MessagePack>({rstMB, 0, nRespond}, m_Receiver.GetAddress(), rstAddr) ? 0 : 1;
}

MessagePack SyncDriver::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, uint32_t nTimeout)
{
    if(!nUID){
        extern ServerEnv *g_ServerEnv;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Sending message to UID 0");
        return {MPK_NONE};
    }

    extern ActorPool *g_ActorPool;
    if(g_ActorPool->IsActorThread()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Calling SyncDriver::Forward() in actor thread, SyncDriver = %p, SyncDriver::UID() = %" PRIu64, this, UID());
        return {MPK_NONE};
    }

    m_CurrID = (m_CurrID + 1) ? (m_CurrID + 1) : 1;
    auto nCurrID = m_CurrID;

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")", UIDFunc::GetUIDName(UID()), UIDFunc::GetUIDName(nUID), nCurrID, nRespond);
    }

    g_ActorPool->PostMessage(nUID, {rstMB, UID(), nCurrID, nRespond});
    switch(m_Receiver.Wait(nTimeout)){
        case 0:
            {
                break;
            }
        case 1:
        default:
            {
                if(auto stvMPK = m_Receiver.Pop(); stvMPK.size()){
                    for(auto p = stvMPK.begin(); p != stvMPK.end(); ++p){
                        if(p->Resp() == nCurrID){
                            return *p;
                        }
                    }
                }
            }
    }
    return {MPK_NONE};
}
