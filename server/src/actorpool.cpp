#include <mutex>
#include <thread>
#include <cstdint>
#include "log.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "receiver.hpp"
#include "actorpod.hpp"
#include "raiitimer.hpp"
#include "actorpool.hpp"
#include "monoserver.hpp"

extern MonoServer *g_monoServer;

// KEEP IN MIND:
// at ANY time only one thread can access one actor message handler
// at ANY time one thread must grab the schedLock before to call the actor message handler
// at ANY time one thread must grab the schedLock before to detach the mailbox with its actor
// at ANY time when we are in the actor message handler function, the sub-bucket it belongs to shouldn't be r/w-locked, but schedLock must be grabbed

// any threads can add new {uid, mailboxPtr} pair to the sub-bucket
// but only the dedicated actor thread can remove {uid, mailboxPtr} pair from the sub-bucket
// don't try to do {uid, mailboxPtr} removal after we detached an actor from its mailbox, leave it to runOneMailboxBucket()
// because runOneMailboxBucket() calls to clearOneMailbox() which clean job for pending messages in the queue
//
// and when a mailboxPtr is detached from its actor in one thread, other threads can still holding this mailboxPtr and not aware of the schedLock flip
// this is the dangerous part, and this is the reason why I strictly restrict the {uid, mailboxPtr} removal should be in runOneMailboxBucket()

// anytime when we get a raw mailboPtr from the sub-bucket
// we need to add r-lock because other thread can spawn new actors to the sub-bucket concurrently

// actor thread id marker
// only get explicit assignment in actor threads
// for any other application threads it returns the defaut value
static thread_local int t_workerID = ActorPool::MAILBOX_ACCESS_PUB;
static int getWorkerID()
{
    return t_workerID;
}

ActorPool::Mailbox::Mailbox(ActorPod *actorPtr, std::function<void()> atStartTrigger)
    : uid(actorPtr->UID())
    , actor(actorPtr)
    , atStart(std::move(atStartTrigger))
{}

ActorPool::ActorPool(int bucketCount, int logicFPS)
    : m_logicFPS([logicFPS]() -> uint32_t
      {
          fflassert(logicFPS > 0);
          return logicFPS;
      }())
    , m_bucketList([bucketCount]() -> uint32_t
      {
          fflassert(bucketCount > 0);
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

void ActorPool::attach(ActorPod *actorPtr, std::function<void()> atStart)
{
    logProfiler();
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
    auto mailboxPtr = std::make_unique<Mailbox>(actorPtr, std::move(atStart));
    auto mailboxRawPtr = mailboxPtr.get();
    auto &subBucketRef = getSubBucket(uid);
    {
        // always place the w-lock-protection
        // this is the only place that can grow a subbucket

        // this requires the sub-bucket lock is not acquired by anyone
        // since lock a mutex multiple time is undefined
        //
        // when calling this function from public threads, then it's fine
        // when calling from the actor threads:
        //   1. in the actor message handler: should be OK, we always call message handler without sub-bucket lock accqured
        //   2. out of   the message handler: in actorpool.cpp, we shouldn't call attach() out of actor message hander in actor threads

        MailboxSubBucket::WLockGuard lockGuard(subBucketRef.lock);
        if(!(subBucketRef.mailboxList.emplace(uid, std::move(mailboxPtr)).second)){
            throw fflerror("actor UID %llu exists in bucket already", to_llu(uid));
        }
    }

    // attached actor has startup trigger
    // schedule it ASAP, currently only try its decicated bucket
    const int bucketId = getBucketID(uid);
    m_bucketList.at(bucketId).uidQPending.push(uid);

    // the mailboxListCache is accessed by index rather than using iterator
    // this helps to insert the new mailbox pointer here

    // the mailboxListCache is never protected by lock
    // but we restrict that the dedicated actor thread can access it

    // if it's not the dedicated actor thread calling the attach()
    // we can't do this because its dedicated actor thread may immediately remove {uid, mailboxPtr} before here we push it

    if(getWorkerID() == bucketId){
        subBucketRef.mailboxListCache.push_back(mailboxRawPtr);
    }
}

void ActorPool::attach(Receiver *receriverPtr)
{
    if(!(receriverPtr && receriverPtr->UID())){
        throw fflerror("invlaid argument: Receiver = %p", to_cvptr(receriverPtr));
    }

    std::lock_guard<decltype(m_receiverLock)> lockGuard(m_receiverLock);
    if(!m_receiverList.emplace(receriverPtr->UID(), receriverPtr).second){
        throw fflerror("receriver UID %llu alreayd exists in receriver list", to_llu(receriverPtr->UID()));
    }
}

void ActorPool::detach(const ActorPod *actorPtr, std::function<void()> fnAtExit)
{
    logProfiler();
    if(!(actorPtr && actorPtr->UID())){
        throw fflerror("invalid arguments: ActorPod = %p, ActorPod::UID() = %llu", to_cvptr(actorPtr), to_llu(actorPtr->UID()));
    }

    // we can use uid as parameter
    // but use actor address prevents other thread detach blindly

    // we only flip the schedLock by this function
    // this function guarentees that it won't remove the {uid, mailboxPtr} from the sub-bucket

    const auto uid = actorPtr->UID();
    const auto workerId = getWorkerID();
    const auto bucketId = getBucketID(uid);

    // if not in dedicated actor thread we need to r-lock the sub-bucket
    // otherwise the dedicated actor thread may detach and remove {uid, mailboxPtr} from the sub-bucket

    Mailbox *mailboxPtr = nullptr;
    MailboxSubBucket::RLockGuard lockGuard;
    if(workerId == bucketId){
        mailboxPtr = tryGetMailboxPtr(uid);
    }
    else{
        auto rlockedMailboxPtr = tryGetRLockedMailboxPtr(uid);
        lockGuard = std::move(rlockedMailboxPtr.first);
        mailboxPtr = rlockedMailboxPtr.second;
    }

    if(!mailboxPtr){
        return;
    }

    // we need to make sure after this funtion
    // there isn't any threads accessing the internal actor state

    uint64_t backOffCount = 0;
    while(true){
        switch(const MailboxLock grabLock(mailboxPtr->schedLock, workerId); const auto lockType = grabLock.lockType()){
            case MAILBOX_DETACHED:
                {
                    // we allow double detach an actor
                    // this helps to always legally call detach() in actor destructor

                    // if found a detached actor
                    // the actor pointer must already be null
                    if(mailboxPtr->actor){
                        throw fflerror("detached mailbox has non-null actor pointer: ActorPod = %p, ActorPod::UID() = %llu", to_cvptr(mailboxPtr->actor), to_llu(uid));
                    }
                    return;
                }
            case MAILBOX_READY:
                {
                    // grabbed the schedLock successfully
                    // no other thread can access the message handler before current thread releases it

                    // now we can safely release the r-lock
                    // because no threads can destroy the mailboxPtr without grab the schedLock
                    if(lockGuard){
                        lockGuard.unlock();
                    }

                    // only check this consistancy when grabbed the lock
                    // otherwise other thread may change the actor pointer to null at any time
                    if(mailboxPtr->actor != actorPtr){
                        throw fflerror("different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %llu", to_cvptr(actorPtr), to_cvptr(mailboxPtr->actor), to_llu(uid));
                    }

                    // do everything before we detach the mailbox
                    // immediately after detaching, the mailboxPtr can't be guaranteed to keep valid, if not in dedicated actor thread

                    // since we can grab the schedLock
                    // this means there is no actor thread running current actor
                    //
                    // and there is no public thread already grabbed it
                    // because schedLock is exclusive, then we can call the atExit immediately here

                    // can we access sub-bucket through fnAtExit?
                    // from here looks OK

                    if(fnAtExit){
                        fnAtExit();
                    }

                    // detached
                    // reset the actorPtr then no-one can access through the mailboxPtr
                    mailboxPtr->actor = nullptr;

                    // detach a locked mailbox
                    // remember any thread can't detach a mailbox before grab it
                    // if the returned value is not current thread id, means there is severe logic error
                    if(const auto detachWorkerId = mailboxPtr->schedLock.detach(); detachWorkerId != workerId){
                        throw fflerror("locked actor flips to invalid status: ActorPod = %p, ActorPod::UID() = %llu, status = %d", to_cvptr(actorPtr), to_llu(uid), workerId);
                    }
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
                        throw fflerror("invalid actor status: ActorPod = %p, ActorPod::UID() = %llu, status = %d", to_cvptr(actorPtr), to_llu(uid), lockType);
                    }

                    // inside actor threads
                    // if calling detach() in the actor's message handler we can only mark the DETACHED status

                    if(lockType == workerId){
                        // current actor thread has already grabbed the schedLock
                        // note that current actor thread may not be the dedicated actor thread
                        // here we can release the r-lock even if when workderId != bucketId, because the schedLock is already grabbed
                        if(lockGuard){
                            lockGuard.unlock();
                        }

                        // the lock is grabed already by current actor thread, check the consistancy
                        // only check it when current thread grabs the lock, otherwise other thread may change it to null at any time
                        if(mailboxPtr->actor != actorPtr){
                            throw fflerror("different actors with same UID: ActorPod = (%p, %p), ActorPod::UID() = %llu", to_cvptr(actorPtr), to_cvptr(mailboxPtr->actor), to_llu(uid));
                        }

                        // this is from inside the actor's actor thread
                        // have to delay calling the atExit() since the message handler is not done yet
                        // the fnAtExit can be [](){ delete this; }

                        if(fnAtExit){
                            mailboxPtr->atExit = std::move(fnAtExit);
                        }

                        mailboxPtr->actor = nullptr;
                        mailboxPtr->schedLock.detach();
                        return;
                    }

                    // target actor is in running status
                    // current thread is not the the one grabs the lock, wait till it release the lock
                    backOff(backOffCount);
                    break;
                }
        }
    }
    throw fflreach();
}

void ActorPool::detach(const Receiver *receiverPtr)
{
    if(!(receiverPtr && receiverPtr->UID())){
        throw fflerror("invalid arguments: Receiver = %p, Receiver::UID() = %llu", to_cvptr(receiverPtr), to_llu(receiverPtr->UID()));
    }

    // we allow detach a receiver multiple times
    // since it's supported for actor

    std::lock_guard<std::mutex> lockGuard(m_receiverLock);
    if(auto p = m_receiverList.find(receiverPtr->UID()); p != m_receiverList.end()){
        if(p->second != receiverPtr){
            throw fflerror("different receivers with same UID: Receiver = (%p, %p), Receiver::UID() = %llu", to_cvptr(receiverPtr), to_cvptr(p->second), to_llu(receiverPtr->UID()));
        }
        m_receiverList.erase(p);
    }
}

bool ActorPool::postMessage(uint64_t uid, ActorMsgPack msg)
{
    logProfiler();
    if(!uid){
        throw fflerror("sending %s to zero UID", mpkName(msg.type()));
    }

    if(!msg){
        throw fflerror("sending empty message to %llu", to_llu(uid));
    }

    if(uidf::isReceiver(uid)){
        std::lock_guard<std::mutex> lockGuard(m_receiverLock);
        if(auto p = m_receiverList.find(uid); p != m_receiverList.end()){
            p->second->pushMessage(std::move(msg));
            return true;
        }
        return false;
    }

    const auto bucketId = getBucketID(uid);
    const auto fnPostMessage = [](Mailbox *mailboxPtr, ActorMsgPack msg) -> bool
    {
        logScopedProfiler("pushMailbox");

        // just a cheat and can remove it
        // try return earlier without acquiring the nextQLock
        if(mailboxPtr->schedLock.detached()){
            return false;
        }

        // still here the mailbox can freely switch to detached status
        // need the actor thread do fully clear job

        // profiler helper
        // measure the delay that when the message reaches actorpool and it gets executed
        const uint64_t nowTime = mailboxPtr->monitor.liveTimer.diff_nsec();
        {
            std::lock_guard<std::mutex> lockGuard(mailboxPtr->nextQLock);
            if(mailboxPtr->schedLock.detached()){
                return false;
            }

            // during the push other thread can still flip the actor to detached status
            // we can't guarantee, the only thing we can do is:
            //   1. don't run an already detached actor
            //   2. clear all pending message in a detached actor by clearOneMailbox()
            mailboxPtr->nextQ.push_back(std::pair<ActorMsgPack, uint64_t>(std::move(msg), nowTime));
        }
        return true;
    };

    // if not in dedicated actor thread we have to grab the r-lock when posting
    // otherwise the {uid, mailboxPtr} can be removed from the sub-bucket by dedicated actor-thread when we are posting it
    //
    // another way is grab the schedLoc when posting
    // but this is exclusive, r-lock is shared and then preferred
    {
        Mailbox *mailboxPtr = nullptr;
        MailboxSubBucket::RLockGuard lockGuard;

        if(getWorkerID() == bucketId){
            mailboxPtr = tryGetMailboxPtr(uid);
        }
        else{
            auto rlockedMailboxPtr = tryGetRLockedMailboxPtr(uid);
            lockGuard = std::move(rlockedMailboxPtr.first);
            mailboxPtr = rlockedMailboxPtr.second;
        }

        if(!mailboxPtr){
            return false;
        }

        if(!fnPostMessage(mailboxPtr, std::move(msg))){
            return false;
        }
    }

    // done posting
    // we don't need any r-lock to send notif

    // here I can send notif to any buckets
    // every dedicated actor thread can handle UID outside of its bucket

    {
        logScopedProfiler("pushUIDQPending");
        const auto bucketCount = to_d(m_bucketList.size());
        for(int i = 0; i < bucketCount; ++i){
            if(m_bucketList.at((bucketId + i) % bucketCount).uidQPending.try_push(uid)){
                return true;
            }
        }

        m_bucketList.at(bucketId).uidQPending.push(uid);
        return true;
    }
}

void ActorPool::runOneUID(uint64_t uid)
{
    const auto workerId = getWorkerID();
    if(!isActorThread(workerId)){
        throw fflerror("running actor UID %llu by public thread", to_llu(uid));
    }

    Mailbox *mailboxPtr = nullptr;
    MailboxSubBucket::RLockGuard lockGuard;
    if(workerId == getBucketID(uid)){
        mailboxPtr = tryGetMailboxPtr(uid);
    }
    else{
        auto rlockedMailboxPtr = tryGetRLockedMailboxPtr(uid);
        lockGuard = std::move(rlockedMailboxPtr.first);
        mailboxPtr = rlockedMailboxPtr.second;
    }

    if(!mailboxPtr){
        return;
    }

    switch(MailboxLock grabLock(mailboxPtr->schedLock, workerId); grabLock.lockType()){
        case MAILBOX_READY:
            {
                if(lockGuard){
                    lockGuard.unlock();
                }

                runOneMailbox(mailboxPtr, false, 0ULL);
                return;
            }
        default:
            {
                return;
            }
    }
}

bool ActorPool::runOneMailbox(Mailbox *mailboxPtr, bool useMetronome, uint64_t startUpdateTime)
{
    if(!isActorThread()){
        throw fflerror("accessing actor message handlers outside of actor threads: %d", getWorkerID());
    }

    if(mailboxPtr->schedLock.detached()){   // don't need this check
        return false;                       // any thread want to call runOneMailbox should first grab the schedLoc
    }                                       // means it's in grabbed status rather than detached status if we can reach here

    // startup point trigger
    // user can provide a start up tigger function to do spawn notification
    if(mailboxPtr->atStart){
        mailboxPtr->atStart();
        mailboxPtr->atStart = nullptr;
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
        if(mailboxPtr->schedLock.detached()){   // don't need this check
            return false;                       // any thread want to call runOneMailbox should first grab the schedLoc
        }                                       // means it's in grabbed status rather than detached status if we can reach here

        if((mailboxPtr->actor->getMetronomeFreq() > 0.0) && (mailboxPtr->lastUpdateTime + std::lround(1000.0 / mailboxPtr->actor->getMetronomeFreq()) <= startUpdateTime)){
            raii_timer procTimer(&(mailboxPtr->monitor.procTick));
            mailboxPtr->actor->innHandler({AM_METRONOME, 0, 0});
            mailboxPtr->lastUpdateTime = startUpdateTime;
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
                // otherwise in clearOneMailbox() it will get handled again with AM_BADACTORPOD
                mailboxPtr->currQ.erase(mailboxPtr->currQ.begin(), p);
                return false;
            }

            const uint64_t timeNow = mailboxPtr->monitor.liveTimer.diff_nsec();
            if(timeNow < p->second){
                throw fflerror("monotonic clock error: %llu -> %llu", to_llu(p->second), to_llu(timeNow));
            }

            mailboxPtr->monitor.avgDelay.store((mailboxPtr->monitor.avgDelay.load() * 7 + (timeNow - p->second)) / 8);
            {
                raii_timer procTimer(&(mailboxPtr->monitor.procTick));
                mailboxPtr->actor->innHandler(p->first);
            }
            mailboxPtr->monitor.messageDone.fetch_add(1);
        }

        mailboxPtr->currQ.clear();

        // finished currQ without detach the actor
        // check when we are running currQ if any other thread post more messages to nextQ
    }
    throw fflreach();
}

void ActorPool::clearOneMailbox(Mailbox *mailboxPtr)
{
    // actor can detach and immedately deconstruct it self
    // when found a mailbox is detached never access it's actor pointer

    // clean one mailbox means:
    // 1. make sure all actor/application threads DONE posting to it
    // 2. make sure all actor/application threads can not post new messages to it
    // 3. make sure all queued message are responded with AM_BADACTORPOD

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
    // 7     return true                        | 7     ReplyBadPod(Mailbox.currQ);               // clearOneMailbox() ends
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

    logProfiler();
    {
        std::lock_guard<std::mutex> lockGuard(mailboxPtr->nextQLock);
        if(!mailboxPtr->nextQ.empty()){
            mailboxPtr->currQ.insert(mailboxPtr->currQ.end(), mailboxPtr->nextQ.begin(), mailboxPtr->nextQ.end());
        }
    }

    for(const auto &p: mailboxPtr->currQ){
        if(!p.first.from()){
            continue;
        }

        AMBadActorPod amBAP;
        std::memset(&amBAP, 0, sizeof(amBAP));

        amBAP.Type    = p.first.type();
        amBAP.from    = p.first.from();
        amBAP.ID      = p.first.seqID();
        amBAP.Respond = p.first.respID();
        amBAP.UID     = mailboxPtr->uid;
        postMessage(p.first.from(), {ActorMsgBuf(AM_BADACTORPOD, amBAP), 0, 0, p.first.seqID()});
    }
    mailboxPtr->currQ.clear();
}

void ActorPool::runOneMailboxBucket(int bucketId, uint64_t startUpdateTime)
{
    logProfiler();
    const int workerId = getWorkerID();
    if(workerId != bucketId){
        throw fflerror("udpate mailbox bucket %d by thread %d", bucketId, workerId);
    }

    const auto fnRunMailbox = [workerId, startUpdateTime, this](Mailbox *mailboxPtr) -> bool
    {
        // we can use raw mailboxPtr here because it's in dedicated actor thread
        // no one else can remove {uid, mailboxPtr} from the sub-bucket

        switch(MailboxLock grabLock(mailboxPtr->schedLock, workerId); const auto lockType = grabLock.lockType()){
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
                    return runOneMailbox(mailboxPtr, true, startUpdateTime);
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

            if(!mailboxPtr->schedLock.detached()){
                throw fflerror("clear mailbox while it's not detached: uid = %llu", to_llu(mailboxPtr->uid));
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

namespace _details
{
    class AutoCounter
    {
        private:
            std::atomic<size_t> &m_ref;

        public:
            AutoCounter(std::atomic<size_t> &ref)
                : m_ref(ref)
            {
                m_ref++;
            }

        public:
            ~AutoCounter()
            {
                m_ref--;
            }
    };
}

void ActorPool::launchPool()
{
    // one bucket has one dedicated thread
    // it will feed the update message METRNOME and can steal jobs when not busy

    for(int bucketId = 0; bucketId < to_d(m_bucketList.size()); ++bucketId){
        m_bucketList.at(bucketId).runThread = std::async(std::launch::async, [bucketId, this]()
        {
            // setup the worker id
            // for any other thread this will NOT get assigned
            t_workerID = bucketId;
            try{
                raii_timer timer;
                uint64_t lastUpdateTime = 0;
                const uint64_t maxUpdateWaitTime = 1000ULL / m_logicFPS;

                std::vector<uint64_t> uidList;
                uidList.reserve(2048);

                const _details::AutoCounter autoCounter(m_countRunning);
                while(true){
                    if(!uidList.empty()){
                        for(const auto uid: uidList){
                            runOneUID(uid);
                        }
                        uidList.clear();
                    }
                    else{
                        const uint64_t currTime = timer.diff_msec();
                        if(currTime >= lastUpdateTime + maxUpdateWaitTime){
                            runOneMailboxBucket(bucketId, currTime);
                            lastUpdateTime = currTime;
                        }

                        for(int i = 0; i < to_d(m_bucketList.size()) * 32; ++i){
                            const int currBucketId = (bucketId + i) % to_d(m_bucketList.size());
                            const size_t maxPopCount = (currBucketId == bucketId) ? 0 : 4;
                            if(m_bucketList[currBucketId].uidQPending.try_pop(uidList, maxPopCount) && !uidList.empty()){
                                break;
                            }
                        }

                        if(uidList.empty()){
                            int ec = 0;
                            const uint64_t exptUpdateTime = lastUpdateTime + maxUpdateWaitTime;
                            if(currTime < exptUpdateTime){
                                m_bucketList[bucketId].uidQPending.pop(uidList, 0, exptUpdateTime - currTime, ec);
                            }
                            else{
                                ec = E_TIMEOUT;
                            }

                            if(ec == E_QCLOSED){
                                break;
                            }
                            else if(ec == E_TIMEOUT){
                                // didn't get any pending UID
                                // and when reach here we are sure the thread needs to update the whole mailbox by METRONOME

                                // do nothing here
                                // hold for next loop
                            }
                            else if(ec == E_DONE){
                                if(uidList.empty()){
                                    throw fflerror("taskQ returns E_DONE with empty uid list");
                                }
                            }
                            else{
                                throw fflerror("uidQPending[bucketId = %d].pop() returns invalid result: %d", bucketId, ec);
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

bool ActorPool::checkUIDValid(uint64_t uid) const
{
    // always need to r-lock the sub-bucket even in dedicated actor thread
    // other bucket can spawn new actor to current sub-bucket

    logProfiler();
    const auto &subBucketCRef = getSubBucket(uid);
    MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);

    const auto p = subBucketCRef.mailboxList.find(uid);
    return p != subBucketCRef.mailboxList.end() && !(p->second->schedLock.detached());
}

bool ActorPool::isActorThread() const
{
    return isActorThread(getWorkerID());
}

bool ActorPool::isActorThread(int workerId) const
{
    return (workerId >= 0) && (workerId < to_d(m_bucketList.size()));
}

ActorMonitor ActorPool::getActorMonitor(uint64_t uid) const
{
    if(isActorThread()){
        throw fflerror("querying actor monitor inside actor thread: WorkerID = %d, UID = %llu", getWorkerID(), to_llu(uid));
    }

    const auto &subBucketCRef = getSubBucket(uid);
    {
        MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);
        if(const auto p = subBucketCRef.mailboxList.find(uid); p != subBucketCRef.mailboxList.end() && !p->second->schedLock.detached()){
            return p->second->dumpMonitor();
        }
    }
    return {};
}

std::vector<ActorMonitor> ActorPool::getActorMonitor() const
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

    std::vector<ActorMonitor> result;
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

ActorPool::Mailbox *ActorPool::tryGetMailboxPtr(uint64_t uid)
{
    // find the mailboxPtr without grabbing its schedLock
    // if not in dedicated actor thread, calling this function likely is a bug

    // because immediately when we release the r-lock
    // the dedicated actor thread can grab the schedLock and remove the {uid, mailboxPtr} pair form the sub-bucket

    logProfiler();
    auto &subBucketRef = getSubBucket(uid);
    MailboxSubBucket::RLockGuard lockGuard(subBucketRef.lock);
    if(auto p = subBucketRef.mailboxList.find(uid); p != subBucketRef.mailboxList.end()){
        // should I check if it's detached here?
        // it's not mandatory
        return p->second.get();
    }
    return nullptr;
}

std::pair<ActorPool::MailboxSubBucket::RLockGuard, ActorPool::Mailbox *> ActorPool::tryGetRLockedMailboxPtr(uint64_t uid)
{
    logProfiler();
    auto &subBucketRef = getSubBucket(uid);
    MailboxSubBucket::RLockGuard lockGuard(subBucketRef.lock);
    if(auto p = subBucketRef.mailboxList.find(uid); p != subBucketRef.mailboxList.end()){
        return {std::move(lockGuard), p->second.get()};
    }
    return {};
}

ActorPodMonitor ActorPool::getPodMonitor(uint64_t uid) const
{
    if(isActorThread()){
        throw fflerror("querying actor pod monitor inside actor thread: WorkerID = %d, UID = %llu", getWorkerID(), to_llu(uid));
    }

    const auto &subBucketCRef = getSubBucket(uid);
    {
        MailboxSubBucket::RLockGuard lockGuard(subBucketCRef.lock);
        if(const auto p = subBucketCRef.mailboxList.find(uid); p != subBucketCRef.mailboxList.end() && !p->second->schedLock.detached()){
            return p->second->actor->dumpPodMonitor();
        }
    }
    return {};
}
