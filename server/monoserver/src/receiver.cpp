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
    if(!g_actorPool->attach(this)){
        g_monoServer->addLog(LOGTYPE_FATAL, "ActorPool::attach(Reciver = %p) failed", this);
    }
}

Receiver::~Receiver()
{
    if(!g_actorPool->detach(this)){
        g_monoServer->addLog(LOGTYPE_FATAL, "ActorPool::detach(Reciver = %p) failed", this);
    }
}

void Receiver::PushMessage(MessagePack stMPK)
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

std::vector<MessagePack> Receiver::Pop()
{
    std::vector<MessagePack> stvPop;
    {
        std::lock_guard<std::mutex> stLockGuard(m_lock);
        std::swap(stvPop, m_messageList);
    }
    return stvPop;
}
