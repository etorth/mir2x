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

#include "uidfunc.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "raiitimer.hpp"
#include "monoserver.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_ActorPool;
extern MonoServer *g_MonoServer;
extern ServerArgParser *g_ServerArgParser;

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
    , m_Trigger(fnTrigger)
    , m_Operation(fnOperation)
    , m_ValidID(0)
    , m_ExpireTime(nExpireTime)
    , m_RespondHandlerGroup()
    , m_PodMonitor()
{
    if(!g_ActorPool->Register(this)){
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

    if(!g_ActorPool->Detach(this, [](){})){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "ActorPool::Detach(ActorPod = %p) failed", this);
        g_MonoServer->Restart();
    }
}

void ActorPod::InnHandler(const MessagePack &rstMPK)
{
    if(g_ServerArgParser->TraceActorMessage){
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    if(m_ExpireTime){
        while(!m_RespondHandlerGroup.empty()){
            if(g_MonoServer->GetTimeTick() >= m_RespondHandlerGroup.begin()->second.ExpireTime){
                // everytime when we received the new MPK we check if there is handler the timeout
                // also this time get counted into the monitor entry
                m_PodMonitor.AMProcMonitorList[MPK_TIMEOUT].RecvCount++;
                {
                    raii_timer stTimer(&(m_PodMonitor.AMProcMonitorList[MPK_TIMEOUT].ProcTick));
                    m_RespondHandlerGroup.begin()->second.Operation(MPK_TIMEOUT);
                }
                m_RespondHandlerGroup.erase(m_RespondHandlerGroup.begin());
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
        if(auto p = m_RespondHandlerGroup.find(rstMPK.Respond()); p != m_RespondHandlerGroup.end()){
            if(p->second.Operation){
                m_PodMonitor.AMProcMonitorList[rstMPK.Type()].RecvCount++;
                {
                    raii_timer stTimer(&(m_PodMonitor.AMProcMonitorList[rstMPK.Type()].ProcTick));
                    p->second.Operation(rstMPK);
                }
            }else{
                throw std::runtime_error(str_fflprintf("%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Response handler not executable",
                            UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond()));
            }
            m_RespondHandlerGroup.erase(p);
        }else{
            // should only caused by deletion of timeout
            // do nothing for this case, don't take this as an error
        }
    }else{
        // this is not a responding message
        // use default message handling operation
        if(m_Operation){
            m_PodMonitor.AMProcMonitorList[rstMPK.Type()].RecvCount++;
            {
                raii_timer stTimer(&(m_PodMonitor.AMProcMonitorList[rstMPK.Type()].ProcTick));
                m_Operation(rstMPK);
            }
        }else{
            // shoud I make it fatal?
            // we may get a lot warning message here
            throw std::runtime_error(str_fflprintf("%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Message handler not executable",
                        UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond()));
        }
    }

    // trigger is for all types of messages
    // currently we don't take trigger time into consideration

    if(m_Trigger){
        raii_timer stTimer(&(m_PodMonitor.TriggerMonitor.ProcTick));
        m_Trigger();
    }
}

uint32_t ActorPod::GetValidID()
{
    // previously I use g_ServerArgParser->TraceActorMessage to select use this one
    // this is dangerous since if we can change g_ServerArgParser->TraceActorMessage during runtime then it's dead
    if(1){
        static std::atomic<uint32_t> s_ValidID(1);
        auto nNewValidID = s_ValidID.fetch_add(1);

        if(nNewValidID){
            return nNewValidID;
        }
        throw std::runtime_error(str_fflprintf("Running out of message ID, exiting..."));
    }

    m_ValidID = (m_RespondHandlerGroup.empty() ? 1 : (m_ValidID + 1));
    if(auto p = m_RespondHandlerGroup.find(m_ValidID); p == m_RespondHandlerGroup.end()){
        return m_ValidID;
    }
    throw std::runtime_error(str_fflprintf("Running out of message ID, exiting..."));
}

bool ActorPod::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": %s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                    UIDFunc::GetUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(!rstMB){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nRespond));
    }

    if(g_ServerArgParser->TraceActorMessage){
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    m_PodMonitor.AMProcMonitorList[rstMB.Type()].SendCount++;
    return g_ActorPool->PostMessage(nUID, {rstMB, UID(), 0, nRespond});
}

bool ActorPod::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, std::function<void(const MessagePack &)> fnOPR)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": %s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                    UIDFunc::GetUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(!rstMB){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nRespond));
    }

    if(!fnOPR){
        throw std::invalid_argument(str_fflprintf(": %s -> %s: (Type: %s, ID: NA, Resp: %" PRIu32 "): Response handler not executable",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    auto nID = GetValidID();
    if(g_ServerArgParser->TraceActorMessage){
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "%s -> %s: (Type: %s, ID: %u, Resp: %u)",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }

    m_PodMonitor.AMProcMonitorList[rstMB.Type()].SendCount++;
    if(g_ActorPool->PostMessage(nUID, {rstMB, UID(), nID, nRespond})){
        if(m_RespondHandlerGroup.try_emplace(nID, g_MonoServer->GetTimeTick() + m_ExpireTime, std::move(fnOPR)).second){
            return true;
        }
        throw std::runtime_error(str_fflprintf(": Failed to register response handler for posted message: %s", MessagePack(rstMB.Type()).Name()));
    }else{
        // respond the response handler here
        // if post failed, it can only be the UID is detached
        if(fnOPR){
            m_PodMonitor.AMProcMonitorList[MPK_BADACTORPOD].RecvCount++;
            {
                raii_timer stTimer(&(m_PodMonitor.AMProcMonitorList[MPK_BADACTORPOD].ProcTick));
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
    return g_ActorPool->Detach(this, fnAtExit);
}

bool ActorPod::CheckInvalid(uint64_t nUID)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": Invalid zero UID"));
    }
    return g_ActorPool->CheckInvalid(nUID);
}

void ActorPod::PrintMonitor() const
{
    for(size_t nIndex = 0; nIndex < m_PodMonitor.AMProcMonitorList.size(); ++nIndex){
        uint64_t nProcTick  = m_PodMonitor.AMProcMonitorList[nIndex].ProcTick / 1000000;
        uint32_t nSendCount = m_PodMonitor.AMProcMonitorList[nIndex].SendCount;
        uint32_t nRecvCount = m_PodMonitor.AMProcMonitorList[nIndex].RecvCount;
        if(nSendCount || nRecvCount){
            g_MonoServer->AddLog(LOGTYPE_DEBUG, "UID: %s %s: ProcTick %" PRIu64 "ms, SendCount %" PRIu32 ", RecvCount %" PRIu32, UIDFunc::GetUIDString(UID()).c_str(), MessagePack(nIndex).Name(), nProcTick, nSendCount, nRecvCount);
        }
    }
}
