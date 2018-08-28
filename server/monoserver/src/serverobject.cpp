/*
 * =====================================================================================
 *
 *       Filename: serverobject.cpp
 *        Created: 04/28/2016 20:51:29
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
#include "metronome.hpp"
#include "serverenv.hpp"
#include "monoserver.hpp"
#include "serverobject.hpp"

ServerObject::ServerObject(uint64_t nUID)
    : m_UID(nUID)
    , m_UIDName(UIDFunc::GetUIDString(nUID))
    , m_StateV()
    , m_StateTimeV()
    , m_ActorPod(nullptr)
    , m_StateHook()
    , m_DelayCmdCount(0)
    , m_DelayCmdQ()
{

    // link to the mono object pool
    // never allocate ServerObject on stack
    // MonoServer::EraseUID() will use *this* to call destructor
    extern MonoServer *g_MonoServer;
    g_MonoServer->LinkUID(m_UID, this);

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
}

ServerObject::~ServerObject()
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
// delete current object totally and create a new one instead
uint64_t ServerObject::Activate()
{
    if(!m_ActorPod){
        try{
            m_ActorPod = new ActorPod(m_UID, [this](){ m_StateHook.Execute(); }, [this](const MessagePack &rstMPK){ OperateAM(rstMPK, stFromAddr); });
        }catch(...){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Activate server object failed: %s", UIDFunc::GetUIDString(m_UID).c_str());
            g_MonoServer->Restart();
        }
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Activation twice: %s", UIDFunc::GetUIDString(UID()).c_str());
        g_MonoServer->Restart();
    }
    return UID();
}

void ServerObject::Deactivate()
{
    if(m_ActorPod){
        m_ActorPod->Detach();
    }
}

void ServerObject::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    extern MonoServer *g_MonoServer;
    m_DelayCmdCount = m_DelayCmdQ.empty() ? 0 : (m_DelayCmdCount + 1);
    m_DelayCmdQ.emplace(nDelayTick + g_MonoServer->GetTimeTick(), m_DelayCmdCount, fnCmd);
}

uint8_t ServerObject::GetState(uint8_t nState)
{
    return m_StateV[nState];
}

void ServerObject::SetState(uint8_t nStateLoc, uint8_t nStateValue)
{
    extern MonoServer *g_MonoServer;
    m_StateV[nStateLoc] = nStateValue;
    m_StateTimeV[nStateLoc] = g_MonoServer->GetTimeTick();
}

uint32_t ServerObject::StateTime(uint8_t nState)
{
    extern MonoServer *g_MonoServer;
    return g_MonoServer->GetTimeTick() - m_StateTimeV[nState];
}

bool ServerObject::AddTick()
{
    extern Metronome *g_Metronome;
    return g_Metronome->Add(UID());
}

void ServerObject::RemoveTick()
{
    extern Metronome *g_Metronome;
    g_Metronome->Remove(UID());
}
