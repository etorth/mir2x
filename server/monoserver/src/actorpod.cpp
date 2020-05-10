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
              throw std::runtime_error("Provide user-defined zero UID");
          }

          if(nUID & 0XFFFF000000000000){
              throw std::runtime_error(str_fflprintf("Provide user-defined UID greater than 0XFFFF000000000000: %" PRIu64, nUID));
          }
          return nUID;
      }())
    , m_trigger(fnTrigger)
    , m_operation(fnOperation)
    , m_validID(0)
    , m_expireTime(nExpireTime)
    , m_respondHandlerGroup()
    , m_podMonitor()
{
    if(!g_actorPool->Register(this)){
        throw std::runtime_error(str_fflprintf("Register actor failed: ActorPod = %p, ActorPod::UID() = %" PRIu64, this, UID()));
    }
}

ActorPod::~ActorPod()
{
    // don't call destructor in running actor thread
    // could be its actor thread or the stealing actor thread

    // we can't check it here...
    // in ActorPod it shouldn't be aware who is calling itself

    // Detach(this, true) will report error if called in running actor thread
    // good enough

    if(!g_actorPool->Detach(this, [](){})){
        g_monoServer->addLog(LOGTYPE_WARNING, "ActorPool::Detach(ActorPod = %p) failed", this);
        g_monoServer->Restart();
    }
}

void ActorPod::InnHandler(const MessagePack &rstMPK)
{
    if(g_serverArgParser->TraceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    if(m_expireTime){
        while(!m_respondHandlerGroup.empty()){
            if(g_monoServer->getCurrTick() >= m_respondHandlerGroup.begin()->second.ExpireTime){
                // everytime when we received the new MPK we check if there is handler the timeout
                // also this time get counted into the monitor entry
                m_podMonitor.AMProcMonitorList[MPK_TIMEOUT].RecvCount++;
                {
                    raii_timer stTimer(&(m_podMonitor.AMProcMonitorList[MPK_TIMEOUT].ProcTick));
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
                m_podMonitor.AMProcMonitorList[rstMPK.Type()].RecvCount++;
                {
                    raii_timer stTimer(&(m_podMonitor.AMProcMonitorList[rstMPK.Type()].ProcTick));
                    p->second.Operation(rstMPK);
                }
            }else{
                throw std::runtime_error(str_fflprintf("%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Response handler not executable",
                            uidf::getUIDString(UID()).c_str(), uidf::getUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond()));
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
            m_podMonitor.AMProcMonitorList[rstMPK.Type()].RecvCount++;
            {
                raii_timer stTimer(&(m_podMonitor.AMProcMonitorList[rstMPK.Type()].ProcTick));
                m_operation(rstMPK);
            }
        }else{
            // shoud I make it fatal?
            // we may get a lot warning message here
            throw std::runtime_error(str_fflprintf("%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Message handler not executable",
                        uidf::getUIDString(UID()).c_str(), uidf::getUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond()));
        }
    }

    // trigger is for all types of messages
    // currently we don't take trigger time into consideration

    if(m_trigger){
        raii_timer stTimer(&(m_podMonitor.TriggerMonitor.ProcTick));
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
        throw std::runtime_error(str_fflprintf("Running out of message ID, exiting..."));
    }

    m_validID = (m_respondHandlerGroup.empty() ? 1 : (m_validID + 1));
    if(auto p = m_respondHandlerGroup.find(m_validID); p == m_respondHandlerGroup.end()){
        return m_validID;
    }
    throw std::runtime_error(str_fflprintf("Running out of message ID, exiting..."));
}

bool ActorPod::forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": %s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                    uidf::getUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                    uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(!rstMB){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                    uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), nRespond));
    }

    if(g_serverArgParser->TraceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    m_podMonitor.AMProcMonitorList[rstMB.Type()].SendCount++;
    return g_actorPool->PostMessage(nUID, {rstMB, UID(), 0, nRespond});
}

bool ActorPod::forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, std::function<void(const MessagePack &)> fnOPR)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": %s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                    uidf::getUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                    uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(!rstMB){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                    uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), nRespond));
    }

    if(!fnOPR){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: %s, ID: NA, Resp: %" PRIu32 "): Response handler not executable",
                    uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    auto nID = GetValidID();
    if(g_serverArgParser->TraceActorMessage){
        g_monoServer->addLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: %u, Resp: %u)",
                uidf::getUIDString(UID()).c_str(), uidf::getUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }

    m_podMonitor.AMProcMonitorList[rstMB.Type()].SendCount++;
    if(g_actorPool->PostMessage(nUID, {rstMB, UID(), nID, nRespond})){
        if(m_respondHandlerGroup.try_emplace(nID, g_monoServer->getCurrTick() + m_expireTime, std::move(fnOPR)).second){
            return true;
        }
        throw std::runtime_error(str_fflprintf(": Failed to register response handler for posted message: %s", MessagePack(rstMB.Type()).Name()));
    }else{
        // respond the response handler here
        // if post failed, it can only be the UID is detached
        if(fnOPR){
            m_podMonitor.AMProcMonitorList[MPK_BADACTORPOD].RecvCount++;
            {
                raii_timer stTimer(&(m_podMonitor.AMProcMonitorList[MPK_BADACTORPOD].ProcTick));
                fnOPR(MPK_BADACTORPOD);
            }
        }
        return false;
    }
}

bool ActorPod::Detach(const std::function<void()> &fnAtExit) const
{
    // we can call detach in its message handler
    // remember the message handler can be executed by worker thread or stealing worker thread
    // we can't guarantee that after detach no thread can access the actor
    //
    //  void MessageHandler()
    //  {
    //      if(bNeedDetach){
    //          Detach();
    //          OtherRelease(); // here still accessing the actor
    //          return;         // after this line we can guarantee actor is not running
    //      }
    //  }
    //
    // it's worse to call it out of its message handler
    // when detach returns we can't guarantee if the actor is till handling message

    // theron library also has this issue
    // only destructor can guarentee the actor is not running any more
    return g_actorPool->Detach(this, fnAtExit);
}

bool ActorPod::CheckInvalid(uint64_t nUID)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": Invalid zero UID"));
    }
    return g_actorPool->CheckInvalid(nUID);
}

void ActorPod::PrintMonitor() const
{
    for(size_t nIndex = 0; nIndex < m_podMonitor.AMProcMonitorList.size(); ++nIndex){
        uint64_t nProcTick  = m_podMonitor.AMProcMonitorList[nIndex].ProcTick / 1000000;
        uint32_t nSendCount = m_podMonitor.AMProcMonitorList[nIndex].SendCount;
        uint32_t nRecvCount = m_podMonitor.AMProcMonitorList[nIndex].RecvCount;
        if(nSendCount || nRecvCount){
            g_monoServer->addLog(LOGTYPE_DEBUG, "UID: %s %s: ProcTick %" PRIu64 "ms, SendCount %" PRIu32 ", RecvCount %" PRIu32, uidf::getUIDString(UID()).c_str(), MessagePack(nIndex).Name(), nProcTick, nSendCount, nRecvCount);
        }
    }
}
