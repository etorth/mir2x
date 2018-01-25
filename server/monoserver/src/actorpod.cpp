/*
 * =====================================================================================
 *
 *       Filename: actorpod.cpp
 *        Created: 05/03/2016 15:00:35
 *  Last Modified: 12/14/2017 19:13:25
 *
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

#include "actorpod.hpp"
#include "serverenv.hpp"
#include "monoserver.hpp"

void ActorPod::InnHandler(const MessagePack &rstMPK, const Theron::Address stFromAddr)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u)",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }

    // everytime when message comes check the expire time
    // remove all expired message handler before any handling
    if(m_ExpireTime){
        while(!m_RespondMessageRecord.empty()){
            extern MonoServer *g_MonoServer;
            if(m_RespondMessageRecord.begin()->second.ExpireTime < g_MonoServer->GetTimeTick()){
                // expired, erase current message handler
                // send MPK_TIMEOUT to registered message handler to indicate erasion
                try{
                    m_RespondMessageRecord.begin()->second.RespondOperation(MPK_TIMEOUT, GetAddress());
                }catch(...){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: MPK_TIMEOUT, ID: 0, Resp: %u) : Caught exception from current message handler",
                            (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), m_RespondMessageRecord.begin()->first);
                }
                m_RespondMessageRecord.erase(m_RespondMessageRecord.begin());
                continue;
            }

            // std::map<ID, Handler> keeps order in ID number
            // ID number is the Resp() of the responding messages
            //
            // and we guarantee ID1 < ID2 ==> ExpireTime1 <= ExpireTime2
            // so if we get first non-expired handler, means the rest are all not expired
            // good feature for std::map, reason why use it instead of std::unordered_map here
            break;
        }
    }

    if(rstMPK.Respond()){
        auto pRecord = m_RespondMessageRecord.find(rstMPK.Respond());
        // try to find the response handler for current responding message
        // 1.     find it, good
        // 2. not find it: 1. didn't register for it
        //                 2. repsonse is too late ooops
        if(pRecord == m_RespondMessageRecord.end()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u) : No valid handler for current message",
                    (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            // won't die here
            // could be legal if then responding actor delay too much
        }else{
            // we do have an record for this message
            // if we still can find it means it's not expired
            if(pRecord->second.RespondOperation){
                try{
                    pRecord->second.RespondOperation(rstMPK, stFromAddr);
                }catch(...){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u) : Caught exception from current message handler",
                            (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
                }
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u) : Current message handler not executable",
                        (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            }
            m_RespondMessageRecord.erase(pRecord);
        }
    }else{
        // informing type message
        // now message are handling not on purpose of response
        if(m_Operation){
            try{
                m_Operation(rstMPK, stFromAddr);
            }catch(...){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u) : Caught exception from current message handler",
                        (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
            }
        }else{
            // TODO
            // this waring will show up many and many if not valid handler found
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u) : Registered operation handler is not executable",
                    (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }
    }

    // no matter this message is for response or initialized by others
    // every time when a message caught, we call trigger to do condition check
    if(m_Trigger){
        try{
            m_Trigger();
        }catch(...){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) : Caught exception in trigger after message (Type: %s, ID: %u, Resp: %u)",
                    (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
        }
    }else{
        // TODO
        // it's ok to work without trigger for an actorpod
    }
}

uint32_t ActorPod::ValidID()
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        // for debug only
        // when debug all messages get unique ID
        // make it convienent to get all records for one message in the log file
        static std::atomic<uint32_t> s_ValidID(1);
        return s_ValidID.fetch_add(1);
    }

    // won't reset the current valid ID if there is handler in record
    // check the requirement for ValidID() in header file
    m_ValidID = (m_RespondMessageRecord.empty() ? 1 : (m_ValidID + 1));

    // before return the ID generated
    // we search the map to make sure no (ID, Operation) registered
    auto pRecord = m_RespondMessageRecord.find(m_ValidID);
    if(pRecord == m_RespondMessageRecord.end()){
        return m_ValidID;
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Response requested message overflows");
        g_MonoServer->Restart();
        return 0;
    }
}

bool ActorPod::Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond)
{
    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u)",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), 0, nRespond);
    }

    if(rstAddr == Theron::Address::Null()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u) : Try to send message to an emtpy address",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), 0, nRespond);
        return false;
    }

    if(rstAddr == GetAddress()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u) : Try to send message to itself",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), 0, nRespond);
        return false;
    }

    if(!Theron::Actor::Send<MessagePack>({rstMB, 0, nRespond}, rstAddr)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u) : Faile to send message to given address",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), 0, nRespond);
        return false;
    }

    return true;
}

// send a responding message and exptecting a reply
bool ActorPod::Forward(const MessageBuf &rstMB,
        const Theron::Address &rstAddr, uint32_t nRespond,
        const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
{
    uint32_t nID = ValidID();

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessage){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_DEBUG, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u)",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }

    if(rstAddr == Theron::Address::Null()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u) : Try to send message to an empty address",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
        return false;
    }

    if(rstAddr == GetAddress()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u) : Try to send message to itself",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
        return false;
    }

    if(!Theron::Actor::Send<MessagePack>({rstMB, nID, nRespond}, rstAddr)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u) : Failed to send message to given address",
                (int)(sizeof(this) * 2), (uintptr_t)(this), Name(), UID(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
        return false;
    }

    extern MonoServer *g_MonoServer;
    return m_RespondMessageRecord.emplace(nID, RespondMessageRecord((g_MonoServer->GetTimeTick() + m_ExpireTime), fnOPR)).second;
}
