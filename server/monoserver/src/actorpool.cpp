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

#include <cstdint>
#include <cinttypes>
#include "actorpool.hpp"
#include "monoserver.hpp"

ActorPool(int nBucketCount = 0)
    : m_BucketCount(nBucketCount > 0 ? nBucketCount : (std::hardware_concurrency() * 2 + 1))
    , m_Terminated(false)
    , m_Futures();
{}

ActorPool::~ActorPool()
{
    if(IsActorThread()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Trying to destroy actor pool in actor thread.");
        return;
    }

    m_Terminated.store(true);
    for(auto p = m_Futures.begin(); p != m_Futures.end(); ++p){
        p->get();
    }
    m_Futures.clear();
}

bool ActorPool::Register(ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
        return false;
    }

    auto nUID = pActor->UID();
    auto &rstBucket = m_BucketArray[nUID % m_BucketCount];
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
        if(auto p = rstBucket.MailboxList.find(nUID); p != rstBucket.end()){
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
    auto &rstBucket = m_BucketArray[nUID % m_BucketCount];

    auto fnDoDetach = [&rstBucket, pActor]() -> bool
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
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't find (ActorPod = %p, ActorPod::UID() = %" PRIu64 ") in actor pool", pActor, pActor->UID());
    return false;
}

void ActorPool::PostMessage(uint64_t nUID, const MessagePack *pMPK, size_t nMPKLen)
{
    if(!(pMPK && nMPKLen)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_FATAL, "Posting empty message buffer");
        return;
    }

    auto fnOnBadPod = [this, pMPK, nMPKLen]()
    {
        for(size_t nIndex = 0; nIndex < nMPKLen; ++nIndex){
            PostMessage(pMPK[nIndex].From(), {MPK_BADACTORPOD, 0, pMPK[nIndex].Resp()});
        }
    };

    if(IsReceiver(nUID)){
        // try to find if there is such receiver
        // push the message buffer
        {
            std::lock_guard<std::mutex> stLockGuard(m_ReceiverLock);
            if(auto p = m_ReceiverList.find(nUID); p != m_ReceiverList.end() && p->second){
                p->second->PushMessage(pMPK, nMPKLen);
                return;
            }
        }

        // no such receiver
        // report BAD_ACTORPOD even we know it's a receiver
        fnOnBadPod();
        return;
    }

    auto &rstBucket = m_BucketArray[nUID % m_BucketCount];
    auto fnPushMessage = [&rstBucket, pMPK, nMPKLen]()
    {
        if(auto p = rstBucket.MailboxList.find(nUID); p != rstBucket.MailboxList.end()){
            std::lock_guard<SpinLock> stLockGuard(p->second.NextQLock);
            p->second.NextQ.insert(p->second.NextQ.end(), pMPK, pMPK + nMPKLen);
            return true;
        }
        return false;
    };

    if(std::this_thread::get_id() == rstBucket.ID){
        if(fnPushMessage()){
            return;
        }

        for(size_t nIndex = 0; nIndex < nMPKLen; ++nIndex){
            PostMessage(pMPK[nIndex].From(), {MPK_BADACTORPOD, 0, pMPK[nIndex].Resp()});
        }
    }else{
        std::shared_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
        if(fnPushMessage()){
            return;
        }
    }
}

void ActorPool::Launch()
{
    for(int nIndex = 0; nIndex < m_BucketCount; ++nIndex){
        m_Feature[nIndex] = std::async(std::launch::async, [nIndex, this]()
        {
            auto &rstBucket = m_BucketArray[nUID % m_BucketCount];
            rstBucket.ID = std::this_thread::get_id();

            extern MonoServer *g_MonoServer;
            auto nCurrTick = g_MonoServer->GetTimeTick();

            while(!m_Terminated.load()){
                auto fnUpdate = [&rstBucket](auto pCurr)
                {
                    while(pCurr != rstBucket.MailboxList.end()){
                        if(auto &rstMailbox = pCurr->second; rstMailbox.Actor){
                            if(rstMailbox.CurrQ.empty()){
                                std::lock_guard<SpinLock> stLockGuard(rstMailbox.NextQLock);
                                if(rstMailbox.NextQ.empty()){
                                    ++pCurr;
                                    continue;
                                }
                                std::swap(rstMailbox.CurrQ, rstMailbox.NextQ);
                            }

                            for(auto p = CurrQ.begin(); p != CurrQ.end(); ++p){
                                rstMailbox.Actor->InnHandler(*p);
                            }

                            rstMailbox.Actor->InnHandler({MPK_METRONOME, 0, 0});

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
                    if(p = fnUpdate(); p == rstBucket.MailboxList.end()){
                        break;
                    }else{
                        std::unique_lock<std::shared_lock> stLock(rstBucket.BucketLock);
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
                std::unique_lock<std::shared_lock> stLock(rstBucket.BucketLock);
                rstBucket.MailboxList.clear();
            }
            return true;
        });
    }
}

void ActorPool::CheckInvalid(uint32_t nUID) const
{
    auto &rstBucket = m_BucketArray[nUID % m_BucketCount];
    auto fnGetInvalid = [this, rstBucket, nUID] -> bool
    {
        if(auto p = rstBucket.find(nUID); p == rstBucket.end() || p->second->Actor == nullptr){
            return true;
        }
        return false;
    }

    if(std::this_thread::get_id() == rstBucket.ID){
        return fnGetInvalid();
    }else{
        std::shared_lock<std::shared_mutex> stLock(rstBucket.BucketLock);
        return fnGetInvalid();
    }
}
