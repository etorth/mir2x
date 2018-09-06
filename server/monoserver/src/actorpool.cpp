/*
 * =====================================================================================
 *
 *       Filename: actorpool.cpp
 *        Created: 09/02/2018 19:07:15
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

#include <mutex>
#include <thread>
#include <cstdint>
#include <cinttypes>
#include "receiver.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

thread_local uint32_t g_WorkerID = -1;
static uint32_t GetWorkerID()
{
    return g_WorkerID;
}

ActorPool::ActorPool(uint32_t nBucketCount, uint32_t nLogicFPS)
    : m_LogicFPS(nLogicFPS)
    , m_Terminated(false)
    , m_FutureList()
    , m_BucketList(nBucketCount)
    , m_ReceiverLock()
    , m_ReceiverList()
{}

ActorPool::~ActorPool()
{
    if(IsActorThread()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Trying to destroy actor pool in actor thread");
        g_MonoServer->Restart();
        return;
    }

    m_Terminated.store(true);
    for(auto p = m_FutureList.begin(); p != m_FutureList.end(); ++p){
        p->get();
    }
    m_FutureList.clear();
}

ActorPool::Mailbox *ActorPool::Register(ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
        return nullptr;
    }

    auto nUID = pActor->UID();
    auto nIndex = nUID % m_BucketList.size();
    auto pMailbox = std::make_shared<Mailbox>(pActor);

    // can call this function from:
    // 1. application thread
    // 2. other/current actor thread

    // exclusively lock before write
    // 1. to make sure any other reading thread done
    // 2. current thread won't accquire the lock in read mode

    const ActorPod *pExistActor = nullptr;
    {
        std::unique_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        if(auto p = m_BucketList[nIndex].MailboxList.find(nUID); p == m_BucketList[nIndex].MailboxList.end()){
            m_BucketList[nIndex].MailboxList[nUID] = pMailbox;
            return pMailbox.get();
        }else{
            pExistActor = p->second->Actor;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "UID exists: UID = %" PRIu64 ", ActorPod = %p", nUID, pExistActor);
    return nullptr;
}

bool ActorPool::Register(Receiver *pReceiver)
{
    if(!pReceiver){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invlaid argument: Receiver = %p", pReceiver);
        return false;
    }

    Receiver *pExistReceiver = nullptr;
    {
        std::lock_guard<std::mutex> stLock(m_ReceiverLock);
        if(auto p = m_ReceiverList.find(pReceiver->UID()); p == m_ReceiverList.end()){
            m_ReceiverList[pReceiver->UID()] = pReceiver;
            return true;
        }else{
            pExistReceiver = p->second;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "UID exists: UID = %" PRIu64 ", Receiver = %p", pExistReceiver->UID(), pExistReceiver);
    return false;
}

bool ActorPool::Remove(const ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
        return false;
    }

    // we can use UID as parameter
    // but use actor address prevents other thread do remove blindly

    auto nUID = pActor->UID();
    auto nIndex = nUID % m_BucketList.size();
    auto fnDoRemove = [&rstMailboxList = m_BucketList[nIndex].MailboxList, pActor, nUID]() -> bool
    {
        if(auto p = rstMailboxList.find(nUID); (p != rstMailboxList.end())){
            if(p->second->Actor != pActor){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, pActor, p->second->Actor, pActor->UID());
                return false;
            }

            // we need to make sure after this funtion
            // the isn't any threads accessing the internal actor state

            uint32_t nBackoff = 0;
            while(true){
                switch(MailboxLock stMailboxLock(*(p->second.get())); stMailboxLock.LockType()){
                    case 'B':
                        {
                            Backoff(nBackoff);
                            break;
                        }
                    case 'R':
                        {
                            if(char chStatus = p->second->Status.exchange('D'); chStatus != 'B'){
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Locked actor has no BUSY status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", Status = %c", pActor, pActor->UID(), chStatus);
                                return false;
                            }
                            return true;
                        }
                    case 'D':
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Double-remove an actor: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
                            return false;
                        }
                    default:
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid actor status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", status = %c", pActor, pActor->UID(), stMailboxLock.LockType());
                            return false;
                        }
                }
            }
        }
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Actor doesn't exist: (ActorPod = %p, ActorPod::UID() = %" PRIu64 ")", pActor, pActor->UID());
        return false;
    };

    // current thread won't lock current actor bucket in read mode
    // any other thread can only accquire in read mode

    if(std::this_thread::get_id() == m_BucketList[nIndex].WorkerID){
        return fnDoRemove();
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        return fnDoRemove();
    }
}

bool ActorPool::Remove(const Receiver *pReceiver)
{
    if(!(pReceiver && pReceiver->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: Receiver = %p, Receiver::UID() = %" PRIu64, pReceiver, pReceiver->UID());
        return false;
    }

    {
        std::lock_guard<std::mutex> stLock(m_ReceiverLock);
        if(auto p = m_ReceiverList.find(pReceiver->UID()); (p != m_ReceiverList.end()) && (p->second == pReceiver)){
            m_ReceiverList.erase(p);
            return true;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Receiver doesn't exist: (Receiver = %p, Receiver::UID() = %" PRIu64 ")", pReceiver, pReceiver->UID());
    return false;
}

bool ActorPool::PostMessage(uint64_t nUID, MessagePack stMPK)
{
    if(!nUID){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Sending %s to zero UID", stMPK.Name());
        return false;
    }

    if(!stMPK){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Sending empty message to UID = %" PRIu64, nUID);
        return false;
    }

    if(IsReceiver(nUID)){
        {
            std::lock_guard<std::mutex> stLockGuard(m_ReceiverLock);
            if(auto p = m_ReceiverList.find(nUID); p != m_ReceiverList.end()){
                p->second->PushMessage(std::move(stMPK));
                return true;
            }
        }

        if(stMPK.From()){
            PostMessage(stMPK.From(), {MessageBuf(MPK_BADACTORPOD), 0, 0, stMPK.Respond()});
        }
        return false;
    }

    auto nIndex = nUID % m_BucketList.size();
    auto fnPushMessage = [this, nIndex, nUID](MessagePack stMPK)
    {
        if(auto p = m_BucketList[nIndex].MailboxList.find(nUID); p != m_BucketList[nIndex].MailboxList.end()){
            switch(auto chStatus = p->second.Status.load()){
                case 'D':
                    {
                        return false;
                    }
                default:
                    {
                        std::lock_guard<SpinLock> stLockGuard(p->second->NextQLock);
                        p->second->NextQ.push_back(std::move(stMPK));
                        return true;
                    }
            }
        }
        return false;
    };

    if(std::this_thread::get_id() == m_BucketList[nIndex].WorkerID){
        if(fnPushMessage(std::move(stMPK))){
            return true;
        }
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        if(fnPushMessage(std::move(stMPK))){
            return true;
        }
    }

    if(stMPK.From()){
        PostMessage(stMPK.From(), {MessageBuf(MPK_BADACTORPOD), 0, 0, stMPK.Respond()});
    }
    return false;
}

void ActorPool::RunOneMailbox(Mailbox *pMailbox, bool bMetronome)
{
    if(bMetronome){
        pMailbox->Actor->InnHandler({MPK_METRONOME, 0, 0});
    }

    if(pMailbox->CurrQ.empty()){
        std::lock_guard<SpinLock> stLockGuard(pMailbox->NextQLock);
        if(pMailbox->NextQ.empty()){
            return;
        }
        std::swap(pMailbox->CurrQ, pMailbox->NextQ);
    }

    for(auto p = pMailbox->CurrQ.begin(); p != pMailbox->CurrQ.end(); ++p){
        pMailbox->Actor->InnHandler(*p);
    }
    pMailbox->CurrQ.clear();
}

void ActorPool::RunWorkerSteal(size_t nMaxIndex)
{
    std::shared_lock<std::shared_mutex> stLock(m_BucketList[nMaxIndex].BucketLock);
    for(auto p = m_BucketList[nMaxIndex].MailboxList.rbegin(); p != m_BucketList[nMaxIndex].MailboxList.rend(); ++p){
        if(MailboxLock stMailboxLock(*(p->second.get())); stMailboxLock.Locked()){
            RunOneMailbox(p->second.get(), false);
        }
    }
}

std::tuple<long, size_t> ActorPool::CheckWorkerTime() const
{
    long nSum = 0;
    size_t nMaxIndex = 0;

    for(int nIndex = 0; nIndex < (int)(m_BucketList.size()); ++nIndex){
        nSum += m_BucketList[nIndex].RunTimer.GetAvgTime();
        if(m_BucketList[nIndex].RunTimer.GetAvgTime() > m_BucketList[nMaxIndex].RunTimer.GetAvgTime()){
            nMaxIndex = nIndex;
        }
    }
    return {nSum / m_BucketList.size(), nMaxIndex};
}

void ActorPool::RunWorker(size_t nIndex)
{
    extern MonoServer *g_MonoServer;
    auto stBeginRun = g_MonoServer->GetTimeNow();
    {
        RunWorkerOneLoop(nIndex);
    }
    m_BucketList[nIndex].RunTimer.Push(g_MonoServer->GetTimeDiff(stBeginRun, "ns"));

    if(HasWorkSteal()){
        auto [nAvgTime, nMaxIndex] = CheckWorkerTime();
        if(m_BucketList[nIndex].RunTimer.GetAvgTime() < nAvgTime){
            extern MonoServer *g_MonoServer;
            auto stBeginSteal = g_MonoServer->GetTimeNow();
            {
                RunWorkerSteal(nMaxIndex);
            }
            m_BucketList[nIndex].StealTimer.Push(g_MonoServer->GetTimeDiff(stBeginSteal, "ns"));
        }
    }
}

void ActorPool::RunWorkerOneLoop(size_t nIndex)
{
    auto fnUpdate = [this](size_t nIndex, auto p)
    {
        while(p != m_BucketList[nIndex].MailboxList.end()){
            switch(MailboxLock stLock(*(p->second.get())); stLock.LockType()){
                case 'R':
                    {
                        RunOneMailbox(p->second.get(), true);
                        ++p;
                        break;
                    }
                case 'B':
                    {
                        break;
                    }
                case 'D':
                    {
                        return p;
                    }
                default:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Unexpected mailbox detected: %c", stLock.LockType());
                        g_MonoServer->Restart();
                        break;
                    }
            }
        }
        return m_BucketList[nIndex].MailboxList.end();
    };

    for(auto p = fnUpdate(nIndex, m_BucketList[nIndex].MailboxList.begin()); p != m_BucketList[nIndex].MailboxList.end(); p = fnUpdate(nIndex, p)){
        std::unique_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        p = m_BucketList[nIndex].MailboxList.erase(p);
    }
}

void ActorPool::Launch()
{
    for(int nIndex = 0; nIndex < (int)(m_BucketList.size()); ++nIndex){
        m_FutureList.emplace_back(std::async(std::launch::async, [nIndex, this]()
        {
            m_BucketList[nIndex].WorkerID = std::this_thread::get_id();

            extern MonoServer *g_MonoServer;
            auto nCurrTick = g_MonoServer->GetTimeTick();

            while(!m_Terminated.load()){
                RunWorker(nIndex);

                auto nExptTick = nCurrTick + 1000 / m_LogicFPS;
                nCurrTick = g_MonoServer->GetTimeTick();

                if(nCurrTick < nExptTick){
                    g_MonoServer->SleepEx(nExptTick - nCurrTick);
                }
            }

            // terminted
            // need to clean all mailboxes
            {
                std::unique_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
                m_BucketList[nIndex].MailboxList.clear();
            }
            return true;
        }).share());
    }
}

bool ActorPool::CheckInvalid(uint64_t nUID) const
{
    auto nIndex = nUID % m_BucketList.size();
    auto fnGetInvalid = [this, &rstMailboxList = m_BucketList[nIndex].MailboxList, nUID]() -> bool
    {
        if(auto p = rstMailboxList.find(nUID); p == rstMailboxList.end() || p->second->Status.load() == 'D'){
            return true;
        }
        return false;
    };

    if(std::this_thread::get_id() == m_BucketList[nIndex].WorkerID){
        return fnGetInvalid();
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        return fnGetInvalid();
    }
}
