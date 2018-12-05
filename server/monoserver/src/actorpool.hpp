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
#include <chrono>
#include <future>
#include <atomic>
#include <vector>
#include <thread>
#include <cstdint>
#include <shared_mutex>
#include "condcheck.hpp"
#include "raiitimer.hpp"
#include "messagepack.hpp"

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
        struct ActorMonitor
        {
            uint64_t UID;

            uint32_t LiveTick;
            uint32_t BusyTick;

            uint32_t MessageDone;
            uint32_t MessagePending;

            ActorMonitor(uint64_t nUID, uint32_t nLiveTick, uint32_t nBusyTick, uint32_t nMessageDone, uint32_t nMessagePending)
                : UID(nUID)
                , LiveTick(nLiveTick)
                , BusyTick(nBusyTick)
                , MessageDone(nMessageDone)
                , MessagePending(nMessagePending)
            {}

            ActorMonitor()
                : UID(0)
            {}

            operator bool () const
            {
                return UID != 0;
            }
        };

        struct ActorThreadMonitor
        {
            int      ThreadID;
            uint64_t ActorCount;

            uint32_t LiveTick;
            uint32_t BusyTick;
        };

    private:
        static void Backoff(uint32_t &nBackoff)
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
        struct SpinLock
        {
            std::atomic_flag Latch;
            SpinLock()
                : Latch {ATOMIC_FLAG_INIT}
            {}

            void lock()
            {
                uint32_t nBackoff = 0;
                while (Latch.test_and_set(std::memory_order_acquire)){
                    Backoff(nBackoff);
                }
            }

            void unlock()
            {
                Latch.clear(std::memory_order_release);
            }
        };

    private:
        template<size_t AVG_LEN = 16> struct AvgTimer
        {
            std::atomic<long> CurrSum;

            size_t Curr;
            std::array<long, AVG_LEN> Array;

            AvgTimer()
                : CurrSum{0}
                , Curr(0)
                , Array()
            {
                for(size_t nIndex = 0; nIndex < Array.size(); ++nIndex){
                    Array[nIndex] = 0;
                }
            }

            void Push(long nNewTime)
            {
                Curr = ((Curr + 1) % Array.size());
                CurrSum.fetch_add(nNewTime - Array[Curr]);
            }

            long GetAvgTime() const
            {
                return CurrSum.load() / Array.size();
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
                std::atomic<int> m_Status;

            public:
                MailboxMutex()
                    : m_Status(MAILBOX_READY)
                {}

                int Detach()
                {
                    return m_Status.exchange(MAILBOX_DETACHED);
                }

                bool Detached() const
                {
                    return m_Status.load() == MAILBOX_DETACHED;
                }
        };

        class MailboxLock
        {
            private:
                int m_Expected;
                int m_WorkerID;

            private:
                MailboxMutex &m_MutexRef;

            public:
                MailboxLock(MailboxMutex &rstMutex, int nWorkerID)
                    : m_Expected(MAILBOX_READY)
                    , m_WorkerID(MAILBOX_ERROR)
                    , m_MutexRef(rstMutex)
                {
                    // need to save the worker id
                    // since the mailbox may get detached quietly
                    if(m_MutexRef.m_Status.compare_exchange_strong(m_Expected, nWorkerID)){
                        m_WorkerID = nWorkerID;
                    }
                }

                ~MailboxLock()
                {
                    if(Locked()){
                        m_Expected = m_WorkerID;
                        if(!m_MutexRef.m_Status.compare_exchange_strong(m_Expected, MAILBOX_READY)){
                            if(m_Expected != MAILBOX_DETACHED){
                                // big error here and should never happen
                                // someone stolen the mailbox without accquire the lock
                                condcheck(m_Expected == MAILBOX_DETACHED);
                            }
                        }
                    }
                }

                MailboxLock(const MailboxLock &) = delete;
                MailboxLock &operator = (MailboxLock) = delete;

            public:
                bool Locked() const
                {
                    return LockType() == MAILBOX_READY;
                }

                int LockType() const
                {
                    return m_Expected;
                }
        };

        struct Mailbox
        {
            ActorPod    *Actor;
            MailboxMutex SchedLock;
            SpinLock     NextQLock;

            std::vector<MessagePack> CurrQ;
            std::vector<MessagePack> NextQ;

            std::function<void()> AtExit;

            // put a monitor structure and always maintain it
            // then no need to acquire SchedLock to dump the monitor
            struct MailboxMonitor
            {
                uint64_t   UID;
                hres_timer LiveTimer;

                std::atomic<uint64_t> ProcTick;
                std::atomic<uint32_t> MessageDone;
                std::atomic<uint32_t> MessagePending;

                MailboxMonitor(uint64_t nUID)
                    : UID(nUID)
                    , LiveTimer()
                    , ProcTick {0}
                    , MessageDone {0}
                    , MessagePending {0}
                {}
            } Monitor;

            auto DumpMonitor() const
            {
                return ActorMonitor
                {
                    Monitor.UID,
                    (uint32_t)(Monitor.LiveTimer.diff_msec()),
                    (uint32_t)(Monitor.ProcTick.load() / 1000000),
                    Monitor.MessageDone.load(),
                    Monitor.MessagePending.load(),
                };
            }

            // put ctor in actorpool.cpp
            // ActorPod is incomplete type in actorpool.hpp
            Mailbox(ActorPod *);
        };

        struct MailboxBucket
        {
            AvgTimer<32> RunTimer;
            AvgTimer<32> StealTimer;

            mutable std::shared_mutex BucketLock;
            std::map<uint64_t, std::shared_ptr<Mailbox>> MailboxList;
        };

    private:
        const uint32_t m_LogicFPS;

    private:
        std::atomic<bool> m_Terminated;

    private:
        std::vector<std::shared_future<bool>> m_FutureList;

    private:
        std::vector<MailboxBucket> m_BucketList;

    private:
        std::mutex m_ReceiverLock;
        std::map<uint64_t, Receiver *> m_ReceiverList;

    public:
        ActorPool(uint32_t = 23, uint32_t = 30);

    public:
        ~ActorPool();

    private:
        bool IsActorThread()    const;
        bool IsActorThread(int) const;

    private:
        bool Register(Receiver *);
        bool Register(ActorPod *);

    private:
        bool Detach(const Receiver *);
        bool Detach(const ActorPod *, const std::function<void()> &);

    public:
        void Launch();
        bool CheckInvalid(uint64_t) const;

    private:
        static bool IsReceiver(uint64_t nUID)
        {
            return nUID & 0XFFFF000000000000;
        }

    private:
        uint64_t GetInnActorUID();

    private:
        bool PostMessage(uint64_t, MessagePack);

    private:
        bool HasWorkSteal() const
        {
            static bool bEnabled= std::getenv("MIR2X_ENABLE_WORK_STEAL");
            return bEnabled && (m_BucketList.size() > 1);
        }

    private:
        static uint64_t CreateReceiverUID()
        {
            static std::atomic<uint64_t> s_RecvUID(1);
            return 0XFFFF000000000000 + s_RecvUID.fetch_add(1);
        }

    private:
        std::tuple<long, size_t> CheckWorkerTime() const;

    private:
        void RunWorker(size_t);
        void RunWorkerSteal(size_t);
        void RunWorkerOneLoop(size_t);
        bool RunOneMailbox(Mailbox *, bool);

    private:
        void ClearOneMailbox(Mailbox *);

    public:
        size_t ActorThreadCount() const
        {
            // always thread safe
            // never change the bucket size after construction
            return m_BucketList.size();
        }

    public:
        ActorMonitor GetActorMonitor(uint64_t) const;
        std::vector<ActorMonitor> GetActorMonitor() const;
};
