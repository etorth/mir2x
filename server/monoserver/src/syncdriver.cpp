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
#include "uidfunc.hpp"
#include "serverenv.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"

MessagePack SyncDriver::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, uint32_t nTimeout)
{
    if(!nUID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Sending message to UID 0");
        return {MPK_NONE};
    }

    // don't use in actor thread
    // because we can use actor send message directly!
    // and this blocks the actor thread caused the wait never finish

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
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")", UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nCurrID, nRespond);
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
                        if(p->Respond() == nCurrID){
                            return *p;
                        }
                    }
                }
            }
    }
    return {MPK_NONE};
}
