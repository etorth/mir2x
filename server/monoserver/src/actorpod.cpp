/*
 * =====================================================================================
 *
 *       Filename: actorpod.cpp
 *        Created: 05/03/2016 15:00:35
 *  Last Modified: 06/05/2016 19:17:20
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
    if(rstMPK.Respond()){
        auto pRecord = m_RespondMessageRecordM.find(rstMPK.Respond());
        if(pRecord != m_RespondMessageRecordM.end()){
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
            m_RespondMessageRecordM.erase(pRecord);
        }else{
            // print detailed info for this message for debug
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "no registered operation for response message: id = %u, resp = %u type = %s, this = %p, sent from: %s, to %s",
                    rstMPK.ID(), rstMPK.Respond(), rstMPK.Name(), this, stFromAddr.AsString(), GetAddress().AsString());
            // TODO & TBD, here we directly die or continue?
            g_MonoServer->Restart();
        }

        // for trigger function envaluation
        goto __ACTORPOD_INNHANDLER_CALL_TRIGGER;
    }

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

    // every time when a message caught, we call trigger for condition check
__ACTORPOD_INNHANDLER_CALL_TRIGGER:
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

// this funciton is not actor-safe, don't call it outside the actor itself
uint32_t ActorPod::ValidID()
{
    // return g_Count++;

    m_ValidID = (m_RespondMessageRecordM.empty() ? 1 : (m_ValidID + 1));
    auto pRecord = m_RespondMessageRecordM.find(m_ValidID);
    if(pRecord != m_RespondMessageRecordM.end()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "response requested message overflows");
        g_MonoServer->Restart();
    }

    return m_ValidID;
}

// uint32_t g_RespCount(UINT_MAX);

// send a responding message and exptecting a reply
bool ActorPod::Forward(const MessageBuf &rstMB,
        const Theron::Address &rstAddr, uint32_t nRespond,
        const std::function<void(const MessagePack&, const Theron::Address &)> &fnOPR)
{
    // 1. get valid ID
    uint32_t nID = ValidID();

    // 2. send it
    bool bRet = Theron::Actor::Send<MessagePack>({rstMB, nID, nRespond}, rstAddr);

    // 3. if send succeed, then register the handler of responding message
    //    here we won't exam fnOPR's callability
    if(bRet){
        m_RespondMessageRecordM.emplace(std::make_pair(nID, fnOPR));
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "ooops send message failed: %s", MessagePack(rstMB.Type()).Name());
        g_MonoServer->Restart();
    }

    // 4. return whether we succeed
    return bRet;
}
