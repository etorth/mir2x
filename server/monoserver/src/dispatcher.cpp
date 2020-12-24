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
#include "uidf.hpp"
#include "serverargparser.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "messagepack.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

bool Dispatcher::forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond)
{
    if(g_serverArgParser->traceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "Dispatcher -> (UID: %s, Type: %s, ID: 0, Resp: %llu)", uidf::getUIDString(nUID).c_str(), mpkName(rstMB.Type()), to_llu(nRespond));
    }

    if(!nUID){
        g_monoServer->addLog(LOGTYPE_DEBUG, "Dispatcher -> (UID: %s, Type: %s, ID: 0, Resp: %llu): Try to send message to UID 0", uidf::getUIDString(nUID).c_str(), mpkName(rstMB.Type()), to_llu(nRespond));
        return false;
    }
    return g_actorPool->postMessage(nUID, {rstMB, 0, 0, nRespond});
}
