/*
 * =====================================================================================
 *
 *       Filename: activeobject.cpp
 *        Created: 04/28/2016 20:51:29
 *  Last Modified: 05/16/2017 18:44:51
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

    auto fnDelayCmdQueue = [this](){
        extern MonoServer *g_MonoServer;
        if(!m_DelayCmdQ.empty()){
            if(m_DelayCmdQ.top().Tick() <= g_MonoServer->GetTimeTick()){
                try{
                    m_DelayCmdQ.top()();
                }catch(...){
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "caught exception for delay cmd");
                }
                m_DelayCmdQ.pop();
            }
        }

        // it's never done
        return false;
    };

    m_StateHook.Install("DelayCmdQueue", fnDelayCmdQueue);

    extern ServerEnv *g_ServerEnv;
    if(g_ServerEnv->MIR2X_PRINT_ACTOR_MESSAGE_COUNT){
        auto fnPrintAMCount = [this](){
            if(ActorPodValid()){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_INFO,
                        "(%s: 0X%0*" PRIXPTR ", Name: %s, UID: %u, Length: %" PRIu32 ")",
                        "ActorPod", (int)(sizeof(this) * 2), (uintptr_t)(this), ClassName(), UID(), m_ActorPod->GetNumQueuedMessages());
            }

            // it's never done
            return false;
        };
        m_StateHook.Install("PrintAMCount", fnPrintAMCount);
    }

    auto fnRegisterClass = [this]() -> void {
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
        extern Theron::Framework *g_Framework;
        m_ActorPod = new ActorPod(g_Framework, [this](){ m_StateHook.Execute(); },
                [this](const MessagePack &rstMPK, const Theron::Address &stFromAddr){ Operate(rstMPK, stFromAddr); });
#if defined(MIR2X_DEBUG) && (MIR2X_DEBUG >= 5)
        {
            m_ActorPod->BindPod(UID(), ClassName());
        }
#endif
    }

    return GetAddress();
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
