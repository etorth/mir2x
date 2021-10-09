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
    : m_uid(uidf::buildReceiverUID())
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

void Receiver::pushMessage(ActorMsgPack mpk)
{
    {
        std::lock_guard<std::mutex> lockGuard(m_lock);
        m_messageList.push_back(std::move(mpk));
    }
    m_condition.notify_all();
}

size_t Receiver::wait(uint32_t timeout)
{
    std::unique_lock<std::mutex> lockGuard(m_lock);
    const auto fnPred = [this, origLen = m_messageList.size()]() -> bool
    {
        return m_messageList.size() > origLen;
    };

    if(timeout){
        m_condition.wait_for(lockGuard, std::chrono::milliseconds(timeout), fnPred);
    }
    else{
        m_condition.wait(lockGuard, fnPred);
    }
    return m_messageList.size();
}

std::vector<ActorMsgPack> Receiver::pop()
{
    std::vector<ActorMsgPack> popList;
    {
        std::lock_guard<std::mutex> lockGuard(m_lock);
        std::swap(popList, m_messageList);
    }
    return popList;
}
