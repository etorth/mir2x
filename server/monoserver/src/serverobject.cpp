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
#include "serverargparser.hpp"
#include "monoserver.hpp"
#include "serverobject.hpp"

extern MonoServer *g_MonoServer;
extern ServerArgParser *g_ServerArgParser;

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
    m_StateV.fill(0);
    m_StateTimeV.fill(0);

    m_StateHook.Install("DelayCmdQueue", [this]() -> bool
    {
        if(!m_DelayCmdQ.empty()){
            if(g_MonoServer->GetTimeTick() >= m_DelayCmdQ.top().Tick()){
                m_DelayCmdQ.top()();
                m_DelayCmdQ.pop();
            }
        }
        return false;
    });

    if(g_ServerArgParser->TraceActorMessage){
        m_StateHook.Install("ReportActorPodMonitor", [this, nLastCheckTick = (uint32_t)(0)]() mutable -> bool
        {
            if(auto nCurrCheckTick = g_MonoServer->GetTimeTick(); nLastCheckTick + 1000 < nCurrCheckTick){
                if(ActorPodValid()){
                    m_ActorPod->PrintMonitor();
                }
                nLastCheckTick = nCurrCheckTick;
            }
            return false;
        });
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
        m_ActorPod = new ActorPod(m_UID, [this](){ m_StateHook.Execute(); }, [this](const MessagePack &rstMPK){ OperateAM(rstMPK); });
        return UID();
    }
    throw std::runtime_error(str_fflprintf(": Activation twice: %s", UIDFunc::GetUIDString(UID()).c_str()));
}

void ServerObject::Deactivate()
{
    if(m_ActorPod){
        m_ActorPod->Detach([this](){ delete this; });
    }
}

void ServerObject::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    m_DelayCmdCount = m_DelayCmdQ.empty() ? 0 : (m_DelayCmdCount + 1);
    m_DelayCmdQ.emplace(nDelayTick + g_MonoServer->GetTimeTick(), m_DelayCmdCount, fnCmd);
}

void ServerObject::SetState(uint8_t nStateLoc, uint8_t nStateValue)
{
    m_StateV[nStateLoc] = nStateValue;
    m_StateTimeV[nStateLoc] = g_MonoServer->GetTimeTick();
}

uint8_t ServerObject::GetState(uint8_t nState) const
{
    return m_StateV[nState];
}

uint32_t ServerObject::GetStateTime(uint8_t nState) const
{
    return g_MonoServer->GetTimeTick() - m_StateTimeV[nState];
}
