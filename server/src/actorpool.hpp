#pragma once
#include <map>
#include <mutex>
#include <array>
#include <queue>
#include <chrono>
#include <future>
#include <atomic>
#include <vector>
#include <thread>
#include <memory>
#include <cstdint>
#include <shared_mutex>
#include <unordered_map>
#include "uidf.hpp"
#include "raiitimer.hpp"
#include "actormsgpack.hpp"
#include "actormonitor.hpp"
#include "parallel_hashmap/phmap.h"

class ActorPod;
class Receiver;
class Dispatcher;
class SyncDriver;

class ActorPool final
{
    private:
        friend class ActorPod;
        friend class Receiver;
        friend class Dispatcher;
        friend class SyncDriver;

    public:
        struct UIDComper
        {
            bool operator () (uint64_t x, uint64_t y) const
            {
                return uidf::getUIDType(x) > uidf::getUIDType(y);
            }
        };

        class UIDPriorityQueue
        {
            private:
                struct UIDPQImpl: public std::priority_queue<uint64_t, std::vector<uint64_t>, UIDComper>
                {
                    UIDPQImpl()
                    {
                        this->c.reserve(2048);
                    }

                    void uidSwap(std::vector<uint64_t> &uidList)
                    {
                        std::swap(uidList, this->c);
                    }
                };

            private:
                UIDPQImpl m_PQ;
                phmap::flat_hash_set<uint64_t> m_uidSet;

            public:
                uint64_t pick_top()
                {
                    const auto uidTop = m_PQ.top();
                    m_PQ.pop();

                    m_uidSet.erase(uidTop);
                    return uidTop;
                }

                void pick_top(std::vector<uint64_t> &uidList, size_t maxPop)
                {
                    uidList.clear();
                    if(maxPop > 0){
                        for(size_t i = 0; i < maxPop; ++i){
                            if(empty()){
                                break;
                            }
                            uidList.push_back(pick_top());
                        }
                    }
                    else{
                        m_PQ.uidSwap(uidList);
                        m_uidSet.clear();
                    }
                }

            public:
                bool empty() const
                {
                    return m_PQ.empty();
                }

                size_t size() const
                {
                    return m_PQ.size();
                }

            public:
                bool push(uint64_t uid)
                {
                    if(m_uidSet.contains(uid)){
                        return false;
                    }

                    m_PQ.push(uid);
                    m_uidSet.insert(uid);
                    return true;
                }
        };

    private:
        template<typename L> class TryLockGuard
        {
            private:
                L &m_lockRef;
                const bool m_locked;

            public:
                explicit TryLockGuard(L &lock)
                    : m_lockRef(lock)
                    , m_locked(lock.try_lock())
                {}

                ~TryLockGuard()
                {
                    if(m_locked){
                        m_lockRef.unlock();
                    }
                }

                operator bool () const
                {
                    return m_locked;
                }

            private:
                TryLockGuard              (const TryLockGuard &) = delete;
                TryLockGuard & operator = (const TryLockGuard &) = delete;
        };

        enum
        {
            E_DONE    = 0,  // no error
            E_QCLOSED = 1,  // queue closed
            E_TIMEOUT = 2,  // wait timeout
        };

        class UIDQueue final
        {
            private:
                bool m_closed = false;
                UIDPriorityQueue m_uidQ;

            private:
                mutable std::mutex m_lock;
                mutable std::condition_variable m_cond;

            public:
                UIDQueue() = default;

            public:
                bool try_push(uint64_t uid)
                {
                    bool added = false;
                    {
                        TryLockGuard<decltype(m_lock)> lockGuard(m_lock);
                        if(!lockGuard){
                            return false;
                        }
                        added = m_uidQ.push(uid);
                    }

                    if(added){
                        m_cond.notify_one();
                    }
                    return true;
                }

                bool try_pop(std::vector<uint64_t> &uidList, size_t maxPop)
                {
                    TryLockGuard<decltype(m_lock)> lockGuard(m_lock);
                    if(!lockGuard || m_uidQ.empty()){
                        return false;
                    }

                    m_uidQ.pick_top(uidList, maxPop);
                    return true;
                }

            public:
                void push(uint64_t uid)
                {
                    bool added = false;
                    {
                        std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                        added = m_uidQ.push(uid);
                    }

                    if(added){
                        m_cond.notify_one();
                    }
                }

                void pop(std::vector<uint64_t> &uidList, size_t maxPop, uint64_t msec, int &ec)
                {
                    std::unique_lock<decltype(m_lock)> lockGuard(m_lock);
                    if(msec > 0){
                        const bool wait_res = m_cond.wait_for(lockGuard, std::chrono::milliseconds(msec), [this]() -> bool
                        {
                            return m_closed || !m_uidQ.empty();
                        });

                        if(wait_res){
                            // pred returns true
                            // means either not expired, or even expired but the pred evals to true now

                            // when queue is closed AND there are still tasks in m_uidQ
                            // what I should do ???

                            // currently I returns the task pending in the m_uidQ
                            // so a UIDQueue can be closed but you can still pop task from it

                            if(!m_uidQ.empty()){
                                ec = E_DONE;
                                m_uidQ.pick_top(uidList, maxPop);
                            }
                            else if(m_closed){
                                ec = E_QCLOSED;
                            }
                            else{
                                // UIDQueue is not closed and m_uidQ is empty
                                // then pred evals to true, can only be time expired
                                ec = E_TIMEOUT;
                            }
                        }
                        else{
                            // by https://en.cppreference.com/w/cpp/thread/condition_variable_any/wait_for
                            // when wait_res returns false:
                            // 1. the time has been expired
                            // 2. the pred still returns false, means:
                            //      1. queue is not closed, and
                            //      2. m_uidQ is still empty
                            ec = E_TIMEOUT;
                        }
                    }
                    else{
                        m_cond.wait(lockGuard, [this]() -> bool
                        {
                            return m_closed || !m_uidQ.empty();
                        });

                        // when there is task in m_uidQ
                        // we always firstly pick & return the task before report E_CLOSED

                        if(!m_uidQ.empty()){
                            ec = E_DONE;
                            m_uidQ.pick_top(uidList, maxPop);
                        }
                        else{
                            ec = E_QCLOSED;
                        }
                    }
                }

            public:
                void close()
                {
                    {
                        std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                        m_closed = true;
                    }
                    m_cond.notify_all();
                }

                size_t size_hint() const
                {
                    std::lock_guard<decltype(m_lock)> lockGuard(m_lock);
                    return m_uidQ.size();
                }
        };

    public:
        enum
        {
            MAILBOX_ERROR          = -4,
            MAILBOX_DETACHED       = -3,
            MAILBOX_READY          = -2,
            MAILBOX_ACCESS_PUB     = -1,
            MAILBOX_ACCESS_WORKER0 =  0,
            MAILBOX_ACCESS_WORKER1 =  1,
        };

    private:
        class MailboxLock;
        class MailboxMutex
        {
            private:
                friend class MailboxLock;

            private:
                // make the atomic status encapsulated
                // then no one can legally access it except the MailboxLock
                std::atomic<int> m_status;

            public:
                MailboxMutex()
                    : m_status(MAILBOX_READY)
                {}

                int detach()
                {
                    return m_status.exchange(MAILBOX_DETACHED);
                }

                bool detached() const
                {
                    return m_status.load() == MAILBOX_DETACHED;
                }
        };

        class MailboxLock
        {
            private:
                int m_expected;
                int m_workerID;

            private:
                MailboxMutex &m_mutexRef;

            public:
                MailboxLock(MailboxMutex &rstMutex, int nWorkerID)
                    : m_expected(MAILBOX_READY)
                    , m_workerID(MAILBOX_ERROR)
                    , m_mutexRef(rstMutex)
                {
                    // need to save the worker id
                    // since the mailbox may get detached quietly
                    if(m_mutexRef.m_status.compare_exchange_strong(m_expected, nWorkerID)){
                        m_workerID = nWorkerID;
                    }
                }

                ~MailboxLock()
                {
                    if(locked()){
                        m_expected = m_workerID;
                        if(!m_mutexRef.m_status.compare_exchange_strong(m_expected, MAILBOX_READY)){
                            if(m_expected != MAILBOX_DETACHED){
                                // big error here and should never happen
                                // someone stolen the mailbox without accquire the lock
                                std::abort();
                            }
                        }
                    }
                }

                MailboxLock(const MailboxLock &) = delete;
                MailboxLock &operator = (MailboxLock) = delete;

            public:
                bool locked() const
                {
                    return lockType() == MAILBOX_READY;
                }

                int lockType() const
                {
                    return m_expected;
                }
        };

    private:
        struct Mailbox
        {
            // uid is duplicated
            // used only when mailbox detached, actor is NULL

            const uint64_t uid = 0;
            ActorPod    *actor = 0;

            MailboxMutex schedLock;
            std::mutex   nextQLock;

            std::vector<std::pair<ActorMsgPack, uint64_t>> currQ;
            std::vector<std::pair<ActorMsgPack, uint64_t>> nextQ;

            std::function<void()> atStart;
            std::function<void()> atExit;

            // pool can automatically send METRONOME to actorpod
            // this record last send time, in ms
            uint64_t lastUpdateTime = 0;

            // put a monitor structure and always maintain it
            // then no need to acquire schedLock to dump the monitor
            struct MailboxMonitor
            {
                hres_timer liveTimer;
                std::atomic<uint64_t> avgDelay{0};
                std::atomic<uint64_t> procTick{0};
                std::atomic<uint32_t> messageDone{0};
                std::atomic<uint32_t> messagePending{0};
            } monitor;

            auto dumpMonitor() const
            {
                return ActorMonitor
                {
                    to_u64(uid),
                    to_u32(monitor.avgDelay.load() / 1000000ULL),
                    to_u32(monitor.liveTimer.diff_msec()),
                    to_u32(monitor.procTick.load() / 1000000ULL),
                    to_u32(monitor.messageDone.load()),
                    to_u32(monitor.messagePending.load()),
                };
            }

            // put ctor in actorpool.cpp
            // ActorPod is incomplete type in actorpool.hpp
            Mailbox(ActorPod *, std::function<void()>);
        };

        struct MailboxSubBucket
        {
            // dirty part, when calling runOneMailboxBucket() we can't prevent current sub-bucket from rehashing, because:
            //
            //   1. we can't r/w-lock current sub-bucket when calling actor message handler, since message handler may call attach/detach and this causes lock twice
            //   2. job-stealing by other actor thread can also spawn new actors and insert to current sub-bucket
            //
            // we can't iterate over the sub-bucket if rehashing happens
            // so we put this pointer array as cache, we alternatively iterate over this cached mailbox list
            //
            // we update the cache whenever needed
            // don't need a lock since it can ONLY be accessed by dedicated actor thread
            //
            // we guarantee mailbox in this cache list is a subset of the mailboxList
            // otherwise it cases multi-update in one loop and more dangeriously: access a mailbox by pointer but which can be delted already
            std::vector<Mailbox *> mailboxListCache;

            // protected mailbox list
            // always need to r/w-lock before change it structurally
            mutable std::shared_mutex lock;
            phmap::flat_hash_map<uint64_t, std::unique_ptr<Mailbox>> mailboxList;

            using RLockGuard = std::shared_lock<std::shared_mutex>;
            using WLockGuard = std::unique_lock<std::shared_mutex>;
        };

    private:
        constexpr static int m_subBucketCount = 97;
        struct MailboxBucket
        {
            std::future<void> runThread;
            UIDQueue uidQPending;
            std::array<MailboxSubBucket, m_subBucketCount> subBucketList;
        };

    private:
        const uint32_t m_logicFPS;

    private:
        std::atomic<size_t> m_countRunning {0};
        std::vector<MailboxBucket> m_bucketList;

    private:
        static void backOff(uint64_t &nBackoff)
        {
            nBackoff++;

            if(nBackoff < 20){
                return;
            }

            if(nBackoff < 50){
                std::this_thread::yield();
                return;
            }

            if(nBackoff < 100){
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                return;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }

    private:
        template<size_t AVG_LEN = 16> class AvgTimer
        {
            private:
                std::atomic<long> m_currSum;

            private:
                size_t m_curr;
                std::array<long, AVG_LEN> m_array;

            public:
                AvgTimer()
                    : m_currSum{0}
                    , m_curr(0)
                {
                    static_assert(AVG_LEN > 0);
                    m_array.fill(0);
                }

            public:
                void push(long newTime)
                {
                    m_curr = ((m_curr + 1) % m_array.size());
                    m_currSum.fetch_add(newTime - m_array[m_curr]);
                }

                long getAvgTime() const
                {
                    return m_currSum.load() / m_array.size();
                }
        };

    private:
        std::mutex m_receiverLock;
        std::unordered_map<uint64_t, Receiver *> m_receiverList;

    public:
        ActorPool(int, int);

    public:
        ~ActorPool();

    public:
        bool running() const
        {
            return m_countRunning >= m_bucketList.size();
        }

    private:
        bool isActorThread()    const;
        bool isActorThread(int) const;

    private:
        void attach(Receiver *);
        void attach(ActorPod *, std::function<void()>);

    private:
        void detach(const Receiver *);
        void detach(const ActorPod *, std::function<void()>);

    public:
        void launchPool();
        bool checkUIDValid(uint64_t) const;

    private:
        uint64_t GetInnActorUID();

    private:
        bool postMessage(uint64_t, ActorMsgPack);

    private:
        void runOneUID(uint64_t);
        bool runOneMailbox(Mailbox *, bool, uint64_t);
        void runOneMailboxBucket(int, uint64_t);

    private:
        void clearOneMailbox(Mailbox *);

    public:
        ActorMonitor getActorMonitor(uint64_t) const;
        std::vector<ActorMonitor> getActorMonitor() const;

    public:
        ActorPodMonitor getPodMonitor(uint64_t) const;

    public:
        int getBucketID(uint64_t uid) const
        {
            return static_cast<int>(uid % to_u64(m_bucketList.size()));
        }

        int getSubBucketID(uint64_t uid) const
        {
            return static_cast<int>((uid / to_u64(m_bucketList.size())) % to_u64(m_subBucketCount));
        }

    public:
        const MailboxSubBucket & getSubBucket(int bucketId, int subBucketId) const
        {
            return const_cast<ActorPool *>(this)->getSubBucket(bucketId, subBucketId);
        }

        MailboxSubBucket & getSubBucket(int bucketId, int subBucketId)
        {
            return m_bucketList.at(bucketId).subBucketList.at(subBucketId);
        }

    public:
        const MailboxSubBucket & getSubBucket(uint64_t uid) const
        {
            return const_cast<ActorPool *>(this)->getSubBucket(uid);
        }

        MailboxSubBucket & getSubBucket(uint64_t uid)
        {
            return getSubBucket(getBucketID(uid), getSubBucketID(uid));
        }

    private:
        Mailbox *tryGetMailboxPtr(uint64_t);
        std::pair<MailboxSubBucket::RLockGuard, Mailbox *> tryGetRLockedMailboxPtr(uint64_t);
};
