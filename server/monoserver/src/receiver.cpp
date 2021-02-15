/*
 * =====================================================================================
 *
 *       Filename: receiver.cpp
 *        Created: 08/23/2018 04:46:42
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

#include "uidf.hpp"
#include "receiver.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;

Receiver::Receiver()
    : m_UID(uidf::buildReceiverUID())
    , m_lock()
    , m_condition()
    , m_messageList()
{
    g_actorPool->attach(this);
}

Receiver::~Receiver()
{
    try{
        g_actorPool->detach(this);
    }
    catch(...){
        g_monoServer->propagateException();
    }
}

void Receiver::pushMessage(ActorMsgPack stMPK)
{
    std::unique_lock<std::mutex> stLock(m_lock);
    m_messageList.push_back(std::move(stMPK));
    m_condition.notify_all();
}

size_t Receiver::Wait(uint32_t nTimeout)
{
    std::unique_lock<std::mutex> stLock(m_lock);
    auto fnPred = [this, nOrigLen = m_messageList.size()]() -> bool
    {
        return m_messageList.size() > nOrigLen;
    };

    if(nTimeout){
        m_condition.wait_for(stLock, std::chrono::milliseconds(nTimeout), fnPred);
    }else{
        m_condition.wait(stLock, fnPred);
    }

    return m_messageList.size();
}

std::vector<ActorMsgPack> Receiver::Pop()
{
    std::vector<ActorMsgPack> stvPop;
    {
        std::lock_guard<std::mutex> stLockGuard(m_lock);
        std::swap(stvPop, m_messageList);
    }
    return stvPop;
}
