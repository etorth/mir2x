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

#include "receiver.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

Receiver::Receiver()
    : m_UID([]() -> uint64_t
      {
          static std::atomic<uint64_t> s_RecvUID {1};
          return s_RecvUID.fetch_add(1);
      }())
    , m_Lock()
    , m_Condition()
    , m_MessageList()
{
    extern ActorPool *g_ActorPool;
    if(!g_ActorPool->Register(this)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->addLog(LOGTYPE_FATAL, "ActorPool::Register(Reciver = %p) failed", this);
    }
}

Receiver::~Receiver()
{
    extern ActorPool *g_ActorPool;
    if(!g_ActorPool->Detach(this)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->addLog(LOGTYPE_FATAL, "ActorPool::Detach(Reciver = %p) failed", this);
    }
}

void Receiver::PushMessage(MessagePack stMPK)
{
    std::unique_lock<std::mutex> stLock(m_Lock);
    m_MessageList.push_back(std::move(stMPK));
    m_Condition.notify_all();
}

size_t Receiver::Wait(uint32_t nTimeout)
{
    std::unique_lock<std::mutex> stLock(m_Lock);
    auto fnPred = [this, nOrigLen = m_MessageList.size()]() -> bool
    {
        return m_MessageList.size() > nOrigLen;
    };

    if(nTimeout){
        m_Condition.wait_for(stLock, std::chrono::milliseconds(nTimeout), fnPred);
    }else{
        m_Condition.wait(stLock, fnPred);
    }

    return m_MessageList.size();
}

std::vector<MessagePack> Receiver::Pop()
{
    std::vector<MessagePack> stvPop;
    {
        std::lock_guard<std::mutex> stLockGuard(m_Lock);
        std::swap(stvPop, m_MessageList);
    }
    return stvPop;
}
