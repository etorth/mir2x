/*
 * =====================================================================================
 *
 *       Filename: actorpod.cpp
 *        Created: 05/03/2016 15:00:35
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
#include <cstdio>
#include <atomic>
#include <cinttypes>

#include "uidf.hpp"
#include "totype.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "raiitimer.hpp"
#include "monoserver.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

ActorPod::ActorPod(uint64_t nUID,
        const std::function<void()> &fnTrigger,
        const std::function<void(const MessagePack &)> &fnOperation, uint32_t nExpireTime)
    : m_UID([nUID]() -> uint64_t
      {
          if(!nUID){
              throw fflerror("provide user-defined zero UID");
          }

          if(nUID & 0X00FF000000000000ULL){
              throw fflerror("provide user-defined UID has non-zero bits at 0X00FF000000000000: 0x%016llu", to_llu(nUID));
          }
          return nUID;
      }())
    , m_trigger(fnTrigger)
    , m_operation(fnOperation)
    , m_validID(0)
    , m_expireTime(nExpireTime)
    , m_respondHandlerGroup()
{
    g_actorPool->attach(this);
}

ActorPod::~ActorPod()
{
    // don't call destructor in running actor thread
    // could be its actor thread or the stealing actor thread

    // we can't check it here...
    // in ActorPod it shouldn't be aware who is calling itself

    try{
        g_actorPool->detach(this, nullptr);
    }
    catch(...){
        g_monoServer->propagateException();
    }
}

void ActorPod::innHandler(const MessagePack &rstMPK)
{
    if(g_serverArgParser->TraceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(rstMPK.from()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    if(m_expireTime){
        while(!m_respondHandlerGroup.empty()){
            if(g_monoServer->getCurrTick() >= m_respondHandlerGroup.begin()->second.ExpireTime){
                // everytime when we received the new MPK we check if there is handler the timeout
                // also this time get counted into the monitor entry
                m_podMonitor.amProcMonitorList[MPK_TIMEOUT].recvCount++;
                {
                    raii_timer stTimer(&(m_podMonitor.amProcMonitorList[MPK_TIMEOUT].procTick));
                    m_respondHandlerGroup.begin()->second.Operation(MPK_TIMEOUT);
                }
                m_respondHandlerGroup.erase(m_respondHandlerGroup.begin());
                continue;
            }

            // std::map<ID, Handler> keeps order in ID number
            // ID number is the Resp() of the responding messages
            //
            // and we guarantee "ID1 < ID2" => "ExpireTime1 <= ExpireTime2"
            // so if we get first non-expired handler, means the rest are all not expired
            break;
        }
    }

    if(rstMPK.Respond()){
        // try to find the response handler for current responding message
        // 1.     find it, good
        // 2. not find it: 1. didn't register for it, we must prevent this at sending
        //                 2. repsonse is too late ooops and the handler has already be deleted
        if(auto p = m_respondHandlerGroup.find(rstMPK.Respond()); p != m_respondHandlerGroup.end()){
            if(p->second.Operation){
                m_podMonitor.amProcMonitorList[rstMPK.Type()].recvCount++;
                {
                    raii_timer stTimer(&(m_podMonitor.amProcMonitorList[rstMPK.Type()].procTick));
                    p->second.Operation(rstMPK);
                }
            }else{
                throw fflerror("%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Response handler not executable",
                        uidf::getUIDString(UID()).c_str(), uidf::getUIDString(rstMPK.from()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            }
            m_respondHandlerGroup.erase(p);
        }else{
            // should only caused by deletion of timeout
            // do nothing for this case, don't take this as an error
        }
    }else{
        // this is not a responding message
        // use default message handling operation
        if(m_operation){
            m_podMonitor.amProcMonitorList[rstMPK.Type()].recvCount++;
            {
                raii_timer stTimer(&(m_podMonitor.amProcMonitorList[rstMPK.Type()].procTick));
                m_operation(rstMPK);
            }
        }else{
            // shoud I make it fatal?
            // we may get a lot warning message here
            throw fflerror("%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Message handler not executable",
                    uidf::getUIDString(UID()).c_str(), uidf::getUIDString(rstMPK.from()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }
    }

    // trigger is for all types of messages
    // currently we don't take trigger time into consideration

    if(m_trigger){
        raii_timer stTimer(&(m_podMonitor.triggerMonitor.procTick));
        m_trigger();
    }
}

uint32_t ActorPod::GetValidID()
{
    // previously I use g_serverArgParser->TraceActorMessage to select use this one
    // this is dangerous since if we can change g_serverArgParser->TraceActorMessage during runtime then it's dead
    if(1){
        static std::atomic<uint32_t> s_ValidID(1);
        auto nNewValidID = s_ValidID.fetch_add(1);

        if(nNewValidID){
            return nNewValidID;
        }
        throw fflerror("running out of message ID, exiting...");
    }

    m_validID = (m_respondHandlerGroup.empty() ? 1 : (m_validID + 1));
    if(auto p = m_respondHandlerGroup.find(m_validID); p == m_respondHandlerGroup.end()){
        return m_validID;
    }
    throw fflerror("running out of message ID, exiting...");
}

bool ActorPod::forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond)
{
    if(!nUID){
        throw fflerror("%s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                uidf::getUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    if(nUID == UID()){
        throw fflerror("%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    if(!rstMB){
        throw fflerror("%s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), nRespond);
    }

    if(g_serverArgParser->TraceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    m_podMonitor.amProcMonitorList[rstMB.Type()].sendCount++;
    return g_actorPool->postMessage(nUID, {rstMB, UID(), 0, nRespond});
}

bool ActorPod::forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, std::function<void(const MessagePack &)> fnOPR)
{
    if(!nUID){
        throw fflerror("%s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                uidf::getUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    if(nUID == UID()){
        throw fflerror("%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    if(!rstMB){
        throw fflerror("%s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), nRespond);
    }

    if(!fnOPR){
        throw fflerror("%s -> %s: (Type: %s, ID: NA, Resp: %" PRIu32 "): Response handler not executable",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    auto nID = GetValidID();
    if(g_serverArgParser->TraceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: %u, Resp: %u)",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }

    m_podMonitor.amProcMonitorList[rstMB.Type()].sendCount++;
    if(g_actorPool->postMessage(nUID, {rstMB, UID(), nID, nRespond})){
        if(m_respondHandlerGroup.try_emplace(nID, g_monoServer->getCurrTick() + m_expireTime, std::move(fnOPR)).second){
            return true;
        }
        throw fflerror("failed to register response handler for posted message: %s", MessagePack(rstMB.Type()).Name());
    }else{
        // respond the response handler here
        // if post failed, it can only be the UID is detached
        if(fnOPR){
            m_podMonitor.amProcMonitorList[MPK_BADACTORPOD].recvCount++;
            {
                raii_timer stTimer(&(m_podMonitor.amProcMonitorList[MPK_BADACTORPOD].procTick));
                fnOPR(MPK_BADACTORPOD);
            }
        }
        return false;
    }
}

void ActorPod::detach(const std::function<void()> &fnAtExit) const
{
    // we can call detach in its message handler
    // remember the message handler can be executed by worker thread or stealing worker thread
    // we can't guarantee that after detach no thread can access the actor
    //
    //  void MessageHandler()
    //  {
    //      if(bNeedDetach){
    //          detach();
    //          OtherRelease(); // here still accessing the actor
    //          return;         // after this line we can guarantee actor is not running
    //      }
    //  }
    //
    // it's worse to call it out of its message handler
    // when detach returns we can't guarantee if the actor is till handling message
    //
    g_actorPool->detach(this, fnAtExit);
}

bool ActorPod::checkUIDValid(uint64_t uid)
{
    if(!uid){
        throw fflerror("invalid zero UID");
    }
    return g_actorPool->checkUIDValid(uid);
}

void ActorPod::PrintMonitor() const
{
    for(size_t nIndex = 0; nIndex < m_podMonitor.amProcMonitorList.size(); ++nIndex){
        const uint64_t nProcTick  = m_podMonitor.amProcMonitorList[nIndex].procTick / 1000000;
        const uint32_t nSendCount = m_podMonitor.amProcMonitorList[nIndex].sendCount;
        const uint32_t nRecvCount = m_podMonitor.amProcMonitorList[nIndex].recvCount;
        if(nSendCount || nRecvCount){
            g_monoServer->addLog(LOGTYPE_DEBUG, "UID: %s %s: procTick %llu ms, sendCount %llu, recvCount %llu", uidf::getUIDString(UID()).c_str(), MessagePack(nIndex).Name(), to_llu(nProcTick), to_llu(nSendCount), to_llu(nRecvCount));
        }
    }
}
