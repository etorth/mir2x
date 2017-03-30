/*
 * =====================================================================================
 *
 *       Filename: actorpod.cpp
 *        Created: 05/03/2016 15:00:35
 *  Last Modified: 03/30/2017 01:31:03
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

#include "actorpod.hpp"
#include "monoserver.hpp"

void ActorPod::InnHandler(const MessagePack &rstMPK, const Theron::Address stFromAddr)
{
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO,
                "(Pod: %p, Name: %s, UID: %u) <- (Type: %s, ID: %u, Resp: %u)",
                this, Name(), UID(), rstMPK.Name(), rstMPK.ID(), rstMPK.Respond());
    }
#endif

    if(rstMPK.Respond()){
        // message for response, we must have handler registered for it before
        // if not, we treat it as an error and die currently
        auto pRecord = m_RespondMessageRecord.find(rstMPK.Respond());
        if(pRecord == m_RespondMessageRecord.end()){
            // print detailed info for this message for debug
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "no registered operation for response message: id = %u, resp = %u type = %s, this = %p, sent from: %s, to %s",
                    rstMPK.ID(), rstMPK.Respond(), rstMPK.Name(), this, stFromAddr.AsString(), GetAddress().AsString());
            // TODO & TBD, here we directly die or continue?
            g_MonoServer->Restart();
        }else{
            // we do have an record for this message
            if(pRecord->second.RespondOperation){
                try{
                    pRecord->second.RespondOperation(rstMPK, stFromAddr);
                }catch(...){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception in operating response message");
                    g_MonoServer->Restart();
                }
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "registered response operation is not callable");
                g_MonoServer->Restart();
            }
            m_RespondMessageRecord.erase(pRecord);
        }
    }else{
        // now message are handling not on purpose of response
        if(m_Operate){
            // in theron address is recommanded to copy rather than ref, but
            // here we use const ref when passing to m_Operate, because when
            // calling m_Operate(), address already has a copy when executaion
            // goes inside InnHandler(), so always there is a valid copy of
            // address for m_Operate()
            //
            // if m_Operate() has any other async\ed operation upon the address
            // inside, it should handler by itself.
            //
            // so there is only one copy of InnHandler() when callback invoked, and
            // don't worry, Theron::Address is actually only a pointer so copying
            // is really cheap
            //
            // for MessagePack copying, since Theron itself using const ref, so I
            // use const ref here
            try{
                m_Operate(rstMPK, stFromAddr);
            }catch(...){
                // 1. assume monoserver is ready when invoking callback
                // 2. AddLog() is well defined in multithread environment
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception in ActorPod: %s", rstMPK.Name());
                g_MonoServer->Restart();
            }
        }else{
            // TODO & TBD
            // this message will show up many and many if not valid handler found
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "registered operation for message is not callable");
            g_MonoServer->Restart();
        }
    }

    // no matter this message is for response or initialized by others
    // every time when a message caught, we call trigger for condition check
    // it's OK to work without a valid trigger
    if(m_Trigger){
        try{
            m_Trigger();
        }catch(...){
            // 1. assume monoserver is ready when invoking callback
            // 2. AddLog() is well defined in multithread environment
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception in ActorPod trigger");
            g_MonoServer->Restart();
        }
    }else{
        // TODO: it's ok to work without trigger for an actorpod
    }
}

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
#include <atomic>
std::atomic<uint32_t> g_ValidID(1);
#endif

// generate an uniqued actor message ID
// uniqueness is required to get the registered response handler
uint32_t ActorPod::ValidID()
{
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        // for debug
        // all sent messages have unique IDs for all ActorPod
        return g_ValidID.fetch_add(1);
    }
#endif
    m_ValidID = (m_RespondMessageRecord.empty() ? 1 : (m_ValidID + 1));

    // before return the ID generated
    // we search the map to make sure no (ID, Operation) registered
    auto pRecord = m_RespondMessageRecord.find(m_ValidID);
    if(pRecord == m_RespondMessageRecord.end()){
        return m_ValidID;
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "response requested message overflows");
        g_MonoServer->Restart();
    }
}

bool ActorPod::Forward(const MessageBuf &rstMB, const Theron::Address &rstAddr, uint32_t nRespond)
{
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO,
                "(Pod: %p, Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u)",
                this, Name(), UID(), MessagePack(rstMB.Type()).Name(), 0, nRespond);
    }
#endif

    if(!rstAddr){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "trying to send message to a null address");
        g_MonoServer->Restart();
    }

    if(rstAddr == GetAddress()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "trying to send message to itself");
        g_MonoServer->Restart();
    }

    return Theron::Actor::Send<MessagePack>({rstMB, 0, nRespond}, rstAddr);
}

// send a responding message and exptecting a reply
bool ActorPod::Forward(const MessageBuf &rstMB,
        const Theron::Address &rstAddr, uint32_t nRespond,
        const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
{
    // 1. get valid ID
    uint32_t nID = ValidID();

#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
    {
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO,
                "(Pod: %p, Name: %s, UID: %u) -> (Type: %s, ID: %u, Resp: %u)",
                this, Name(), UID(), MessagePack(rstMB.Type()).Name(), nID, nRespond);
    }
#endif

    if(!rstAddr){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "trying to send message to an invalid address");
        g_MonoServer->Restart();
    }

    if(rstAddr == GetAddress()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "trying to send message to itself");
        g_MonoServer->Restart();
    }

    // 2. send it
    bool bRet = Theron::Actor::Send<MessagePack>({rstMB, nID, nRespond}, rstAddr);

    // 3. if send succeed, then register the handler of responding message
    //    here we won't exam fnOPR's callability
    if(bRet){
        m_RespondMessageRecord.emplace(std::make_pair(nID, fnOPR));
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "ooops send message failed: %s", MessagePack(rstMB.Type()).Name());
        g_MonoServer->Restart();
    }

    // 4. return whether we succeed
    return bRet;
}
