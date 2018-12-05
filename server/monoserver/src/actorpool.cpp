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
#include "hrestimer.hpp"
#include "receiver.hpp"
#include "actorpod.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

extern MonoServer *g_MonoServer;

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

ActorPool::Mailbox::Mailbox(ActorPod *pActor)
    : Actor(pActor)
    , SchedLock()
    , NextQLock()
    , CurrQ()
    , NextQ()
    , AtExit()
    , Monitor(pActor->UID())
{}

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
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Trying to destroy actor pool in actor thread %d", GetWorkerID());
        g_MonoServer->Restart();
        return;
    }

    // don't throw in dtor
    // can only put the log for diag

    m_Terminated.store(true);
    try{
        for(auto p = m_FutureList.begin(); p != m_FutureList.end(); ++p){
            // in lanuch part if an actor thread can join
            // it already passed the unique_lock to clean all readers
            p->get();
        }
    }catch(...){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Exceptions should be caught before exiting actor threads");
        g_MonoServer->Restart();
    }
    m_FutureList.clear();
}

bool ActorPool::Register(ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        throw std::invalid_argument(str_ffl_printf(": Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID()));
    }

    auto nUID = pActor->UID();
    auto nIndex = nUID % m_BucketList.size();
    auto pMailbox = std::make_shared<Mailbox>(pActor);

    // can call this function from:
    // 1. application thread
    // 2. other/current actor thread spawning new actors

    // remeber here the writer thread may starve, check:
    // https://stackoverflow.com/questions/2190090/how-to-prevent-writer-starvation-in-a-read-write-lock-in-pthreads

    // exclusively lock before write
    // 1. to make sure any other reading thread done
    // 2. current thread won't accquire the lock in read mode
    {
        std::unique_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        if(auto p = m_BucketList[nIndex].MailboxList.find(nUID); p == m_BucketList[nIndex].MailboxList.end()){
            m_BucketList[nIndex].MailboxList[nUID] = pMailbox;
            return true;
        }else{
            throw std::runtime_error(str_ffl_printf(": UID exists: UID = %" PRIu64 ", ActorPod = %p", nUID, p->second->Actor));
        }
    }
}

bool ActorPool::Register(Receiver *pReceiver)
{
    if(!pReceiver){
        throw std::invalid_argument(str_ffl_printf(": Invlaid argument: Receiver = %p", pReceiver));
    }

    {
        std::lock_guard<std::mutex> stLock(m_ReceiverLock);
        if(auto p = m_ReceiverList.find(pReceiver->UID()); p == m_ReceiverList.end()){
            m_ReceiverList[pReceiver->UID()] = pReceiver;
            return true;
        }else{
            throw std::runtime_error(str_ffl_printf(": UID exists: UID = %" PRIu64 ", Receiver = %p", p->second->UID(), p->second));
        }
    }
}

bool ActorPool::Detach(const ActorPod *pActor, const std::function<void()> &fnAtExit)
{
    if(!(pActor && pActor->UID())){
        throw std::invalid_argument(str_ffl_printf(": Invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, pActor, pActor->UID()));
    }

    // we can use UID as parameter
    // but use actor address prevents other thread detach blindly

    auto nIndex = pActor->UID() % m_BucketList.size();
    auto fnDoDetach = [this, &rstMailboxList = m_BucketList[nIndex].MailboxList, pActor, &fnAtExit]() -> bool
    {
        // we need to make sure after this funtion
        // there isn't any threads accessing the internal actor state
        if(auto p = rstMailboxList.find(pActor->UID()); p != rstMailboxList.end()){
            uint32_t nBackoff = 0;
            while(true){
                switch(MailboxLock stMailboxLock(p->second->SchedLock, GetWorkerID()); stMailboxLock.LockType()){
                    case MAILBOX_DETACHED:
                        {
                            // we allow double detach an actor
                            // this helps to always legally call Detach() in actor destructor

                            // if found a detached actor
                            // then the actor pointer must already be null
                            if(p->second->Actor){
                                throw std::runtime_error(str_ffl_printf(": Detached mailbox has non-zero actor pointer: ActorPod = %p, ActorPod::UID() = %" PRIu64, p->second->Actor, pActor->UID()));
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
                                throw std::runtime_error(str_ffl_printf(": Different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, pActor, p->second->Actor, pActor->UID()));
                            }

                            // detach a locked mailbox
                            // remember any thread can't flip mailbox to detach before lock it!!!
                            if(auto nWorkerID = p->second->SchedLock.Detach(); nWorkerID != GetWorkerID()){
                                throw std::runtime_error(str_ffl_printf(": Locked actor flips to invalid status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", Status = %d", pActor, pActor->UID(), nWorkerID));
                            }

                            // we are all good, call actor atexit() function immediately now here
                            // don't need to delay it since it's not in the actor thread
                            //                                     ^^^^^^^^^^^^^^^^^^^^^

                            // 12/04/2018: i can't understand last line of comment by myself...
                            // I guess it means: since we successfully grab the lock, there is no actor thread refers to the actor instance/pointer
                            // then it's safe to call the AtExit function, which may call ``delete this"

                            // or maybe previously I assume only actor itself can call this->Detach()
                            // then it must not be in actor thread since we can grab its lock
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
                            // allow public thread to access the SchedLock
                            // but it should never ever execute the actor handler

                            // I was planning to forbid any public thread to access the SchedLock, but 
                            //     1. Detach outside of actor threads
                            //     2. Query actor status
                            // needs to access the SchedLock

                            Backoff(nBackoff);
                            break;
                        }
                    default:
                        {
                            // can't grab the SchedLock, means someone else is accessing it
                            // and we know it's not public threads accessing, then must be actor threads
                            if(!IsActorThread(stMailboxLock.LockType())){
                                throw std::runtime_error(str_ffl_printf(": Invalid actor status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", status = %c", pActor, pActor->UID(), stMailboxLock.LockType()));
                            }

                            // inside actor threads
                            // if calling detach in the actor's message handler we can only mark the DETACHED status

                            if(stMailboxLock.LockType() == GetWorkerID()){
                                // the lock is grabed already by current actor thread, check the consistancy
                                // only check it when current thread grabs the lock, otherwise other thread may change it to null at any time
                                if(p->second->Actor != pActor){
                                    throw std::runtime_error(str_ffl_printf(": Different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, pActor, p->second->Actor, pActor->UID()));
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
                            // and current thread is not the the one grabs the lock, wait till it release the lock
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
        // we still can't find it in the pool then it's an error, how can I check it?
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
        throw std::invalid_argument(str_ffl_printf(": Invalid arguments: Receiver = %p, Receiver::UID() = %" PRIu64, pReceiver, pReceiver->UID()));
    }

    // we allow detach a receiver multiple times
    // since we support this for actorpod, no better reason...
    {
        std::lock_guard<std::mutex> stLock(m_ReceiverLock);
        if(auto p = m_ReceiverList.find(pReceiver->UID()); (p != m_ReceiverList.end()) && (p->second == pReceiver)){
            m_ReceiverList.erase(p);
        }
        return true;
    }
}

bool ActorPool::PostMessage(uint64_t nUID, MessagePack stMPK)
{
    if(!nUID){
        throw std::invalid_argument(str_ffl() + str_printf(": Sending %s to zero UID", stMPK.Name()));
    }

    if(!stMPK){
        throw std::invalid_argument(str_ffl() + str_printf(": Sending empty message to %" PRIu64, nUID));
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
        throw std::runtime_error(str_ffl_printf(": Accessing actor message handlers outside of any actor threads: %d", GetWorkerID()));
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

        {
            hres_timer stTimer(&(pMailbox->Monitor.ProcTick));
            pMailbox->Actor->InnHandler({MPK_METRONOME, 0, 0});
        }
        pMailbox->Monitor.MessageDone.fetch_add(1);
    }

    if(pMailbox->CurrQ.empty()){
        std::lock_guard<SpinLock> stLockGuard(pMailbox->NextQLock);
        if(pMailbox->NextQ.empty()){
            return true;
        }
        std::swap(pMailbox->CurrQ, pMailbox->NextQ);
    }

    // this is a good place to store the pending message size
    // during the *normal* runtime, no one will push mesages to CurrQ, all to the NextQ
    pMailbox->Monitor.MessagePending.store(pMailbox->CurrQ.size());

    // if we return in the middle
    // we may leave unhandled message in CurrQ

    for(auto p = pMailbox->CurrQ.begin(); p != pMailbox->CurrQ.end(); ++p){
        if(pMailbox->SchedLock.Detached()){
            // need to erase all handled messages: [begin, p)
            // otherwise in ClearOneMailbox() will get handled again with MPK_BADACTORPOD
            pMailbox->CurrQ.erase(pMailbox->CurrQ.begin(), p);
            return false;
        }

        {
            hres_timer stTimer(&(pMailbox->Monitor.ProcTick));
            pMailbox->Actor->InnHandler(*p);
        }
        pMailbox->Monitor.MessageDone.fetch_add(1);
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
    hres_timer stHRTimer;
    RunWorkerOneLoop(nIndex);
    m_BucketList[nIndex].RunTimer.Push(stHRTimer.diff_nsec());

    if(HasWorkSteal()){
        auto [nAvgTime, nMaxIndex] = CheckWorkerTime();
        if((m_BucketList[nIndex].RunTimer.GetAvgTime()) < nAvgTime && (nIndex != nMaxIndex)){
            hres_timer stHRStealTimer;
            RunWorkerSteal(nMaxIndex);
            m_BucketList[nIndex].StealTimer.Push(stHRStealTimer.diff_nsec());
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
        throw std::invalid_argument(str_ffl_printf(": Accessing message handler outside of its dedicated actor thread: WorkerID = %d, BucketID = %d", GetWorkerID(), (int)(nIndex)));
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
                        // we need to allow public threads accessing for
                        //    1. public thread can detach it by force
                        //    2. public thread can query actorpod statistics
                        // but should I spin here or just leave and try next mailbox ???
                        ++p;
                        break;
                    }
                default:
                    {
                        if(!IsActorThread(stMailboxLock.LockType())){
                            throw std::runtime_error(str_ffl_printf(": Invalid actor status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", status = %d", p->second->Actor, p->second->Actor->UID(), stMailboxLock.LockType()));
                        }

                        // current dedicated *actor* thread has already grabbed this mailbox lock
                        // means 1. recursively call this RunWorkerOneLoop()
                        //       2. forget to release the lock

                        if(stMailboxLock.LockType() == GetWorkerID()){
                            throw std::runtime_error(str_ffl_printf(": Mailbox sched_lock has already been grabbed by current thread: %d", stMailboxLock.LockType()));
                        }

                        // if a mailbox grabbed by some other actor thread
                        // we skip it and try next one, the grbbing thread will handle all its queued message
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
            try{
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
            }catch(...){
                g_MonoServer->PropagateException();
            }

            // won't let the std::future get the exception
            // keep it clear since no polling on std::future::get() in main thread
            return true;
        }).share());

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

ActorPool::ActorMonitor ActorPool::GetActorMonitor(uint64_t nUID) const
{
    if(IsActorThread()){
        throw std::runtime_error(str_ffl_printf(": Querying actor monitor inside actor thread: WorkerID = %d, UID = %" PRIu64, GetWorkerID(), nUID));
    }

    auto nIndex = nUID % m_BucketList.size();
    {
        // lock the bucket
        // other thread can detach the actor but can't erase the mailbox
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);

        // just read the mailbox cached variables
        // I don't need to acquire the sched_lock, I don't need it 100% accurate
        if(auto p = m_BucketList[nIndex].MailboxList.find(nUID); (p != m_BucketList[nIndex].MailboxList.end()) && (!p->second->SchedLock.Detached())){
            return p->second->DumpMonitor();
        }
        return {};
    }
}

std::vector<ActorPool::ActorMonitor> ActorPool::GetActorMonitor() const
{
    if(IsActorThread()){
        throw std::runtime_error(str_ffl_printf(": Querying actor monitor inside actor thread: WorkerID = %d", GetWorkerID()));
    }

    std::vector<ActorPool::ActorMonitor> stRetList;
    for(size_t nIndex = 0; nIndex < m_BucketList.size(); ++nIndex){
        std::shared_lock<std::shared_mutex> stLock(m_BucketList[nIndex].BucketLock);
        {
            if(auto nMostNeed = stRetList.size() + m_BucketList[nIndex].MailboxList.size(); nMostNeed > stRetList.capacity()){
                stRetList.reserve(nMostNeed);
            }

            for(auto p = m_BucketList[nIndex].MailboxList.begin(); p != m_BucketList[nIndex].MailboxList.end(); ++p){
                if(!p->second->SchedLock.Detached()){
                    stRetList.emplace_back(p->second->DumpMonitor());
                }
            }
        }
    }
    return stRetList;
}
