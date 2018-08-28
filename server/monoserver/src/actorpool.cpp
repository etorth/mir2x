/*
 * =====================================================================================
 *
 *       Filename: actorpool.cpp
 *        Created: 08/16/2018 23:44:41
 *    Description:
 *                  implementation:
 *                  1. application thread won't accquire bucket lock
 *                  2. current actor thread can accquire bucket lock in w mode
 *                  3.   other actor thread can accquire bucket lock in r mode
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

#include <mutex>
#include <thread>
#include <cstdint>
#include <cinttypes>
#include "receiver.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

ActorPool::ActorPool(uint32_t nBucketCount)
    : m_BucketCount(nBucketCount > 0 ? nBucketCount : (std::thread::hardware_concurrency() * 2 + 1))
    , m_LogicFPS(60)
    , m_Terminated(false)
    , m_FutureList()
    , m_BucketList()
    , m_ReceiverLock()
    , m_ReceiverList()
{}

ActorPool::~ActorPool()
{
    if(IsActorThread()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Trying to destroy actor pool in actor thread");
        return;
    }

    m_Terminated.store(true);
    for(auto p = m_FutureList.begin(); p != m_FutureList.end(); ++p){
        p->get();
    }
    m_FutureList.clear();
}

bool ActorPool::Register(ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
        return false;
    }

    auto nUID = pActor->UID();
    auto &rstBucket = m_BucketList[nUID % m_BucketCount];
    auto pMailbox = std::make_shared<ActorMailbox>(pActor);

    // can call this function from:
    // 1. application thread
    // 2. other/current actor thread

    // exclusively lock before write
    // 1. to make sure any other reading thread done
    // 2. current thread won't accquire the lock in read mode

    const ActorPod *pExistActor = nullptr;
    {
        std::unique_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
        if(auto p = rstBucket.MailboxList.find(nUID); p != rstBucket.MailboxList.end()){
            rstBucket.MailboxList[nUID] = pMailbox;
            return true;
        }else{
            pExistActor = p->second->Actor;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "UID exists: UID = %" PRIu32 ", ActorPod = %p", nUID, pExistActor);
    return false;
}

bool ActorPool::Detach(const ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
        return false;
    }

    // we can use UID as parameter
    // but use actor address prevents other thread do detach blindly

    auto nUID = pActor->UID();
    auto &rstBucket = m_BucketList[nUID % m_BucketCount];

    auto fnDoDetach = [&rstBucket, pActor, nUID]() -> bool
    {
        if(auto p = rstBucket.MailboxList.find(nUID); (p != rstBucket.MailboxList.end()) && (p->second->Actor == pActor)){
            p->second->Actor = nullptr;
            return true;
        }
        return false;
    };

    // current thread won't lock current actor bucket in read mode
    // any other thread can only accquire in read mode

    if(std::this_thread::get_id() == rstBucket.ID){
        if(fnDoDetach()){
            return true;
        }
    }else{
        std::shared_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
        if(fnDoDetach()){
            return true;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Actor doesn't exist: (ActorPod = %p, ActorPod::UID() = %" PRIu64 ")", pActor, pActor->UID());
    return false;
}

bool ActorPool::PostMessage(uint64_t nUID, const MessagePack *pMPK, size_t nMPKLen)
{
    if(!(pMPK && nMPKLen)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Posting empty message buffer");
        return false;
    }

    auto fnOnBadPod = [this, pMPK, nMPKLen]()
    {
        for(size_t nIndex = 0; nIndex < nMPKLen; ++nIndex){
            PostMessage(pMPK[nIndex].From(), {MessageBuf(MPK_BADACTORPOD), 0, 0, pMPK[nIndex].Respond()});
        }
    };

    if(IsReceiver(nUID)){
        // try to find if there is such receiver
        // push the message buffer
        {
            std::lock_guard<std::mutex> stLockGuard(m_ReceiverLock);
            if(auto p = m_ReceiverList.find(nUID); p != m_ReceiverList.end() && p->second){
                p->second->PushMessage(pMPK, nMPKLen);
                return true;
            }
        }

        // no such receiver
        // report BAD_ACTORPOD even we know it's a receiver
        fnOnBadPod();
        return false;
    }

    auto &rstBucket = m_BucketList[nUID % m_BucketCount];
    auto fnPushMessage = [&rstBucket, pMPK, nMPKLen, nUID]()
    {
        if(auto p = rstBucket.MailboxList.find(nUID); p != rstBucket.MailboxList.end()){
            std::lock_guard<SpinLock> stLockGuard(p->second->NextQLock);
            p->second->NextQ.insert(p->second->NextQ.end(), pMPK, pMPK + nMPKLen);
            return true;
        }
        return false;
    };

    if(std::this_thread::get_id() == rstBucket.ID){
        if(fnPushMessage()){
            return true;
        }
    }else{
        std::shared_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
        if(fnPushMessage()){
            return true;
        }
    }

    for(size_t nIndex = 0; nIndex < nMPKLen; ++nIndex){
        PostMessage(pMPK[nIndex].From(), {MessageBuf(MPK_BADACTORPOD), 0, 0, pMPK[nIndex].Respond()});
    }
    return false;
}

void ActorPool::Launch()
{
    for(int nIndex = 0; nIndex < (int)(m_BucketCount); ++nIndex){
        m_FutureList[nIndex] = std::async(std::launch::async, [nIndex, this]()
        {
            auto &rstBucket = m_BucketList[nIndex];
            rstBucket.ID = std::this_thread::get_id();

            extern MonoServer *g_MonoServer;
            auto nCurrTick = g_MonoServer->GetTimeTick();

            while(!m_Terminated.load()){
                auto fnUpdate = [&rstBucket](auto pCurr)
                {
                    while(pCurr != rstBucket.MailboxList.end()){
                        if(auto pMailbox = pCurr->second; pMailbox->Actor){
                            if(pMailbox->CurrQ.empty()){
                                std::lock_guard<SpinLock> stLockGuard(pMailbox->NextQLock);
                                if(pMailbox->NextQ.empty()){
                                    ++pCurr;
                                    continue;
                                }
                                std::swap(pMailbox->CurrQ, pMailbox->NextQ);
                            }

                            for(auto p = pCurr->second->CurrQ.begin(); p != pCurr->second->CurrQ.end(); ++p){
                                pMailbox->Actor->InnHandler(*p);
                            }

                            pMailbox->Actor->InnHandler({MPK_METRONOME, 0, 0});

                            ++pCurr;
                            continue;
                        }else{
                            // detached actor
                            // need to remove it
                            return pCurr;
                        }
                    }
                    return rstBucket.MailboxList.end();
                };

                // start to update
                // if found detached actor, remove it
                for(auto p = rstBucket.MailboxList.begin(); p != rstBucket.MailboxList.end();){
                    if(p = fnUpdate(p); p == rstBucket.MailboxList.end()){
                        break;
                    }else{
                        std::unique_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
                        p = rstBucket.MailboxList.erase(p);
                    }
                }

                // sleep
                auto nExptTick = nCurrTick + 1000 / m_LogicFPS;
                nCurrTick = g_MonoServer->GetTimeTick();

                if(nCurrTick < nExptTick){
                    g_MonoServer->SleepEx(nExptTick - nCurrTick);
                }
            }

            // terminted
            // need to clean all mailboxes
            {
                std::unique_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
                rstBucket.MailboxList.clear();
            }
            return true;
        });
    }
}

bool ActorPool::CheckInvalid(uint64_t nUID) const
{
    auto &rstBucket = m_BucketList[nUID % m_BucketCount];
    auto fnGetInvalid = [this, &rstMailboxList = rstBucket.MailboxList, nUID]() -> bool
    {
        if(auto p = rstMailboxList.find(nUID); p == rstMailboxList.end() || p->second->Actor == nullptr){
            return true;
        }
        return false;
    };

    if(std::this_thread::get_id() == rstBucket.ID){
        return fnGetInvalid();
    }else{
        std::shared_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
        return fnGetInvalid();
    }
}

uint64_t ActorPool::GetInnActorUID()
{
    if(auto nUID = m_InnActorUID.fetch_add(1); nUID < 0X0000FFFFFFFFFFFF){
        return 0X00FF000000000000 + nUID;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_FATAL, "InnActorUID overflows");
    return 0;
}
