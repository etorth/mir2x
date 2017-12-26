/*
 * =====================================================================================
 *
 *       Filename: activeobject.cpp
 *        Created: 04/28/2016 20:51:29
 *  Last Modified: 12/26/2017 06:06:24
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

#include <cinttypes>
#include "actorpod.hpp"
#include "serverenv.hpp"
#include "monoserver.hpp"
#include "activeobject.hpp"

ActiveObject::ActiveObject()
    : ServerObject()
    , m_StateV()
    , m_StateTimeV()
    , m_ActorPod(nullptr)
    , m_StateHook()
    , m_DelayCmdCount(0)
    , m_DelayCmdQ()
{
    m_StateV.fill(0);
    m_StateTimeV.fill(0);

    auto fnDelayCmdQueue = [this]() -> bool
    {
        extern MonoServer *g_MonoServer;
        if(!m_DelayCmdQ.empty()){
            if(g_MonoServer->GetTimeTick() >= m_DelayCmdQ.top().Tick()){
                try{
                    m_DelayCmdQ.top()();
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Caught exception for delay cmd");
                }
                m_DelayCmdQ.pop();
            }
        }

        // it's never done
        return false;
    };

    m_StateHook.Install("DelayCmdQueue", fnDelayCmdQueue);

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->TraceActorMessageCount){
        auto fnPrintAMCount = [this]() -> bool
        {
            if(ActorPodValid()){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_DEBUG, "(ActorPod: 0X%0*" PRIXPTR ", Name: %s, UID: %u, Length: %" PRIu32 ")",
                        (int)(sizeof(this) * 2), (uintptr_t)(this), ClassName(), UID(), m_ActorPod->GetNumQueuedMessages());
            }

            // it's never done
            return false;
        };
        m_StateHook.Install("PrintAMCount", fnPrintAMCount);
    }

    auto fnRegisterClass = [this]()
    {
        if(!RegisterClass<ActiveObject, ServerObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <ActiveObject, ServerObject> failed");
            g_MonoServer->Restart();
        }
    };

    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);
}

ActiveObject::~ActiveObject()
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
Theron::Address ActiveObject::Activate()
{
    if(!m_ActorPod){
        // 1. enable the scheduling by actor threads
        extern Theron::Framework *g_Framework;
        m_ActorPod = new ActorPod(g_Framework, [this](){ m_StateHook.Execute(); },
                [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){ OperateAM(rstMPK, stFromAddr); });
        // 2. bind the class information to the actorpod
        //    between 1 and 2 there could be gap but OK since before exiting current function
        //    no actor message will be passed or forwarded
        m_ActorPod->BindPod(UID(), ClassName());
        return GetAddress();
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Try to do activation twice: UID = %" PRIu32 ", ClassName = %s", UID(), ClassName());
        g_MonoServer->Restart();

        return Theron::Address::Null();
    }
}

void ActiveObject::Deactivate()
{
    if(m_ActorPod){ m_ActorPod->Detach(); }
}

void ActiveObject::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    extern MonoServer *g_MonoServer;
    m_DelayCmdCount = m_DelayCmdQ.empty() ? 0 : (m_DelayCmdCount + 1);
    m_DelayCmdQ.emplace(nDelayTick + g_MonoServer->GetTimeTick(), m_DelayCmdCount, fnCmd);
}

uint8_t ActiveObject::GetState(uint8_t nState)
{
    return m_StateV[nState];
}

void ActiveObject::SetState(uint8_t nStateLoc, uint8_t nStateValue)
{
    extern MonoServer *g_MonoServer;
    m_StateV[nStateLoc] = nStateValue;
    m_StateTimeV[nStateLoc] = g_MonoServer->GetTimeTick();
}

uint32_t ActiveObject::StateTime(uint8_t nState)
{
    extern MonoServer *g_MonoServer;
    return g_MonoServer->GetTimeTick() - m_StateTimeV[nState];
}
