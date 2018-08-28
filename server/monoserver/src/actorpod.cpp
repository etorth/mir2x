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
        const std::string &szName,
        const std::function<void()> &fnTrigger,
        const std::function<void(const MessagePack &)> &fnOperation, uint32_t nExpireTime)
    : m_UID([nUID]() -> uint64_t
      {
          if(nUID){
              if(nUID & 0XFFFF000000000000){
                  extern MonoServer *g_MonoServer;
                  g_MonoServer->AddLog(LOGTYPE_FATAL, "Provide user-defined UID greater than 0XFFFF000000000000: %" PRIu64, nUID);
                  return 0;
              }
              return nUID;
          }

          // user won't provide uid by themselves
          // generate an UID from reserved uid region

          extern ActorPool *g_ActorPool;
          return g_ActorPool->GetInnActorUID();
      }())
    , m_Name(szName)
    , m_Trigger(fnTrigger)
    , m_Operation(fnOperation)
    , m_ValidID(0)
    , m_ExpireTime(nExpireTime)
    , m_RespondHandlerGroup()
{
    extern ActorPool *g_ActorPool;
    g_ActorPool->Register(this);
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

    if(m_ExpireTime){
        while(!m_RespondHandlerGroup.empty()){
            extern MonoServer *g_MonoServer;
            if(g_MonoServer->GetTimeTick() >= m_RespondHandlerGroup.begin()->second.ExpireTime){
                try{
                    m_RespondHandlerGroup.begin()->second.Operation(MPK_TIMEOUT);
                }catch(...){
                    extern MonoServer *g_MonoServer;
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
            extern MonoServer *g_MonoServer;
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
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exception in response handler",
                            UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
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
            try{
                m_Operation(rstMPK);
            }catch(...){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exception in message handler",
                        UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
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
        try{
            m_Trigger();
        }catch(...){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "%s <- %s : (Type: %s, ID: %" PRIu32 ", Resp: %" PRIu32 "): Caught exeption in trigger after message handling",
                    UIDFunc::GetUIDString(UID()).c_str(), UIDFunc::GetUIDString(rstMPK.From()).c_str(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
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

void ActorPod::Detach() const
{
    extern ActorPool *g_ActorPool;
    g_ActorPool->Detach(this);
}
