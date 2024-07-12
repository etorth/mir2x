#include <cinttypes>
#include "uidf.hpp"
#include "fflerror.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

ActorMsgPack SyncDriver::forward(uint64_t uid, const ActorMsgBuf &mb, uint32_t resp, uint32_t timeout)
{
    // don't use in actor thread
    // because we can use actor send message directly
    // and this blocks the actor thread caused the wait never finish

    fflassert(uid);
    fflassert(!g_actorPool->isActorThread());

    m_currID++;
    if(m_currID == 0){
        m_currID = 1;
    }

    if(g_serverArgParser->traceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (type: %s, ID: %llu, Resp: %llu)", uidf::getUIDString(UID()).c_str(), uidf::getUIDString(uid).c_str(), mpkName(mb.type()), to_llu(m_currID), to_llu(resp));
    }

    if(!g_actorPool->postMessage(uid, {mb, UID(), m_currID, resp})){
        AMBadActorPod amBAP;
        std::memset(&amBAP, 0, sizeof(amBAP));

        amBAP.Type    = mb.type();
        amBAP.from    = UID();
        amBAP.ID      = m_currID;
        amBAP.Respond = resp;

        return {ActorMsgBuf(AM_BADACTORPOD, amBAP), 0, 0, m_currID};
    }

    switch(m_receiver.wait(timeout)){
        case 0:
            {
                break;
            }
        case 1:
        default:
            {
                if(auto mpkList = m_receiver.pop(); mpkList.size()){
                    for(auto p = mpkList.begin(); p != mpkList.end(); ++p){
                        if(p->respID() == m_currID){
                            return *p;
                        }
                    }
                }
            }
    }
    return {AM_NONE};
}
