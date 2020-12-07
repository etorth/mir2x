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
#include "uidf.hpp"
#include "serverargparser.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"

MessagePack SyncDriver::forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, uint32_t nTimeout)
{
    if(!nUID){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Sending message to UID 0");
        return {MPK_NONE};
    }

    // don't use in actor thread
    // because we can use actor send message directly
    // and this blocks the actor thread caused the wait never finish

    extern ActorPool *g_actorPool;
    if(g_actorPool->isActorThread()){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_FATAL, "Calling SyncDriver::forward() in actor thread, SyncDriver = %p, SyncDriver::UID() = %" PRIu64, this, UID());
        return {MPK_NONE};
    }

    m_currID = (m_currID + 1) ? (m_currID + 1) : 1;
    auto nCurrID = m_currID;

    extern ServerArgParser *g_serverArgParser;
    if(g_serverArgParser->TraceActorMessage){
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")", uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), nCurrID, nRespond);
    }

    if(!g_actorPool->postMessage(nUID, {rstMB, UID(), nCurrID, nRespond})){
        AMBadActorPod stAMBAP;
        std::memset(&stAMBAP, 0, sizeof(stAMBAP));

        stAMBAP.Type    = rstMB.Type();
        stAMBAP.from    = UID();
        stAMBAP.ID      = nCurrID;
        stAMBAP.Respond = nRespond;

        return {MessageBuf(MPK_BADACTORPOD, stAMBAP), 0, 0, nCurrID};
    }

    switch(m_receiver.Wait(nTimeout)){
        case 0:
            {
                break;
            }
        case 1:
        default:
            {
                if(auto stvMPK = m_receiver.Pop(); stvMPK.size()){
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
