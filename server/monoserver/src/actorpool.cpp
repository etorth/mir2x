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
#include "uidf.hpp"
#include "typecast.hpp"
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
static thread_local int t_workerID = ActorPool::MAILBOX_ACCESS_PUB;
static int getWorkerID()
{
    return t_workerID;
}

ActorPool::Mailbox::Mailbox(ActorPod *actorPtr)
    : uid(actorPtr->UID())
    , actor(actorPtr)
{}

ActorPool::ActorPool(int bucketCount, int logicFPS)
    : m_logicFPS([logicFPS]() -> uint32_t
      {
          if(logicFPS <= 0){
              return 5;
          }

          if(logicFPS >= 30){
              return 30;
          }
          return logicFPS;
      }())
    , m_bucketList([bucketCount]() -> uint32_t
      {
          if(bucketCount <= 0){
              return 1;
          }
          return bucketCount;
      }())
{
    g_monoServer->addLog(LOGTYPE_INFO, "Server FPS: %llu", to_llu(m_logicFPS));
}

ActorPool::~ActorPool()
{
    try{
        if(isActorThread()){
            throw fflerror("destroy actor pool in actor thread %d", getWorkerID());
        }

        for(auto &bucketRef: m_bucketList){
            bucketRef.uidQPending.close();
        }

        for(auto &bucketRef: m_bucketList){
            bucketRef.runThread.get();
        }
        m_bucketList.clear();
    }
    catch(...){
        g_monoServer->propagateException();
    }
}

void ActorPool::attach(ActorPod *actorPtr)
{
    if(!(actorPtr && actorPtr->UID())){
        throw fflerror("invalid arguments: ActorPod = %p, ActorPod::UID() = %llu", to_cvptr(actorPtr), to_llu(actorPtr->UID()));
    }

    // can call this function from:
    // 1. application thread
    // 2. other/current actor thread spawning new actors

    // remeber here the writer thread may starve, check:
    // https://stackoverflow.com/questions/2190090/how-to-prevent-writer-starvation-in-a-read-write-lock-in-pthreads

    // exclusively lock before write
    // 1. to make sure any other reading thread done
    // 2. current thread won't accquire the lock in read mode

    const auto uid = actorPtr->UID();
    auto mailboxPtr = std::make_unique<Mailbox>(actorPtr);
    auto &subBucketRef = getSubBucket(uid);
    {
        // always place the w-lock-protection
        // this is the only place that can grow a subbucket

        // this requires the sub-bucket lock is not acquired by anyone
        // when calling this function from public threads, then it's OK
        // when calling from the actor threads:
        //   1. in the actor message handler: should be OK, we always call message handler without sub-bucket lock accqured
        //   2. out of   the message handler: in actorpool.cpp

        MailboxSubBucket::WLockGuard lockGuard(subBucketRef.lock);
        if(!(subBucketRef.mailboxList.emplace(uid, std::move(mailboxPtr)).second)){
            throw fflerror("actor UID %llu exists in bucket already", to_llu(uid));
        }
    }

    // don't push the pointer into mailboxListCache, because mailboxListCache is used to iteration
    // if this attach is from the the iteration loop, it invalidates the mailboxListCache iterators in runOneMailboxBucket()
}

void ActorPool::attach(Receiver *receriverPtr)
{
    if(!(receriverPtr && receriverPtr->UID())){
        throw fflerror("invlaid argument: Receiver = %p", to_cvptr(receriverPtr));
    }

    {
        std::lock_guard<decltype(m_receiverLock)> lockGuard(m_receiverLock);
        if(m_receiverList.contains(receriverPtr->UID())){
            throw fflerror("receriver UID %llu alreayd exists in receriver list", to_llu(receriverPtr->UID()));
        }
        m_receiverList[receriverPtr->UID()] = receriverPtr;
    }
}

void ActorPool::detach(const ActorPod *actorPtr, const std::function<void()> &fnAtExit)
{
    if(!(actorPtr && actorPtr->UID())){
        throw fflerror("invalid arguments: ActorPod = %p, ActorPod::UID() = %llu", to_cvptr(actorPtr), to_llu(actorPtr->UID()));
    }

    // we can use UID as parameter
    // but use actor address prevents other thread detach blindly

    const auto bucketId = getBucketID(actorPtr->UID());
    auto &subBucketRef = getSubBucket(actorPtr->UID());
    const auto fnDetach = [this, &subBucketRef, actorPtr, &fnAtExit]()
    {
        // we need to make sure after this funtion
        // there isn't any threads accessing the internal actor state
        auto p = subBucketRef.mailboxList.find(actorPtr->UID());
        if(p == subBucketRef.mailboxList.end()){
            return;
        }

        uint64_t backOffCount = 0;
        auto mailboxPtr = p->second.get();

        while(true){
            switch(const MailboxLock grabLock(mailboxPtr->schedLock, getWorkerID()); const auto lockType = grabLock.lockType()){
                case MAILBOX_DETACHED:
                    {
                        // we allow double detach an actor
                        // this helps to always legally call detach() in actor destructor

                        // if found a detached actor
                        // the actor pointer must already be null
                        if(mailboxPtr->actor){
                            throw fflerror("detached mailbox has non-null actor pointer: ActorPod = %p, ActorPod::UID() = %llu", to_cvptr(mailboxPtr->actor), to_llu(actorPtr->UID()));
                        }
                        return;
                    }
                case MAILBOX_READY:
                    {
                        // grabbed the schedLock successfully
                        // no other thread can access the message handler before current thread releases it

                        // only check this consistancy when grabbed the lock
                        // otherwise other thread may change the actor pointer to null at any time
                        if(mailboxPtr->actor != actorPtr){
                            throw fflerror("different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %llu", to_cvptr(actorPtr), to_cvptr(mailboxPtr->actor), to_llu(actorPtr->UID()));
                        }

                        // detach a locked mailbox
                        // remember any thread can't detach a mailbox before grab it
                        if(const auto workerID = mailboxPtr->schedLock.detach(); workerID != getWorkerID()){
                            throw fflerror("locked actor flips to invalid status: ActorPod = %p, ActorPod::UID() = %llu, status = %d", to_cvptr(actorPtr), to_llu(actorPtr->UID()), workerID);
                        }

                        // since we can grab the schedLock
                        // this means there is no actor thread running current actor
                        //
                        // and there is no public thread already grabbed it
                        // because schedLock is exclusive, then we can call the atExit immediately here

                        if(fnAtExit){
                            fnAtExit();
                        }

                        // detached
                        // remove the actor pointer then no-one can access through it
                        mailboxPtr->actor = nullptr;
                        return;
                    }
                case MAILBOX_ACCESS_PUB:
                    {
                        // allow public thread to access the schedLock
                        // but it should never ever execute the actor handler

                        // I was planning to forbid any public thread to access the schedLock, but
                        //     1. detach outside of actor threads
                        //     2. query actor status
                        // needs to access the schedLock
                        // especially for case-1, we need this to forcely remove an actor from the actor threads
                        //
                        //     int main()
                        //     {
                        //         ActorPod actor;  // automatically added to the pool
                        //                          // it get scheduled by the actor threads
                        //         // ...
                        //         // ...
                        //         actor.detach();  // detach from main-thread, not actor thread
                        //     }
                        backOff(backOffCount);
                        break;
                    }
                default:
                    {
                        // can't grab the schedLock, means someone else is accessing it
                        // and we know it's not public threads accessing, then must be actor threads
                        if(!isActorThread(lockType)){
                            throw fflerror("invalid actor status: ActorPod = %p, ActorPod::UID() = %llu, status = %d", to_cvptr(actorPtr), to_llu(actorPtr->UID()), lockType);
                        }

                        // inside actor threads
                        // if calling detach in the actor's message handler we can only mark the DETACHED status

                        if(lockType == getWorkerID()){
                            // the lock is grabed already by current actor thread, check the consistancy
                            // only check it when current thread grabs the lock, otherwise other thread may change it to null at any time
                            if(mailboxPtr->actor != actorPtr){
                                throw fflerror("different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %llu", to_cvptr(actorPtr), to_cvptr(mailboxPtr->actor), to_llu(actorPtr->UID()));
                            }

                            // this is from inside the actor's actor thread
                            // have to delay the atexit() since the message handler is not done yet
                            mailboxPtr->schedLock.detach();
                            if(fnAtExit){
                                mailboxPtr->atExit = fnAtExit;
                            }

                            mailboxPtr->actor = nullptr;
                            return;
                        }

                        // target actor is in running status
                        // current thread is not the the one grabs the lock, wait till it release the lock
                        backOff(backOffCount);
                        break;
                    }
            }
        }
        throw bad_reach();
    };

    // current thread won't lock current actor bucket in read mode
    // any other thread can only accquire in read mode

    if(getWorkerID() == bucketId){
        fnDetach();
    }
    else{
        MailboxSubBucket::RLockGuard lockGuard(subBucketRef.lock);
        fnDetach();
    }
}

void ActorPool::detach(const Receiver *receiverPtr)
{
    if(!(receiverPtr && receiverPtr->UID())){
        throw fflerror("invalid arguments: Receiver = %p, Receiver::UID() = %llu", to_cvptr(receiverPtr), to_llu(receiverPtr->UID()));
    }

    // we allow detach a receiver multiple times
    // since we support this for actor
    {
        std::lock_guard<std::mutex> stLock(m_receiverLock);
        auto p = m_receiverList.find(receiverPtr->UID());

        if(p == m_receiverList.end()){
            return;
        }

        if(p->second == receiverPtr){
            m_receiverList.erase(p);
            return;
        }
        throw fflerror("different receivers with same UID: Receiver = (%p, %p), Receiver::UID() = %llu", to_cvptr(receiverPtr), to_cvptr(p->second), to_llu(receiverPtr->UID()));
    }
}

bool ActorPool::postMessage(uint64_t uid, MessagePack msg)
{
    if(!uid){
        throw fflerror("sending %s to zero UID", msg.Name());
    }

    if(!msg){
        throw fflerror("sending empty message to %llu", to_llu(uid));
    }

    if(uidf::isReceiver(uid)){
        std::lock_guard<std::mutex> lockGuard(m_receiverLock);
        if(auto p = m_receiverList.find(uid); p != m_receiverList.end()){
            p->second->PushMessage(std::move(msg));
            return true;
        }
        return false;
    }

    const auto bucketId = getBucketID(uid);
    auto &subBucketRef = getSubBucket(uid);
    const auto fnPostMessage = [this, bucketId, &subBucketRef, uid](MessagePack msg)
    {
        // here won't try lock the mailbox
        // but it will check if the mailbox is detached

        if(auto p = subBucketRef.mailboxList.find(uid); p != subBucketRef.mailboxList.end()){
            // just a cheat and can remove it
            // try return earlier without acquiring the nextQLock
            if(p->second->schedLock.detached()){
                return false;
            }

            // still here the mailbox can freely switch to detached status
            // need the actor thread do fully clear job
            {
                std::lock_guard<std::mutex> lockGuard(p->second->nextQLock);
                if(p->second->schedLock.detached()){
                    return false;
                }

                // during the push other thread can still flip the actor to detached status
                // we can't guarantee, the only thing we can do is:
                //   1. don't run an already detached actor
                //   2. clear all pending message in a detached actor by clearOneMailbox()
                p->second->nextQ.push_back(std::move(msg));
            }

            // here I can push to any buckets
            // every dedicated actor thread can handle UID outside of its bucket

            const auto bucketCount = (int)(m_bucketList.size());
            for(int i = 0; i < bucketCount; ++i){
                if(m_bucketList.at((bucketId + i) % bucketCount).uidQPending.try_push(uid)){
                    return true;
                }
            }

            m_bucketList.at(bucketId).uidQPending.push(uid);
            return true;
        }
        return false;
    };

    if(getWorkerID() == bucketId){
        return fnPostMessage(std::move(msg));
    }
    else{
        MailboxSubBucket::RLockGuard lockGuard(subBucketRef.lock);
        return fnPostMessage(std::move(msg));
    }
}

void ActorPool::runOneUID(uint64_t uid)
{
    const auto workerID = getWorkerID();
    if(!isActorThread(workerID)){
        throw fflerror("running actor UID %llu by public thread", to_llu(uid));
    }

    const auto fnRunMailbox = [workerID, this](auto mailboxPtr) -> bool
    {
        switch(MailboxLock grabLock(mailboxPtr->schedLock, workerID); const auto lockType = grabLock.lockType()){
            case MAILBOX_DETACHED:
                {
                    return false;
                }
            case MAILBOX_READY:
                {
                    return runOneMailbox(mailboxPtr, false);
                }
            case MAILBOX_ACCESS_PUB:
                {
                    // accessed by public thread
                    // should I spin here?
                    return true;
                }
            default:
                {
                    return true;
                }
        }
    };

    const int bucketId = getBucketID(uid);
    auto &subBucketRef = getSubBucket(uid);
    auto mailboxPtr = [&subBucketRef, uid]() -> Mailbox *
    {
        // IMPORTANT: even in dedicated actor thread we still need to use r-lock proctection
        //            because other actor may spawn new actors which get added to current sub-bucket
        MailboxSubBucket::RLockGuard lockGuard(subBucketRef.lock);
        if(auto p = subBucketRef.mailboxList.find(uid); p != subBucketRef.mailboxList.end()){
            return p->second.get();
        }
        return nullptr;
    }();

    if(!mailboxPtr){
        return;
    }

    if(fnRunMailbox(mailboxPtr)){
        return;
    }

    // run the mailbox and found it has been detached
    // remove it here if possible

    if(workerID == bucketId){
        MailboxSubBucket::WLockGuard lockGuard(subBucketRef.lock);
        subBucketRef.mailboxList.erase(uid);
    }
}

bool ActorPool::runOneMailbox(Mailbox *mailboxPtr, bool useMetronome)
{
    if(!isActorThread()){
        throw fflerror("accessing actor message handlers outside of actor threads: %d", getWorkerID());
    }

    // before every call to innHandler()
    // check if it's detached in case actor call detach() in its message handler
    //
    //     if(actorPtr->detached()){       // 1. check
    //         return false;               // 2. return
    //     }                               // 3. actor can't flip to detached here
    //     actorPtr->innHandler(stMPK);    // 4. handle

    // don't worry about actor detached at line-3
    // because it's in the message handling thread and it grabs the schedLock
    // any thread want to flip the actor to detached status must firstly grab its schedLock

    if(useMetronome){
        if(mailboxPtr->schedLock.detached()){
            return false;
        }

        {
            raii_timer procTimer(&(mailboxPtr->monitor.procTick));
            mailboxPtr->actor->innHandler({MPK_METRONOME, 0, 0});
        }

        mailboxPtr->monitor.messageDone.fetch_add(1);
        if(mailboxPtr->schedLock.detached()){
            return false;
        }
    }

    // process till first time can see empty queue
    // outside of this function always give up if found the mailbox is running

    while(true){
        if(mailboxPtr->currQ.empty()){
            std::lock_guard<std::mutex> lockGuard(mailboxPtr->nextQLock);
            if(mailboxPtr->nextQ.empty()){
                return true;
            }
            std::swap(mailboxPtr->currQ, mailboxPtr->nextQ);
        }

        // this is a good place to store the pending message size
        // during the *normal* runtime, no one will push mesages to currQ, all to the nextQ
        mailboxPtr->monitor.messagePending.store(mailboxPtr->currQ.size());

        // if we return in the middle
        // we may leave unhandled message in currQ

        for(auto p = mailboxPtr->currQ.begin(); p != mailboxPtr->currQ.end(); ++p){
            if(mailboxPtr->schedLock.detached()){
                // need to erase messages already done: [begin, p)
                // otherwise in clearOneMailbox() it will get handled again with MPK_BADACTORPOD
                mailboxPtr->currQ.erase(mailboxPtr->currQ.begin(), p);
                return false;
            }

            {
                raii_timer procTimer(&(mailboxPtr->monitor.procTick));
                mailboxPtr->actor->innHandler(*p);
            }
            mailboxPtr->monitor.messageDone.fetch_add(1);
        }

        mailboxPtr->currQ.clear();

        // finished currQ without detach the actor
        // check when we are running currQ if any other thread post more messages to nextQ
    }
    throw bad_reach();
}

void ActorPool::clearOneMailbox(Mailbox *mailboxPtr)
{
    // actor can detach and immedately deconstruct it self
    // when found a mailbox is detached never access it's actor pointer

    // clean one mailbox means:
    // 1. make sure all actor/application threads DONE posting to it
    // 2. make sure all actor/application threads can not post new messages to it
    // 3. make sure all queued message are responded with MPK_BADACTORPOD

    // I call this function that:
    // 1. without accquiring writer lock
    // 2. always with Mailbox.detached() as TRUE

    // 0 // thread-1: posting message to nextQ  | 0 // thread-2: try clear all pending/sending messages
    // 1 {                                      | 1 if(Mailbox.detached()){
    // 2     LockGuard(Mailbox.nextQLock);      | 2     {                                         // clearOneMailbox() begins
    // 3     if(Mailbox.detached()){            | 3         LockGuard(Mailbox.nextQLock);         //
    // 4         return false;                  | 4         Mailbox.currQ.Enqueue(Mailbox.nextQ); //
    // 5     }                                  | 5     }                                         //
    // 6     Mailbox.nextQ.Enqueue(MSG);        | 6                                               //
    // 7     return true                        | 7     Handle(Mailbox.currQ);                    // clearOneMailbox() ends
    // 8 }                                      | 8 }

    // currQ is only used in main/stealing actor threads
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

    // so we conclude if in thread-2 clearOneMailbox() returns
    // we are sure there is no message is posting or to post message to nextQ

    {
        std::lock_guard<std::mutex> stLockGuard(mailboxPtr->nextQLock);
        if(!mailboxPtr->nextQ.empty()){
            mailboxPtr->currQ.insert(mailboxPtr->currQ.end(), mailboxPtr->nextQ.begin(), mailboxPtr->nextQ.end());
        }
    }

    for(auto pMPK = mailboxPtr->currQ.begin(); pMPK != mailboxPtr->currQ.end(); ++pMPK){
        if(pMPK->from()){
            AMBadActorPod stAMBAP;
            std::memset(&stAMBAP, 0, sizeof(stAMBAP));

            // when calling this function the Mailbox::actor may already be empty
            // have to know this mailbox's UID, should I move monitor::UID to Mailbox::UID ?

            stAMBAP.Type    = pMPK->Type();
            stAMBAP.from    = pMPK->from();
            stAMBAP.ID      = pMPK->ID();
            stAMBAP.Respond = pMPK->Respond();
            stAMBAP.UID     = mailboxPtr->uid;

            postMessage(pMPK->from(), {MessageBuf(MPK_BADACTORPOD, stAMBAP), 0, 0, pMPK->ID()});
        }
    }
    mailboxPtr->currQ.clear();
}

void ActorPool::runOneMailboxBucket(int bucketId)
{
    const int workerId = getWorkerID();
    if(workerId != bucketId){
        throw fflerror("udpate mailbox bucket %d by thread %d", bucketId, workerId);
    }

    const auto fnRunMailbox = [workerId, this](Mailbox *p) -> bool
    {
        // return false if detached
        // we can't add r/w-lock on the current sub-bucket
        // because when run each mailbox, its message handler may spawn new actors which changes current sub-bucket

        switch(MailboxLock grabLock(p->schedLock, workerId); const auto lockType = grabLock.lockType()){
            case MAILBOX_DETACHED:
                {
                    return false;
                }
            case MAILBOX_READY:
                {
                    // we grabbed the mailbox successfully
                    // but it can flip to detached if call detach() in its message handler

                    // if between message handling it flips to detached
                    // we just return immediately and leave the rest handled message there

                    // don't try clean it
                    // since we can't guarentee to clean it complately
                    return runOneMailbox(p, true);
                }
            case MAILBOX_ACCESS_PUB:
                {
                    // we need to allow public threads accessing the schedLock to detach actors
                    //    1. public thread can detach it by force
                    //    2. public thread can query actorpod statistics
                    // but should I spin here or just leave and try next mailbox ???
                    return true;
                }
            default:
                {
                    // we didn't get the schedlock
                    // then don't access the actor pointer here for read/log...
                    if(!isActorThread(lockType)){
                        throw fflerror("invalid mailbox status: %d", lockType);
                    }

                    // current dedicated *actor* thread has already grabbed this mailbox lock
                    // means 1. recursively call this runWorkerOneLoop()
                    //       2. forget to release the lock

                    if(lockType == workerId){
                        throw fflerror("mailbox sched_lock has already been grabbed by current thread: %d", lockType);
                    }

                    // if a mailbox grabbed by some other actor thread
                    // we skip it and try next one, the grbbing thread will handle all its queued message
                    return true;
                }
        }
    };

    auto &bucketRef = m_bucketList.at(bucketId);
    for(int subBucketId = 0; subBucketId < m_subBucketCount; ++subBucketId){
        auto &subBucketRef = bucketRef.subBucketList.at(subBucketId);
        auto &listCacheRef = subBucketRef.mailboxListCache;
        {
            MailboxSubBucket::RLockGuard lockGuard(subBucketRef.lock);
            if(listCacheRef.size() != subBucketRef.mailboxList.size()){
                listCacheRef.clear();
                listCacheRef.reserve(subBucketRef.mailboxList.size());
                for(auto &p: subBucketRef.mailboxList){
                    listCacheRef.push_back(p.second.get());
                }
            }
        }

        // implemented to use index, not iterator
        // this can help to support if we append new mailbox pointers into the cache

        bool hasDeletedMailbox = false;
        for(size_t mailboxIndex = 0; mailboxIndex < listCacheRef.size(); ++mailboxIndex){
            auto mailboxPtr = listCacheRef.at(mailboxIndex);
            if(fnRunMailbox(mailboxPtr)){
                continue;
            }

            if(mailboxPtr->atExit){
                mailboxPtr->atExit();
                mailboxPtr->atExit = nullptr;
            }

            clearOneMailbox(mailboxPtr);
            {
                MailboxSubBucket::WLockGuard lockGuard(subBucketRef.lock);
                subBucketRef.mailboxList.erase(mailboxPtr->uid);
                hasDeletedMailbox = true;
            }
        }

        // clear the cache if we removed any mailbox
        // otherwise accesses dangling pointers when we deleted one mailbox and and one mailbox
        // the size check can't detect it

        if(hasDeletedMailbox){
            listCacheRef.clear();
        }
    }
}

void ActorPool::launchPool()
{
    // one bucket has one dedicated thread
    // it will feed the update message METRNOME and can steal jobs when not busy

    for(int bucketId = 0; bucketId < (int)(m_bucketList.size()); ++bucketId){
        m_bucketList.at(bucketId).runThread = std::async(std::launch::async, [bucketId, this]()
        {
            // setup the worker id
            // for any other thread this will NOT get assigned
            t_workerID = bucketId;
            try{
                raii_timer timer;
                uint64_t lastUpdateTime = 0;
                const uint64_t maxUpdateWaitTime = 1000ULL / m_logicFPS;

                bool hasUID = false;
                uint64_t uidPending = 0;

                while(true){
                    if(hasUID){
                        runOneUID(uidPending);
                        hasUID = false;
                        uidPending = 0;
                    }

                    else{
                        const uint64_t currTime = timer.diff_msec();
                        if(currTime >= lastUpdateTime + maxUpdateWaitTime){
                            runOneMailboxBucket(bucketId);
                            lastUpdateTime = currTime;
                        }

                        for(int i = 0; i < (int)(m_bucketList.size()) * 4; ++i){
                            if(m_bucketList[(bucketId + i) % m_bucketList.size()].uidQPending.try_pop(uidPending)){
                                hasUID = true;
                                break;
                            }
                        }

                        if(!hasUID){
                            int ec = 0;
                            const uint64_t exptUpdateTime = lastUpdateTime + maxUpdateWaitTime;
                            if(currTime < exptUpdateTime){
                                m_bucketList[bucketId].uidQPending.pop(uidPending, exptUpdateTime - currTime, ec);
                            }
                            else{
                                ec = asyncf::E_TIMEOUT;
                            }

                            if(ec == asyncf::E_QCLOSED){
                                break;
                            }
                            else if(ec == asyncf::E_TIMEOUT){
                                // didn't get any pending UID
                                // and when reach here we are sure the thread needs to update the whole mailbox by METRONOME

                                // do nothing here
                                // hold for next loop
                            }
                            else if(ec == asyncf::E_DONE){
                                hasUID = true;
                            }
                            else{
                                throw fflerror("asyncf::taskQ::pop() returns invalid result: %d", ec);
                            }
                        }
                    }
                }
            }
            catch(...){
                g_monoServer->propagateException();
            }
        });
        g_monoServer->addLog(LOGTYPE_INFO, "Actor thread %d launched", bucketId);
    }
}

bool ActorPool::checkInvalid(uint64_t uid) const
{
    const auto bucketId = getBucketID(uid);
    const auto &subBucketCRef = getSubBucket(uid);
    const auto fnCheckInvalid = [this, &subBucketCRef, uid]() -> bool
    {
        const auto p = subBucketCRef.mailboxList.find(uid);
        return p == subBucketCRef.mailboxList.end() || p->second->schedLock.detached();
    };

    if(getWorkerID() == bucketId){
        return fnCheckInvalid();
    }
    else{
        MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);
        return fnCheckInvalid();
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

ActorPool::ActorMonitor ActorPool::getActorMonitor(uint64_t uid) const
{
    if(isActorThread()){
        throw fflerror("querying actor monitor inside actor thread: WorkerID = %d, UID = %llu", getWorkerID(), to_llu(uid));
    }

    const auto &subBucketCRef = getSubBucket(uid);
    {
        MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);
        if(const auto p = subBucketCRef.mailboxList.find(uid); p != subBucketCRef.mailboxList.end() && p->second->schedLock.detached()){
            return p->second->dumpMonitor();
        }
    }
    return {};
}

std::vector<ActorPool::ActorMonitor> ActorPool::getActorMonitor() const
{
    if(isActorThread()){
        throw fflerror("querying actor monitor inside actor thread: WorkerID = %d", getWorkerID());
    }

    size_t actorCount = 0;
    for(const auto &bucketCRef: m_bucketList){
        for(const auto &subBucketCRef: bucketCRef.subBucketList){
            MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);
            actorCount += subBucketCRef.mailboxList.size();
        }
    }

    std::vector<ActorPool::ActorMonitor> result;
    result.reserve(to_llu(actorCount * 1.5));

    for(const auto &bucketCRef: m_bucketList){
        for(const auto &subBucketCRef: bucketCRef.subBucketList){
            MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);
            for(const auto &p: subBucketCRef.mailboxList){
                if(p.second->schedLock.detached()){
                    continue;
                }
                result.push_back(p.second->dumpMonitor());
            }
        }
    }
    return result;
}
