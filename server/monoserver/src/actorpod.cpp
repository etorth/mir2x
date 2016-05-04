/*
 * =====================================================================================
 *
 *       Filename: actorpod.cpp
 *        Created: 05/03/2016 15:00:35
 *  Last Modified: 05/04/2016 12:20:23
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

#include "actorpod.hpp"
#include "monoserver.hpp"

void ActorPod::InnHandler(const MessagePack &rstMPK, Theron::Address stFromAddr)
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
                    g_MonoServer->AddLog(LOGTYPE_WARNING,
                            "caught exception in operating response message");
                }
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING,
                        "registered response operation is not callable");
            }
            m_RespondMessageRecordM.erase(pRecord);
        }else{
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING,
                    "no registered operation for response message found");
        }
        // TODO & TBD
        // we ignore it or send it to m_Operate??? currently just dropped
        goto __ACTORPOD_INNHANDLER_CALL_TRIGGER;
    }

    // now message are handling  not on purpose of response only
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
            g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception in ActorPod");
        }
    }else{
        // TODO & TBD
        // this message will show up many and many if not valid handler found
        // extern MonoServer *g_MonoServer;
        // g_MonoServer->AddLog(LOGTYPE_WARNING,
        //         "registered operation for message is not callable");
    }

    // every time when a message caught, we call trigger
__ACTORPOD_INNHANDLER_CALL_TRIGGER:
    if(m_Trigger){
        m_Trigger();
    }
}

uint32_t ActorPod::ValidID()
{
    m_ValidID = (m_RespondMessageRecordM.empty() ? 1 : m_ValidID + 1);
    auto pRecord = m_RespondMessageRecordM.find(m_ValidID);
    if(pRecord != m_RespondMessageRecordM.end()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "response requested message overflows");
        // TODO
        // this function won't return;
        g_MonoServer->Restart();
    }

    return m_ValidID;
}
