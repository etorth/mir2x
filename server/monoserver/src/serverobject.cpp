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
#include "actorpool.hpp"
#include "uidf.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

ServerObject::ServerObject(uint64_t uid)
    : m_UID(uid)
    , m_UIDName(uidf::getUIDString(uid))
    , m_stateV()
    , m_stateTimeV()
    , m_actorPod(nullptr)
    , m_stateHook()
    , m_delayCmdCount(0)
    , m_delayCmdQ()
{
    m_stateV.fill(0);
    m_stateTimeV.fill(0);

    m_stateHook.Install("DelayCmdQueue", [this]() -> bool
    {
        if(!m_delayCmdQ.empty()){
            if(g_monoServer->getCurrTick() >= m_delayCmdQ.top().Tick()){
                m_delayCmdQ.top()();
                m_delayCmdQ.pop();
            }
        }
        return false;
    });

    if(g_serverArgParser->TraceActorMessage){
        m_stateHook.Install("ReportActorPodMonitor", [this, nLastCheckTick = (uint32_t)(0)]() mutable -> bool
        {
            if(auto nCurrCheckTick = g_monoServer->getCurrTick(); nLastCheckTick + 1000 < nCurrCheckTick){
                if(checkActorPod()){
                    m_actorPod->PrintMonitor();
                }
                nLastCheckTick = nCurrCheckTick;
            }
            return false;
        });
    }
}

ServerObject::~ServerObject()
{
    delete m_actorPod;
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
uint64_t ServerObject::activate()
{
    if(m_actorPod){
        throw fflerror("activation twice: %s", uidf::getUIDString(UID()).c_str());
    }

    m_actorPod = new ActorPod
    {
        m_UID,
        [this]()
        {
            m_stateHook.Execute();
        },

        [this](const MessagePack &rstMPK)
        {
            operateAM(rstMPK);
        },

        3600 * 1000,
    };

    // seperate attach call
    // this triggers the startup callback, i.e. the onActivate()
    // if automatically call attach() in ActorPod::ctor() then m_actorPod is invalid yet

    m_actorPod->attach([this]()
    {
        onActivate();
    });
    return UID();
}

void ServerObject::deactivate()
{
    if(m_actorPod){
        m_actorPod->detach([this](){ delete this; });
    }
}

void ServerObject::Delay(uint32_t nDelayTick, const std::function<void()> &fnCmd)
{
    m_delayCmdCount = m_delayCmdQ.empty() ? 0 : (m_delayCmdCount + 1);
    m_delayCmdQ.emplace(nDelayTick + g_monoServer->getCurrTick(), m_delayCmdCount, fnCmd);
}
