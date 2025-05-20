#include <cinttypes>
#include "uidf.hpp"
#include "actorpool.hpp"
#include "server.hpp"
#include "dispatcher.hpp"
#include "actormsgpack.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern Server *g_server;
extern ServerArgParser *g_serverArgParser;

bool Dispatcher::post(uint64_t uid, const ActorMsgBuf &msgBuf, uint64_t resp)
{
    fflassert(uid);
    if(g_serverArgParser->sharedConfig().traceActorMessage){
        g_server->addLog(LOGTYPE_INFO, "DISPATCHER -> %s", to_cstr(ActorMsgPack(msgBuf, 0, 0, resp).str(uid)));
    }
    return g_actorPool->postMessage(uid, {msgBuf, 0, 0, resp});
}
