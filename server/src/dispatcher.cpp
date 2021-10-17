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
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "dispatcher.hpp"
#include "actormsgpack.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

bool Dispatcher::forward(uint64_t uid, const ActorMsgBuf &msgBuf, uint32_t resp)
{
    fflassert(uid);
    if(g_serverArgParser->traceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "Dispatcher -> (UID: %s, Type: %s, ID: 0, Resp: %llu)", uidf::getUIDString(uid).c_str(), mpkName(msgBuf.type()), to_llu(resp));
    }
    return g_actorPool->postMessage(uid, {msgBuf, 0, 0, resp});
}
