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

// keep in mind:
// 1. at ANY time only one thread can access one actor message handler
// 2. at ANY time one thread must grab the SchedLock before detach the mailbox

// actor thread id marker
// only get explicit assignment in actor threads
// for any other application threads it returns the defaut value
static thread_local int t_WorkerID = ActorPool::MAILBOX_ACCESS_PUB;
static int GetWorkerID()
{
    return t_WorkerID;
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
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Trying to destroy actor pool in actor thread %d", GetWorkerID());
        g_MonoServer->Restart();
        return;
    }

    m_Terminated.store(true);
    for(auto p = m_FutureList.begin(); p != m_FutureList.end(); ++p){
        // in lanuch part if an actor thread can join
        // it already passed the unique_lock to clean all readers
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
    auto nIndex = nUID % m_BucketList.size();
    auto pMailbox = std::make_shared<Mailbox>(pActor);

    // can call this function from:
    // 1. application thread
    // 2. other/current actor thread spawning new actors

    // exclusively lock before write
    // 1. to make sure any other reading thread done
    // 2. current thread won't accquire the lock in read mode

    // remeber here the writer thread may starve, check:
    // https://stackoverflow.com/questions/2190090/how-to-prevent-writer-starvation-in-a-read-write-lock-in-pthreads

    const ActorPod *pExistActor = nullptr;
    {
        std::unique_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        if(auto p = m_BucketList[nIndex].MailboxList.find(nUID); p == m_BucketList[nIndex].MailboxList.end()){
            m_BucketList[nIndex].MailboxList[nUID] = pMailbox;
            return true;
        }else{
            pExistActor = p->second->Actor;
        }
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "UID exists: UID = %" PRIu64 ", ActorPod = %p", nUID, pExistActor);
    return false;
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

bool ActorPool::Detach(const ActorPod *pActor, const std::function<void()> &fnAtExit)
{
    if(!(pActor && pActor->UID())){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID());
        return false;
    }

    // we can use UID as parameter
    // but use actor address prevents other thread detach blindly

    auto nIndex = pActor->UID() % m_BucketList.size();
    auto fnDoDetach = [this, &rstMailboxList = m_BucketList[nIndex].MailboxList, pActor, &fnAtExit]() -> bool
    {
        // we need to make sure after this funtion
        // the isn't any threads accessing the internal actor state
        if(auto p = rstMailboxList.find(pActor->UID()); p != rstMailboxList.end()){
            uint32_t nBackoff = 0;
            while(true){
                switch(MailboxLock stMailboxLock(p->second->SchedLock, GetWorkerID()); stMailboxLock.LockType()){
                    case MAILBOX_DETACHED:
                        {
                            // we allow double detach an actor
                            // this helps to always legally call Detach() in actor destructor

                            // if found a detached actor
                            // then the actor pointer should be null
                            if(p->second->Actor){
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Detached mailbox has non-zero actor pointer: ActorPod = %p, ActorPod::UID() = %" PRIu64, p->second->Actor, pActor->UID());
                                return false;
                            }
                            return true;
                        }
                    case MAILBOX_READY:
                        {
                            // grabbed the SchedLock successfully
                            // no other thread can access the message handler before unlock

                            // only check this consistancy when grabbed the lock
                            // otherwise other thread may change the actor pointer to null at any time
                            if(p->second->Actor != pActor){
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, pActor, p->second->Actor, pActor->UID());
                                return false;
                            }

                            // detach a locked mailbox
                            // remember any thread can't flip mailbox to detach before lock it
                            if(auto nWorkerID = p->second->SchedLock.Detach(); nWorkerID != GetWorkerID()){
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Locked actor flips to invalid status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", Status = %d", pActor, pActor->UID(), nWorkerID);
                                g_MonoServer->Restart();
                                return false;
                            }

                            // call actor atexit() function immediately
                            // don't need to delay it since it's not in the actor thread
                            if(fnAtExit){
                                fnAtExit();
                            }

                            // help to never access to it
                            // detached actor outside of the pool can free itself immediately
                            p->second->Actor = nullptr;
                            return true;
                        }
                    case MAILBOX_ACCESS_PUB:
                        {
                            Backoff(nBackoff);
                            break;
                        }
                    default:
                        {
                            if(!IsActorThread(stMailboxLock.LockType())){
                                extern MonoServer *g_MonoServer;
                                g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid actor status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", status = %c", pActor, pActor->UID(), stMailboxLock.LockType());
                                g_MonoServer->Restart();
                                return false;
                            }

                            // in these actor threads
                            // if calling detach in the actor's message handler we can only mark the DETACHED status

                            if(stMailboxLock.LockType() == GetWorkerID()){
                                // only check this consistancy when grabbed the lock
                                // otherwise other thread may change the actor pointer to null at any time
                                if(p->second->Actor != pActor){
                                    extern MonoServer *g_MonoServer;
                                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, pActor, p->second->Actor, pActor->UID());
                                    return false;
                                }

                                // this is from inside the actor's actor thread
                                // have to delay the atexit() since the message handler is not done yet
                                p->second->SchedLock.Detach();
                                if(fnAtExit){
                                    p->second->AtExit = fnAtExit;
                                }

                                p->second->Actor = nullptr;
                                return true;
                            }

                            // target actor is in running status
                            // and current thread is other actor thread not driving the target actor
                            Backoff(nBackoff);
                            break;
                        }
                }
            }
        }

        // we allow double-detach an actor
        // this helps to put Detach() call in actor destructor

        // then if we can't find this actor
        // most likely it's removed already by its worker thread

        // but if call from the message handler inside
        // we still can't find it in the pool then it's an error
        // how can I check it?
        return true;
    };

    // current thread won't lock current actor bucket in read mode
    // any other thread can only accquire in read mode

    if(GetWorkerID() == (int)(nIndex)){
        return fnDoDetach();
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        return fnDoDetach();
    }
}

bool ActorPool::Detach(const Receiver *pReceiver)
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
        std::lock_guard<std::mutex> stLockGuard(m_ReceiverLock);
        if(auto p = m_ReceiverList.find(nUID); p != m_ReceiverList.end()){
            p->second->PushMessage(std::move(stMPK));
            return true;
        }
        return false;
    }

    auto nIndex = nUID % m_BucketList.size();
    auto fnPostMessage = [this, nIndex, nUID](MessagePack stMPK)
    {
        // here won't try lock the mailbox
        // but it will check if the mailbox is detached

        if(auto p = m_BucketList[nIndex].MailboxList.find(nUID); p != m_BucketList[nIndex].MailboxList.end()){
            // just a cheat and can remove it
            // try return earlier with acquire the NextQLock
            if(p->second->SchedLock.Detached()){
                return false;
            }

            // still here the mailbox can freely switch to detached status
            // need the actor thread do fully clear job
            {
                std::lock_guard<SpinLock> stLockGuard(p->second->NextQLock);
                if(p->second->SchedLock.Detached()){
                    return false;
                }
                p->second->NextQ.push_back(std::move(stMPK));
                return true;
            }
        }
        return false;
    };

    if(GetWorkerID() == (int)(nIndex)){
        return fnPostMessage(stMPK);
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        return fnPostMessage(stMPK);
    }
}

bool ActorPool::RunOneMailbox(Mailbox *pMailbox, bool bMetronome)
{
    if(!IsActorThread()){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Accessing actor message handlers outside of any actor threads: %d", GetWorkerID());
        g_MonoServer->Restart();
        return false;
    }

    // before every call to InnHandler()
    // check if it's detached in case actor call Detach() in its message handler
    //
    //     if(pActor->Detached()){       // 1. check
    //         return false;             // 2. return
    //     }                             // 3. actor can't flip to detached here
    //     pActor->InnHandler(stMPK);    // 4. handle

    // don't worry about actor detached at line-3
    // because it's in the message handling thread and it grabs the SchedLock
    // any thread want to flip the actor to detached status must firstly grab its SchedLock

    if(bMetronome){
        if(pMailbox->SchedLock.Detached()){
            return false;
        }
        pMailbox->Actor->InnHandler({MPK_METRONOME, 0, 0});
    }

    if(pMailbox->CurrQ.empty()){
        std::lock_guard<SpinLock> stLockGuard(pMailbox->NextQLock);
        if(pMailbox->NextQ.empty()){
            return true;
        }
        std::swap(pMailbox->CurrQ, pMailbox->NextQ);
    }

    // if we return in the middle
    // we may leave unhandled message in CurrQ

    for(auto p = pMailbox->CurrQ.begin(); p != pMailbox->CurrQ.end(); ++p){
        if(pMailbox->SchedLock.Detached()){
            // need to erase all handled messages: [begin, p)
            // otherwise in ClearOneMailbox() will get handled again with MPK_BADACTORPOD
            pMailbox->CurrQ.erase(pMailbox->CurrQ.begin(), p);
            return false;
        }
        pMailbox->Actor->InnHandler(*p);
    }

    pMailbox->CurrQ.clear();
    return !pMailbox->SchedLock.Detached();
}

void ActorPool::RunWorkerSteal(size_t nMaxIndex)
{
    std::shared_lock<std::shared_mutex> stLock(m_BucketList[nMaxIndex].BucketLock);
    for(auto p = m_BucketList[nMaxIndex].MailboxList.rbegin(); p != m_BucketList[nMaxIndex].MailboxList.rend(); ++p){
        if(MailboxLock stMailboxLock(p->second->SchedLock, GetWorkerID()); stMailboxLock.Locked()){
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

void ActorPool::ClearOneMailbox(Mailbox *pMailbox)
{
    // actor can detach and immedately deconstruct it self
    // when found a mailbox is detached never access it's actor pointer

    // clean one mailbox means:
    // 1. make sure all actor/application threads DONE posting to it
    // 2. make sure all actor/application threads can not post new messages to it
    // 3. make sure all queued message are responded with MPK_BADACTORPOD

    // I call this function that:
    // 1. without accquiring writer lock
    // 2. always with Mailbox.Detached() as TRUE

    // 0 // thread-1: posting message to NextQ  | 0 // thread-2: try clear all pending/sending messages
    // 1 {                                      | 1 if(Mailbox.Detached()){
    // 2     LockGuard(Mailbox.NextQLock);      | 2     {                                         // ClearOneMailbox() begins
    // 3     if(Mailbox.Detached()){            | 3         LockGuard(Mailbox.NextQLock);         //
    // 4         return false;                  | 4         Mailbox.CurrQ.Enqueue(Mailbox.NextQ); //
    // 5     }                                  | 5     }                                         //
    // 6     Mailbox.NextQ.Enqueue(MSG);        | 6                                               //
    // 7     return true                        | 7     Handle(Mailbox.CurrQ);                    // ClearOneMailbox() ends
    // 8 }                                      | 8 }

    // CurrQ is only used in main/stealing actor threads
    // what I want to make sure is:
    //
    //      when thread-2 is at or after line 4, threads like thread-1 can't reach line 6
    //
    // 1. when thread-2 is at line 4, thread-1 can't acquire the lock, simple
    // 2. when thread-2 is after line 4 (i.e. at line 7), if thread-1 is at line 6, this means that
    //
    //      a. thread-1 has accquired the lock
    //      b. at the time it accquired the lock, thread-1 didn't see mailbox detached
    // 
    // it's possible that when thread-1 reaches line 6 and sleeps, then thread-2 reaches line 2.
    // but thread-2 can't reach line 7 because it can't pass line 3

    // so we conclude if in thread-2 ClearOneMailbox() returns
    // we are sure there is no message is posting or to post message to NextQ

    {
        std::lock_guard<SpinLock> stLockGuard(pMailbox->NextQLock);
        if(!pMailbox->NextQ.empty()){
            pMailbox->CurrQ.insert(pMailbox->CurrQ.end(), pMailbox->NextQ.begin(), pMailbox->NextQ.end());
        }
    }

    for(auto pMPK = pMailbox->CurrQ.begin(); pMPK != pMailbox->CurrQ.end(); ++pMPK){
        if(pMPK->From()){
            AMBadActorPod stAMBAP;
            std::memset(&stAMBAP, 0, sizeof(stAMBAP));

            stAMBAP.Type    = pMPK->Type();
            stAMBAP.From    = pMPK->From();
            stAMBAP.ID      = pMPK->ID();
            stAMBAP.Respond = pMPK->Respond();

            PostMessage(pMPK->From(), {MessageBuf(MPK_BADACTORPOD, stAMBAP), 0, 0, pMPK->ID()});
        }
    }
    pMailbox->CurrQ.clear();
}

void ActorPool::RunWorkerOneLoop(size_t nIndex)
{
    // check if in the correct thread
    // stealing actor thread can't call this function

    if(GetWorkerID() != (int)(nIndex)){
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Accessing message handlers outside of its actor thread: WorkerID = %d, BucketID = %d", GetWorkerID(), (int)(nIndex));
        g_MonoServer->Restart();
        return;
    }

    auto fnUpdate = [this](size_t nIndex, auto p)
    {
        while(p != m_BucketList[nIndex].MailboxList.end()){
            switch(MailboxLock stMailboxLock(p->second->SchedLock, GetWorkerID()); stMailboxLock.LockType()){
                case MAILBOX_DETACHED:
                    {
                        return p;
                    }
                case MAILBOX_READY:
                    {
                        // we grabbed the mailbox successfully
                        // but it can flip to detached if call Detach() in its message handler

                        // if between message handling it flips to detached
                        // we just return immediately and leave the rest handled message there

                        // don't try clean it
                        // since we can't guarentee to clean it complately
                        if(!RunOneMailbox(p->second.get(), true)){
                            return p;
                        }

                        ++p;
                        break;
                    }
                case MAILBOX_ACCESS_PUB:
                    {
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Actor message handler executed by application thread, should never happen");
                        g_MonoServer->Restart();
                        break;
                    }
                default:
                    {
                        if(!IsActorThread(stMailboxLock.LockType())){
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid actor status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", status = %d", p->second->Actor, p->second->Actor->UID(), stMailboxLock.LockType());
                            g_MonoServer->Restart();
                            return p;
                        }

                        // shouldn't by current actor thread
                        // the mailbox is running in work stealing thread

                        if(stMailboxLock.LockType() == GetWorkerID()){
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Incorrect actor worker id: %d", stMailboxLock.LockType());
                            g_MonoServer->Restart();
                            return p;
                        }

                        // if a mailbox grabbed by an actor thread
                        // the thead will handle all its queued message
                        ++p;
                        break;
                    }
            }
        }
        return m_BucketList[nIndex].MailboxList.end();
    };

    for(auto p = fnUpdate(nIndex, m_BucketList[nIndex].MailboxList.begin()); p != m_BucketList[nIndex].MailboxList.end(); p = fnUpdate(nIndex, p)){
        // we detected a detached actor
        // clear all its queued messages and remove it if ready
        if(p->second->AtExit){
            p->second->AtExit();
            p->second->AtExit = {};
        }
        ClearOneMailbox(p->second.get());
        {
            std::unique_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
            p = m_BucketList[nIndex].MailboxList.erase(p);
        }
    }
}

void ActorPool::Launch()
{
    for(int nIndex = 0; nIndex < (int)(m_BucketList.size()); ++nIndex){
        m_FutureList.emplace_back(std::async(std::launch::async, [nIndex, this]()
        {
            // record the worker id
            // for application this won't get assigned
            t_WorkerID = nIndex;

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

        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_INFO, "Actor thread %d launched", nIndex);
    }
}

bool ActorPool::CheckInvalid(uint64_t nUID) const
{
    auto nIndex = nUID % m_BucketList.size();
    auto fnGetInvalid = [this, &rstMailboxList = m_BucketList[nIndex].MailboxList, nUID]() -> bool
    {
        if(auto p = rstMailboxList.find(nUID); p == rstMailboxList.end() || p->second->SchedLock.Detached()){
            return true;
        }
        return false;
    };

    if(GetWorkerID() == (int)(nIndex)){
        return fnGetInvalid();
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        return fnGetInvalid();
    }
}

bool ActorPool::IsActorThread() const
{
    return IsActorThread(GetWorkerID());
}

bool ActorPool::IsActorThread(int nWorkerID) const
{
    return (nWorkerID >= 0) && (nWorkerID < (int)(m_BucketList.size()));
}
