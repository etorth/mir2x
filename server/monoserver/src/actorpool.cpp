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
#include <climits>
#include "uidf.hpp"
#include "fflerror.hpp"
#include "receiver.hpp"
#include "actorpod.hpp"
#include "raiitimer.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

extern MonoServer *g_monoServer;

// keep in mind:
// 1. at ANY time only one thread can access one actor message handler
// 2. at ANY time one thread must grab the schedLock before detach the mailbox

// actor thread id marker
// only get explicit assignment in actor threads
// for any other application threads it returns the defaut value
static thread_local int t_WorkerID = ActorPool::MAILBOX_ACCESS_PUB;
static int getWorkerID()
{
    return t_WorkerID;
}

ActorPool::Mailbox::Mailbox(ActorPod *pActor)
    : Actor(pActor)
    , schedLock()
    , nextQLock()
    , CurrQ()
    , NextQ()
    , AtExit()
    , Monitor(pActor->UID())
{}

ActorPool::ActorPool(uint32_t nBucketCount, uint32_t nLogicFPS)
    : m_logicFPS(nLogicFPS)
    , m_terminated(false)
    , m_bucketList(nBucketCount)
    , m_receiverLock()
    , m_receiverList()
{}

ActorPool::~ActorPool()
{
    if(isActorThread()){
        g_monoServer->addLog(LOGTYPE_WARNING, "destroy actor pool in actor thread %d", getWorkerID());
        g_monoServer->Restart();
    }

    // don't throw in dtor
    // can only put the log for diag

    m_terminated.store(true);
    try{
        for(auto p = m_futureList.begin(); p != m_futureList.end(); ++p){
            // in lanuch part if an actor thread can join
            // it already passed the unique_lock to clean all readers
            p->get();
        }
    }
    catch(...){
        g_monoServer->addLog(LOGTYPE_WARNING, "exceptions should be caught before exiting actor threads");
        g_monoServer->Restart();
    }
    m_futureList.clear();
}

bool ActorPool::Register(ActorPod *pActor)
{
    if(!(pActor && pActor->UID())){
        throw fflerror("invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, to_cvptr(pActor), pActor->UID());
    }

    auto nUID = pActor->UID();
    auto nIndex = uidf::getThreadID(nUID);
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
        std::unique_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);
        if(auto p = m_bucketList[nIndex].MailboxList.find(nUID); p == m_bucketList[nIndex].MailboxList.end()){
            m_bucketList[nIndex].MailboxList[nUID] = pMailbox;
            return true;
        }
        else{
            throw fflerror("UID exists: UID = %" PRIu64 ", ActorPod = %p", nUID, to_cvptr(p->second->Actor));
        }
    }
}

bool ActorPool::Register(Receiver *pReceiver)
{
    if(!pReceiver){
        throw fflerror("invlaid argument: Receiver = %p", to_cvptr(pReceiver));
    }

    {
        std::lock_guard<std::mutex> stLock(m_receiverLock);
        if(auto p = m_receiverList.find(pReceiver->UID()); p == m_receiverList.end()){
            m_receiverList[pReceiver->UID()] = pReceiver;
            return true;
        }
        else{
            throw fflerror("UID exists: UID = %" PRIu64 ", Receiver = %p", p->second->UID(), to_cvptr(p->second));
        }
    }
}

bool ActorPool::Detach(const ActorPod *pActor, const std::function<void()> &fnAtExit)
{
    if(!(pActor && pActor->UID())){
        throw fflerror("invalid arguments: ActorPod = %p, ActorPod::UID() = %" PRIu64, to_cvptr(pActor), pActor->UID());
    }

    // we can use UID as parameter
    // but use actor address prevents other thread detach blindly

    auto nIndex = uidf::getThreadID(pActor->UID());
    auto fnDoDetach = [this, &rstMailboxList = m_bucketList[nIndex].MailboxList, pActor, &fnAtExit]() -> bool
    {
        // we need to make sure after this funtion
        // there isn't any threads accessing the internal actor state
        if(auto p = rstMailboxList.find(pActor->UID()); p != rstMailboxList.end()){
            uint64_t nBackoff = 0;
            while(true){
                switch(MailboxLock stMailboxLock(p->second->schedLock, getWorkerID()); stMailboxLock.lockType()){
                    case MAILBOX_DETACHED:
                        {
                            // we allow double detach an actor
                            // this helps to always legally call Detach() in actor destructor

                            // if found a detached actor
                            // then the actor pointer must already be null
                            if(p->second->Actor){
                                throw fflerror("detached mailbox has non-zero actor pointer: ActorPod = %p, ActorPod::UID() = %" PRIu64, to_cvptr(p->second->Actor), pActor->UID());
                            }
                            return true;
                        }
                    case MAILBOX_READY:
                        {
                            // grabbed the schedLock successfully
                            // no other thread can access the message handler before unlock

                            // only check this consistancy when grabbed the lock
                            // otherwise other thread may change the actor pointer to null at any time
                            if(p->second->Actor != pActor){
                                throw fflerror("different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, to_cvptr(pActor), to_cvptr(p->second->Actor), pActor->UID());
                            }

                            // detach a locked mailbox
                            // remember any thread can't flip mailbox to detach before lock it!!!
                            if(auto nWorkerID = p->second->schedLock.Detach(); nWorkerID != getWorkerID()){
                                throw fflerror("locked actor flips to invalid status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", Status = %d", to_cvptr(pActor), pActor->UID(), nWorkerID);
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
                            // allow public thread to access the schedLock
                            // but it should never ever execute the actor handler

                            // I was planning to forbid any public thread to access the schedLock, but
                            //     1. Detach outside of actor threads
                            //     2. Query actor status
                            // needs to access the schedLock

                            backOff(nBackoff);
                            break;
                        }
                    default:
                        {
                            // can't grab the schedLock, means someone else is accessing it
                            // and we know it's not public threads accessing, then must be actor threads
                            if(!isActorThread(stMailboxLock.lockType())){
                                throw fflerror("invalid actor status: ActorPod = %p, ActorPod::UID() = %" PRIu64 ", status = %c", to_cvptr(pActor), pActor->UID(), stMailboxLock.lockType());
                            }

                            // inside actor threads
                            // if calling detach in the actor's message handler we can only mark the DETACHED status

                            if(stMailboxLock.lockType() == getWorkerID()){
                                // the lock is grabed already by current actor thread, check the consistancy
                                // only check it when current thread grabs the lock, otherwise other thread may change it to null at any time
                                if(p->second->Actor != pActor){
                                    throw fflerror("different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %" PRIu64, to_cvptr(pActor), to_cvptr(p->second->Actor), pActor->UID());
                                }

                                // this is from inside the actor's actor thread
                                // have to delay the atexit() since the message handler is not done yet
                                p->second->schedLock.Detach();
                                if(fnAtExit){
                                    p->second->AtExit = fnAtExit;
                                }

                                p->second->Actor = nullptr;
                                return true;
                            }

                            // target actor is in running status
                            // and current thread is not the the one grabs the lock, wait till it release the lock
                            backOff(nBackoff);
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

    if(getWorkerID() == (int)(nIndex)){
        return fnDoDetach();
    }
    else{
        std::shared_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);
        return fnDoDetach();
    }
}

bool ActorPool::Detach(const Receiver *pReceiver)
{
    if(!(pReceiver && pReceiver->UID())){
        throw fflerror("invalid arguments: Receiver = %p, Receiver::UID() = %" PRIu64, to_cvptr(pReceiver), pReceiver->UID());
    }

    // we allow detach a receiver multiple times
    // since we support this for actorpod, no better reason...
    {
        std::lock_guard<std::mutex> stLock(m_receiverLock);
        if(auto p = m_receiverList.find(pReceiver->UID()); (p != m_receiverList.end()) && (p->second == pReceiver)){
            m_receiverList.erase(p);
        }
        return true;
    }
}

bool ActorPool::PostMessage(uint64_t nUID, MessagePack stMPK)
{
    if(!nUID){
        throw fflerror("sending %s to zero UID", stMPK.Name());
    }

    if(!stMPK){
        throw fflerror("sending empty message to %" PRIu64, nUID);
    }

    if(uidf::isReceiver(nUID)){
        std::lock_guard<std::mutex> stLockGuard(m_receiverLock);
        if(auto p = m_receiverList.find(nUID); p != m_receiverList.end()){
            p->second->PushMessage(std::move(stMPK));
            return true;
        }
        return false;
    }

    auto nIndex = uidf::getThreadID(nUID);
    auto fnPostMessage = [this, nIndex, nUID](MessagePack stMPK)
    {
        // here won't try lock the mailbox
        // but it will check if the mailbox is detached

        if(auto p = m_bucketList[nIndex].MailboxList.find(nUID); p != m_bucketList[nIndex].MailboxList.end()){
            // just a cheat and can remove it
            // try return earlier with acquire the nextQLock
            if(p->second->schedLock.Detached()){
                return false;
            }

            // still here the mailbox can freely switch to detached status
            // need the actor thread do fully clear job
            {
                std::lock_guard<std::mutex> stLockGuard(p->second->nextQLock);
                if(p->second->schedLock.Detached()){
                    return false;
                }
                p->second->NextQ.push_back(std::move(stMPK));
                return true;
            }
        }
        return false;
    };

    if(getWorkerID() == (int)(nIndex)){
        return fnPostMessage(stMPK);
    }
    else{
        std::shared_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);
        return fnPostMessage(stMPK);
    }
}

bool ActorPool::runOneMailbox(Mailbox *pMailbox, bool bMetronome)
{
    if(!isActorThread()){
        throw fflerror("accessing actor message handlers outside of any actor threads: %d", getWorkerID());
    }

    // before every call to InnHandler()
    // check if it's detached in case actor call Detach() in its message handler
    //
    //     if(pActor->Detached()){       // 1. check
    //         return false;             // 2. return
    //     }                             // 3. actor can't flip to detached here
    //     pActor->InnHandler(stMPK);    // 4. handle

    // don't worry about actor detached at line-3
    // because it's in the message handling thread and it grabs the schedLock
    // any thread want to flip the actor to detached status must firstly grab its schedLock

    if(bMetronome){
        if(pMailbox->schedLock.Detached()){
            return false;
        }

        {
            raii_timer stTimer(&(pMailbox->Monitor.ProcTick));
            pMailbox->Actor->InnHandler({MPK_METRONOME, 0, 0});
        }
        pMailbox->Monitor.MessageDone.fetch_add(1);
    }

    if(pMailbox->CurrQ.empty()){
        std::lock_guard<std::mutex> stLockGuard(pMailbox->nextQLock);
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
        if(pMailbox->schedLock.Detached()){
            // need to erase all handled messages: [begin, p)
            // otherwise in ClearOneMailbox() will get handled again with MPK_BADACTORPOD
            pMailbox->CurrQ.erase(pMailbox->CurrQ.begin(), p);
            return false;
        }

        {
            raii_timer stTimer(&(pMailbox->Monitor.ProcTick));
            pMailbox->Actor->InnHandler(*p);
        }
        pMailbox->Monitor.MessageDone.fetch_add(1);
    }

    pMailbox->CurrQ.clear();
    return !pMailbox->schedLock.Detached();
}

void ActorPool::runWorker(size_t nIndex)
{
    hres_timer timer;
    runWorkerOneLoop(nIndex);
    m_bucketList[nIndex].runTimer.push(timer.diff_nsec());
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
    // 2     LockGuard(Mailbox.nextQLock);      | 2     {                                         // ClearOneMailbox() begins
    // 3     if(Mailbox.Detached()){            | 3         LockGuard(Mailbox.nextQLock);         //
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
        std::lock_guard<std::mutex> stLockGuard(pMailbox->nextQLock);
        if(!pMailbox->NextQ.empty()){
            pMailbox->CurrQ.insert(pMailbox->CurrQ.end(), pMailbox->NextQ.begin(), pMailbox->NextQ.end());
        }
    }

    for(auto pMPK = pMailbox->CurrQ.begin(); pMPK != pMailbox->CurrQ.end(); ++pMPK){
        if(pMPK->from()){
            AMBadActorPod stAMBAP;
            std::memset(&stAMBAP, 0, sizeof(stAMBAP));

            // when calling this function the Mailbox::Actor may already be empty
            // have to know this mailbox's UID, should I move Monitor::UID to Mailbox::UID ?

            stAMBAP.Type    = pMPK->Type();
            stAMBAP.from    = pMPK->from();
            stAMBAP.ID      = pMPK->ID();
            stAMBAP.Respond = pMPK->Respond();
            stAMBAP.UID     = pMailbox->Monitor.UID;

            PostMessage(pMPK->from(), {MessageBuf(MPK_BADACTORPOD, stAMBAP), 0, 0, pMPK->ID()});
        }
    }
    pMailbox->CurrQ.clear();
}

void ActorPool::runWorkerOneLoop(size_t nIndex)
{
    // check if in the correct thread
    // stealing actor thread can't call this function

    if(getWorkerID() != (int)(nIndex)){
        throw fflerror("accessing message handler outside of its dedicated actor thread: WorkerID = %d, BucketID = %d", getWorkerID(), (int)(nIndex));
    }

    auto fnUpdate = [this](size_t nIndex, auto p)
    {
        while(p != m_bucketList[nIndex].MailboxList.end()){
            switch(MailboxLock stMailboxLock(p->second->schedLock, getWorkerID()); stMailboxLock.lockType()){
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
                        if(!runOneMailbox(p->second.get(), true)){
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
                        // we didn't get the schedlock
                        // then don't access the actor pointer here for read/log...
                        if(!isActorThread(stMailboxLock.lockType())){
                            throw fflerror("invalid actor status: %d", stMailboxLock.lockType());
                        }

                        // current dedicated *actor* thread has already grabbed this mailbox lock
                        // means 1. recursively call this runWorkerOneLoop()
                        //       2. forget to release the lock

                        if(stMailboxLock.lockType() == getWorkerID()){
                            throw fflerror("mailbox sched_lock has already been grabbed by current thread: %d", stMailboxLock.lockType());
                        }

                        // if a mailbox grabbed by some other actor thread
                        // we skip it and try next one, the grbbing thread will handle all its queued message
                        ++p;
                        break;
                    }
            }
        }
        return m_bucketList[nIndex].MailboxList.end();
    };

    for(auto p = fnUpdate(nIndex, m_bucketList[nIndex].MailboxList.begin()); p != m_bucketList[nIndex].MailboxList.end(); p = fnUpdate(nIndex, p)){
        // we detected a detached actor
        // clear all its queued messages and remove it if ready
        if(p->second->AtExit){
            p->second->AtExit();
            p->second->AtExit = nullptr;
        }

        ClearOneMailbox(p->second.get());
        {
            std::unique_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);
            p = m_bucketList[nIndex].MailboxList.erase(p);
        }
    }
}

void ActorPool::Launch()
{
    for(int threadId = 0; threadId < (int)(m_bucketList.size()); ++threadId){
        m_futureList.emplace_back(std::async(std::launch::async, [threadId, this]()
        {
            // record the worker id
            // for application this will NOT get assigned
            t_WorkerID = threadId;
            try{
                auto currTick = g_monoServer->getCurrTick();
                while(!m_terminated.load()){
                    runWorker(threadId);

                    const auto exptTick = currTick + 1000 / m_logicFPS;
                    currTick = g_monoServer->getCurrTick();

                    if(currTick < exptTick){
                        g_monoServer->sleepExt(exptTick - currTick);
                    }
                }

                // terminted
                // need to clean all mailboxes
                {
                    std::unique_lock<std::shared_mutex> bucketLockGuard(m_bucketList[threadId].BucketLock);
                    m_bucketList[threadId].MailboxList.clear();
                }
            }
            catch(...){
                g_monoServer->propagateException();
            }

            // won't let the std::future get the exception
            // keep it clear since no polling on std::future::get() in main thread
            return true;
        }));
        g_monoServer->addLog(LOGTYPE_INFO, "Actor thread %d launched", threadId);
    }
}

bool ActorPool::CheckInvalid(uint64_t nUID) const
{
    auto nIndex = uidf::getThreadID(nUID);
    auto fnGetInvalid = [this, &rstMailboxList = m_bucketList[nIndex].MailboxList, nUID]() -> bool
    {
        if(auto p = rstMailboxList.find(nUID); p == rstMailboxList.end() || p->second->schedLock.Detached()){
            return true;
        }
        return false;
    };

    if(getWorkerID() == (int)(nIndex)){
        return fnGetInvalid();
    }else{
        std::shared_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);
        return fnGetInvalid();
    }
}

bool ActorPool::isActorThread() const
{
    return isActorThread(getWorkerID());
}

bool ActorPool::isActorThread(int nWorkerID) const
{
    return (nWorkerID >= 0) && (nWorkerID < (int)(m_bucketList.size()));
}

ActorPool::ActorMonitor ActorPool::GetActorMonitor(uint64_t nUID) const
{
    if(isActorThread()){
        throw fflerror("querying actor monitor inside actor thread: WorkerID = %d, UID = %" PRIu64, getWorkerID(), nUID);
    }

    auto nIndex = uidf::getThreadID(nUID);
    {
        // lock the bucket
        // other thread can detach the actor but can't erase the mailbox
        std::shared_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);

        // just read the mailbox cached variables
        // I don't need to acquire the sched_lock, I don't need it 100% accurate
        if(auto p = m_bucketList[nIndex].MailboxList.find(nUID); (p != m_bucketList[nIndex].MailboxList.end()) && (!p->second->schedLock.Detached())){
            return p->second->DumpMonitor();
        }
        return {};
    }
}

std::vector<ActorPool::ActorMonitor> ActorPool::GetActorMonitor() const
{
    if(isActorThread()){
        throw fflerror("querying actor monitor inside actor thread: WorkerID = %d", getWorkerID());
    }

    std::vector<ActorPool::ActorMonitor> stRetList;
    for(size_t nIndex = 0; nIndex < m_bucketList.size(); ++nIndex){
        std::shared_lock<std::shared_mutex> stLock(m_bucketList[nIndex].BucketLock);
        {
            stRetList.reserve(stRetList.size() + m_bucketList[nIndex].MailboxList.size());
            for(auto p = m_bucketList[nIndex].MailboxList.begin(); p != m_bucketList[nIndex].MailboxList.end(); ++p){
                if(!p->second->schedLock.Detached()){
                    stRetList.push_back(p->second->DumpMonitor());
                }
            }
        }
    }
    return stRetList;
}

int ActorPool::pickThreadID() const
{
    if(m_bucketList.empty()){
        throw fflerror("no bucket allocated");
    }

    int minTimeIndex = 0;
    long minTime = LONG_MAX;

    for(int index = 0; const auto &bucketRef: m_bucketList){
        if(const auto currTime = bucketRef.runTimer.getAvgTime(); minTime > currTime){
            minTimeIndex = index;
            minTime = currTime;
        }
        index++;
    }
    return minTimeIndex;
}
