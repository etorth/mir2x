/*
 * =====================================================================================
 *
 *       Filename: dispatcher.cpp
 *        Created: 01/26/2018 15:37:14
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
#include "dispatcher.hpp"
#include "messagepack.hpp"

void Dispatcher::Forward(const MessageBuf &rstMB, uint32_t nUID, uint32_t nRespond)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "Dispatcher -> (UID: %s, Type: %s, ID: 0, Resp: %" PRIu32 ")", UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    if(!nUID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "Dispatcher -> (UID: %s, Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to UID 0", UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
        return;
    }

    extern ActorPool *g_ActorPool;
    g_ActorPool->PostMessage(nUID, {rstMB, 0, nRespond});
}
