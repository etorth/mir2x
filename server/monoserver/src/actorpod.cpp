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
#include "monoserver.hpp"
#include "serverargparser.hpp"

extern ActorPool *g_ActorPool;
extern MonoServer *g_MonoServer;

ActorPod::ActorPod(uint64_t nUID,
        const std::function<void()> &fnTrigger,
        const std::function<void(const MessagePack &)> &fnOperation, uint32_t nExpireTime)
    : m_UID([nUID]() -> uint64_t
      {
          if(!nUID){
              g_MonoServer->AddLog(LOGTYPE_WARNING, "Provide user-defined zero UID");
              g_MonoServer->Restart();
              return 0;
          }

          if(nUID & 0XFFFF000000000000){
              g_MonoServer->AddLog(LOGTYPE_WARNING, "Provide user-defined UID greater than 0XFFFF000000000000: %" PRIu64, nUID);
              g_MonoServer->Restart();
              return 0;
          }

          return nUID;
      }())
    , m_Trigger(fnTrigger)
    , m_Operation(fnOperation)
    , m_ValidID(0)
    , m_ExpireTime(nExpireTime)
    , m_RespondHandlerGroup()
{
    if(!g_ActorPool->Register(this)){
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

    if(!g_ActorPool->Detach(this, [](){})){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "ActorPool::Detach(ActorPod = %p) failed", this);
        g_MonoServer->Restart();
    }
}

void ActorPod::InnHandler(const MessagePack &rstMPK)
{
    extern ServerArgParser *g_ServerArgParser;
    if(g_ServerArgParser->TraceActorMessage){
        g_MonoServer->AddLog(LOGTYPE_DEBUG,
                "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 ")",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    if(m_ExpireTime){
        while(!m_RespondHandlerGroup.empty()){
            if(g_MonoServer->GetTimeTick() >= m_RespondHandlerGroup.begin()->second.ExpireTime){
                try{
                    m_RespondHandlerGroup.begin()->second.Operation(MPK_TIMEOUT);
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "%s <- NA : (Type: MPK_TIMEOUT, ID: %" PRIu32 ", Resp: NA): Caught exception in handing timeout",
                            UIDFunc::GetUIDString(UID()).c_str(), m_RespondHandlerGroup.begin()->first, rstMPK.Respond());
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
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): No valid handler for current message",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            // won't die here
            // could be legal if then responding actor delays too much
        }else{
            if(p->second.Operation){
                try{
                    p->second.Operation(rstMPK);
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exception in response handler",
                            UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
                }
            }else{
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
            try{
                m_Operation(rstMPK);
            }catch(...){
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exception in message handler",
                        UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            }
        }else{
            // shoud I make it fatal?
            // we may get a lot warning message here
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Current message handler not executable",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }
    }

    if(m_Trigger){
        try{
            m_Trigger();
        }catch(...){
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exeption in trigger after message handling",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }
    }
}

uint32_t ActorPod::GetValidID()
{
    extern ServerArgParser *g_ServerArgParser;
    if(g_ServerArgParser->TraceActorMessage){
        static std::atomic<uint32_t> s_ValidID(1);
        return s_ValidID.fetch_add(1);
    }

    m_ValidID = (m_RespondHandlerGroup.empty() ? 1 : (m_ValidID + 1));
    if(auto p = m_RespondHandlerGroup.find(m_ValidID); p == m_RespondHandlerGroup.end()){
        return m_ValidID;
    }else{
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Running out of message ID, exiting...");
        return 0;
    }
}

bool ActorPod::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond)
{
    if(!nUID){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                UIDFunc::GetUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(!rstMB){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nRespond));
    }

    extern ServerArgParser *g_ServerArgParser;
    if(g_ServerArgParser->TraceActorMessage){
        g_MonoServer->AddLog(LOGTYPE_DEBUG,
                "%s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 ")",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond);
    }

    return g_ActorPool->PostMessage(nUID, {rstMB, UID(), 0, nRespond});
}

bool ActorPod::Forward(uint64_t nUID, const MessageBuf &rstMB, uint32_t nRespond, std::function<void(const MessagePack &)> fnOPR)
{
    if(!nUID){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> NONE: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to an empty address",
                UIDFunc::GetUIDString(UID()).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> %s: (Type: %s, ID: 0, Resp: %" PRIu32 "): Try to send message to itself",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    if(!rstMB){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> %s: (Type: MPK_NONE, ID: 0, Resp: %" PRIu32 "): Try to send an empty message",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), nRespond));
    }

    if(!fnOPR){
        throw std::invalid_argument(str_ffl() +
                str_printf(": %s -> %s: (Type: %s, ID: NA, Resp: %" PRIu32 "): Response handler not executable",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nRespond));
    }

    auto nID = GetValidID();

    extern ServerArgParser *g_ServerArgParser;
    if(g_ServerArgParser->TraceActorMessage){
        g_MonoServer->AddLog(LOGTYPE_DEBUG,
                "%s -> %s: (Type: %s, ID: %u, Resp: %u)",
                UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(nUID).c_str(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }

    if(g_ActorPool->PostMessage(nUID, {rstMB, UID(), nID, nRespond})){
        if(m_RespondHandlerGroup.try_emplace(nID, g_MonoServer->GetTimeTick() + m_ExpireTime, std::move(fnOPR)).second){
            return true;
        }
        throw std::runtime_error(str_ffl() + str_printf(": Failed to register response handler for posted message: %s", MessagePack(rstMB.Type()).Name()));
    }else{
        // respond the response handler here
        // if post failed, it can only be the UID is detached
        if(fnOPR){
            fnOPR(MPK_BADUID);
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

uint32_t ActorPod::GetMessageCount() const
{
    return 0;
}
