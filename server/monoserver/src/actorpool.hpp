/*
 * =====================================================================================
 *
 *       Filename: actorpool.hpp
 *        Created: 09/02/2018 18:20:15
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
#include "asyncf.hpp"
#include "condcheck.hpp"
#include "raiitimer.hpp"
#include "messagepack.hpp"
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
            uint64_t uid;
            bool operator < (const UIDComper &other) const
            {
                return uidf::getUIDType(uid) > uidf::getUIDType(other.uid);
            }
        };

        class UIDPriorityQueue
        {
            private:
                std::priority_queue<UIDComper> m_PQ;
                phmap::flat_hash_set<uint64_t> m_uidSet;

            public:
                uint64_t pick_top()
                {
                    const auto top = m_PQ.top().uid;
                    m_PQ.pop();
                    m_uidSet.erase(top);
                    return top;
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
                void push(uint64_t uid)
                {
                    if(m_uidSet.contains(uid)){
                        return;
                    }

                    m_PQ.push(UIDComper(uid));
                    m_uidSet.insert(uid);
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
                                condcheck(m_expected == MAILBOX_DETACHED);
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
            ActorPod    *actor;
            MailboxMutex schedLock;
            std::mutex   nextQLock;

            std::vector<MessagePack> currQ;
            std::vector<MessagePack> nextQ;

            std::function<void()> atExit;

            // put a monitor structure and always maintain it
            // then no need to acquire schedLock to dump the monitor
            struct MailboxMonitor
            {
                uint64_t   uid;
                hres_timer liveTimer;

                std::atomic<uint64_t> procTick{0};
                std::atomic<uint32_t> messageDone{0};
                std::atomic<uint32_t> messagePending{0};

                MailboxMonitor(uint64_t argUID)
                    : uid(argUID)
                {}
            } monitor;

            auto dumpMonitor() const
            {
                return ActorMonitor
                {
                    to_u64(monitor.uid),
                    to_u32(monitor.liveTimer.diff_msec()),
                    to_u32(monitor.procTick.load() / 1000000ULL),
                    to_u32(monitor.messageDone.load()),
                    to_u32(monitor.messagePending.load()),
                };
            }

            // put ctor in actorpool.cpp
            // ActorPod is incomplete type in actorpool.hpp
            Mailbox(ActorPod *);
        };

        struct MailboxSubBucket
        {
            mutable std::shared_mutex lock;
            phmap::flat_hash_map<uint64_t, std::unique_ptr<Mailbox>> mailboxList;

            using RLockGuard = std::shared_lock<decltype(lock)>;
            using WLockGuard = std::unique_lock<decltype(lock)>;
        };

    private:
        constexpr static int m_subBucketCount = 13;
        struct MailboxBucket
        {
            std::future<void> runThread;
            asyncf::taskQ<uint64_t, UIDPriorityQueue> uidQPending;
            std::array<MailboxSubBucket, m_subBucketCount> subBucketList;
        };

    private:
        const uint32_t m_logicFPS;
        std::vector<MailboxBucket> m_bucketList;

    public:
        struct ActorMonitor
        {
            uint64_t uid = 0;

            uint32_t liveTick = 0;
            uint32_t busyTick = 0;

            uint32_t messageDone    = 0;
            uint32_t messagePending = 0;

            operator bool () const
            {
                return uid != 0;
            }
        };

        struct ActorThreadMonitor
        {
            int      threadId;
            uint64_t actorCount;

            uint32_t liveTick;
            uint32_t busyTick;
        };

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

    private:
        bool isActorThread()    const;
        bool isActorThread(int) const;

    private:
        void attach(Receiver *);
        void attach(ActorPod *);

    private:
        void detach(const Receiver *);
        void detach(const ActorPod *, const std::function<void()> &);

    public:
        void launchPool();
        bool checkInvalid(uint64_t) const;

    private:
        uint64_t GetInnActorUID();

    private:
        bool postMessage(uint64_t, MessagePack);

    private:
        bool runOneUID(uint64_t);
        bool runOneMailbox(Mailbox *, bool);
        void runOneMailboxBucket(int);

    private:
        void clearOneMailbox(Mailbox *);

    public:
        ActorMonitor getActorMonitor(uint64_t) const;
        std::vector<ActorMonitor> getActorMonitor() const;

    public:
        int pickThreadID() const;

    private:
        Mailbox *findMailbox(uint64_t);

    public:
        int getBucketID(uint64_t uid) const
        {
            return static_cast<int>(uid % (uint64_t)(m_bucketList.size()));
        }

        int getSubBucketID(uint64_t uid) const
        {
            return static_cast<int>(uid % (uint64_t)(m_subBucketCount));
        }

        const MailboxSubBucket & getSubBucket(int bucketId, int subBucketId) const
        {
            return m_bucketList.at(bucketId).subBucketList.at(subBucketId);
        }

        MailboxSubBucket & getSubBucket(int bucketId, int subBucketId)
        {
            return const_cast<ActorPool *>(this)->getSubBucket(bucketId, subBucketId);
        }

        const MailboxSubBucket & getSubBucket(uint64_t uid) const
        {
            return getSubBucket(getBucketID(uid), getSubBucketID(uid));
        }

        MailboxSubBucket & getSubBucket(uint64_t uid)
        {
            return const_cast<ActorPool *>(this)->getSubBucket(uid);
        }
};
