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

bool Dispatcher::forward(uint64_t uid, const ActorMsgBuf &msgBuf, uint32_t resp)
{
    fflassert(uid);
    if(g_serverArgParser->traceActorMessage){
        g_server->addLog(LOGTYPE_DEBUG, "Dispatcher -> (UID: %s, Type: %s, ID: 0, Resp: %llu)", uidf::getUIDString(uid).c_str(), mpkName(msgBuf.type()), to_llu(resp));
    }
    return g_actorPool->postMessage(uid, {msgBuf, 0, 0, resp});
}
