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
{
    m_stateTrigger.install([this]() -> bool
    {
        if(!m_delayCmdQ.empty()){
            if(g_monoServer->getCurrTick() >= m_delayCmdQ.top().tick()){
                m_delayCmdQ.top()();
                m_delayCmdQ.pop();
            }
        }
        return false;
    });

    if(g_serverArgParser->traceActorMessage){
        m_stateTrigger.install([this, lastCheckTick = to_u32(0)]() mutable -> bool
        {
            if(const auto currTick = g_monoServer->getCurrTick(); lastCheckTick + 1000 < currTick){
                if(checkActorPod()){
                    m_actorPod->PrintMonitor();
                }
                lastCheckTick = currTick;
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
            m_stateTrigger.run();
        },

        [this](const ActorMsgPack &mpk)
        {
            operateAM(mpk);
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

void ServerObject::addDelay(uint32_t delayTick, std::function<void()> cmd)
{
    m_delayCmdIndex = m_delayCmdQ.empty() ? 0 : (m_delayCmdIndex + 1);
    m_delayCmdQ.emplace(delayTick + g_monoServer->getCurrTick(), m_delayCmdIndex, std::move(cmd));
}

void ServerObject::forwardNetPackage(uint64_t uid, uint8_t type, const void *buf, size_t bufLen)
{
    fflassert(uid != UID());
    fflassert(uidf::getUIDType(uid) == UID_PLY);

    AMSendPackage amSP;
    std::memset(&amSP, 0, sizeof(amSP));

    buildActorDataPackage(&(amSP.package), type, buf, bufLen);
    m_actorPod->forward(uid, {AM_SENDPACKAGE, amSP});
}
