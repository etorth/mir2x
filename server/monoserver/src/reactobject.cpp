/*
 * =====================================================================================
 *
 *       Filename: reactobject.cpp
 *        Created: 04/28/2016 20:51:29
 *  Last Modified: 05/31/2016 15:38:30
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
#include "reactobject.hpp"

ReactObject::ReactObject(uint8_t nCategory)
    : ServerObject(nCategory)
    , m_ActorPod(nullptr)
{
    auto fnDelayCmdQueue = [this](){
        if(!m_DelayCmdQ.empty()){
            extern MonoServer *g_MonoServer;
            if(m_DelayCmdQ.top().Tick() >= g_MonoServer->GetTimeTick()){
                try{
                    m_DelayCmdQ.top()();
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception for delay cmd");
                }
                m_DelayCmdQ.pop();
            }
        }
    };
    Install("DelayCmdQueue", fnDelayCmdQueue);
}

ReactObject::~ReactObject()
{
    delete m_ActorPod;
}

// TODO & TBD
// when an actor is activated by more than one time, we can
// 1. delete previously allocated actor and create a new one
// 2. just return current address
//
// Now I use method-2, since the address could hanve been assigned to many other place
// for communication, delete it may cause problems
//
// And if we really want to change the address of current object, maybe we need to
// delte current object in total and create a new one instead
Theron::Address ReactObject::Activate()
{
    if(!m_ActorPod){
        extern Theron::Framework *g_Framework;
        m_ActorPod = new ActorPod(g_Framework, [this](){ InnTrigger(); },
                [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){
                this->Operate(rstMPK, stFromAddr);
                });
        m_ThisAddress = m_ActorPod->GetAddress();
    }

    // TODO & TBD no idea of whether this is needed
    return m_ThisAddress;
}

bool ReactObject::AccessCheck()
{
    if(m_ActorPod){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Direct access internal state of an actor after activation");
        g_MonoServer->Restart();

        return false;
    }
    return true;
}

void ReactObject::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    extern MonoServer *g_MonoServer;
    m_DelayCmdQ.emplace(nDelayTick + g_MonoServer->GetTimeTick(), fnCmd);
}
