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
#include "serverenv.hpp"
#include "monoserver.hpp"

ActorPod::ActorPod(uint64_t nUID,
        const std::function<void()> &fnTrigger,
        const std::function<void(const MessagePack &)> &fnOperation, uint32_t nExpireTime)
    : m_UID([nUID]() -> uint64_t
      {
          if(!nUID){
              extern MonoServer *g_MonoServer;
              g_MonoServer->AddLog(LOGTYPE_WARNING, "Provide user-defined zero UID");
              g_MonoServer->Restart();
              return 0;
          }

          if(nUID & 0XFFFF000000000000){
              extern MonoServer *g_MonoServer;
              g_MonoServer->AddLog(LOGTYPE_WARNING, "Provide user-defined UID greater than 0XFFFF000000000000: %" PRIu64, nUID);
              g_MonoServer->Restart();
              return 0;
          }

          return nUID;
      }())
    , m_Trigger(fnTrigger)
    , m_Operation(fnOperation)
    , m_Detached(std::make_shared<std::atomic<bool>>(false))
    , m_ValidID(0)
    , m_ExpireTime(nExpireTime)
    , m_RespondHandlerGroup()
{
    extern ActorPool *g_ActorPool;
    if(!g_ActorPool->Register(this)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Register actor failed: ActorPod = %p, ActorPod::UID() = %" PRIu64, this, UID());
        g_MonoServer->Restart();
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

    extern ActorPool *g_ActorPool;
    if(!g_ActorPool->Detach(this, true)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "ActorPool::Detach(ActorPod = %p) failed", this);
        g_MonoServer->Restart();
    }
}

void ActorPod::InnHandler(const MessagePack &rstMPK)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG,
                "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    if(m_Detached->load()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Detached actor get scheduled",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    if(m_ExpireTime){
        while(!m_RespondHandlerGroup.empty()){
            extern MonoServer *g_MonoServer;
            if(g_MonoServer->GetTimeTick() >= m_RespondHandlerGroup.begin()->second.ExpireTime){
                // save the information ref to this actor
                // to support actor to call Detach() inside its message handler
                // need to use std::shared_ptr<> to keep m_Detached
                auto pDetached  = m_Detached;
                uint64_t nUID   = UID();
                uint32_t nMPKID = m_RespondHandlerGroup.begin()->first;
                try{
                    m_RespondHandlerGroup.begin()->second.Operation(MPK_TIMEOUT);
                }catch(...){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "%s <- NA : (Type: MPK_TIMEOUT, ID: %" PRIu32 ", Resp: NA): Caught exception in handing timeout",
                            UIDFunc::GetUIDString(nUID).c_str(), nMPKID, rstMPK.Respond());
                }

                // detached in message handler detected, immediate leave it
                // user can do suicide inside the message handler
                //
                //    if(need_detach){
                //        Detach();
                //        delete this;
                //    }
                //
                // here deletion invalidate the this->m_Detached
                // but pDetached is still valid for check

                // check detached status after message handler
                // immediately return if detached

                if(pDetached->load()){
                    return;
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
        // 2. not find it: 1. didn't register for it
        //                 2. repsonse is too late ooops
        if(auto p = m_RespondHandlerGroup.find(rstMPK.Respond()); p == m_RespondHandlerGroup.end()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): No valid handler for current message",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            // won't die here
            // could be legal if then responding actor delays too much
        }else{
            if(p->second.Operation){
                auto pDetached = m_Detached;
                uint64_t nUID  = UID();
                try{
                    p->second.Operation(rstMPK);
                }catch(...){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exception in response handler",
                            UIDFunc::GetUIDString(nUID).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
                }

                // check detached status after message handler
                // immediately return if detached
                if(pDetached->load()){
                    return;
                }
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Current response handler not executable",
                        UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            }
            m_RespondHandlerGroup.erase(p);
        }
    }else{
        // this is not a responding message
        // use default message handling operation
        if(m_Operation){
            auto pDetached = m_Detached;
            uint64_t nUID  = UID();
            try{
                m_Operation(rstMPK);
            }catch(...){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exception in message handler",
                        UIDFunc::GetUIDString(nUID).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            }

            // check detached status after message handler
            // immediately return if detached
            if(pDetached->load()){
                return;
            }
        }else{
            // shoud I make it fatal?
            // we may get a lot warning message here
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Current message handler not executable",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }
    }

    if(m_Trigger){
        auto pDetached = m_Detached;
        uint64_t nUID  = UID();
        try{
            m_Trigger();
        }catch(...){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exeption in trigger after message handling",
                    UIDFunc::GetUIDString(nUID).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }

        // check detached status after message handler
        // immediately return if detached
        if(pDetached->load()){
            return;
        }
    }
}

uint32_t ActorPod::GetValidID()
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        static std::atomic<uint32_t> s_ValidID(1);
        return s_ValidID.fetch_add(1);
    }

    m_ValidID = (m_RespondHandlerGroup.empty() ? 1 : (m_ValidID + 1));
    if(auto p = m_RespondHandlerGroup.find(m_ValidID); p == m_RespondHandlerGroup.end()){
        return m_ValidID;
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Running out of message ID, exiting...");
        return 0;
    }
}

bool ActorPod::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond)
{
    if(!nUID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                UIDFunc::GetUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
        return false;
    }

    if(nUID == UID()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
        return false;
    }

    if(!rstMB){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nRespond);
        return false;
    }

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG,
                "%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    extern ActorPool *g_ActorPool;
    return g_ActorPool->PostMessage(nUID, {rstMB, UID(), 0, nRespond});
}

bool ActorPod::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, std::function<void(const MessagePack &)> fnOPR)
{
    if(!nUID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> NONE: (Type: %s, ID: NA, Resp: %" PRIu32 "): Try to send message to an empty address",
                UIDFunc::GetUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
        return false;
    }

    if(nUID == UID()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> %s: (Type: %s, ID: NA, Resp: %" PRIu32 "): Try to send message to itself",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
        return false;
    }

    if(!rstMB){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> %s: (Type: MPK_NONE, ID: NA, Resp: %" PRIu32 "): Try to send an empty message",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nRespond);
        return false;
    }

    if(!fnOPR){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING,
                "%s -> %s: (Type: %s, ID: NA, Resp: %" PRIu32 "): Response handler not executable",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    auto nID = GetValidID();

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG,
                "%s -> %s: (Type: %s, ID: %u, Resp: %u)",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }

    extern ActorPool *g_ActorPool;
    if(!g_ActorPool->PostMessage(nUID, {rstMB, UID(), nID, nRespond})){
        return false;
    }

    extern MonoServer *g_MonoServer;
    return m_RespondHandlerGroup.emplace(nID, RespondHandler(g_MonoServer->GetTimeTick() + m_ExpireTime, std::move(fnOPR))).second;
}

bool ActorPod::Detach(bool bForce) const
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
    extern ActorPool *g_ActorPool;
    if(g_ActorPool->Detach(this, bForce)){
        m_Detached->store(true);
        return true;
    }
    return false;
}

uint32_t ActorPod::GetMessageCount() const
{
    return 0;
}
