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
#include <future>
#include <atomic>
#include <vector>
#include <thread>
#include <cstdint>
#include <shared_mutex>
#include "condcheck.hpp"
#include "messagepack.hpp"

class Receiver;
class ActorPod;
class ActorPool final
{
    private:
        struct SpinLock
        {
            std::atomic_flag Latch;
            SpinLock()
                : Latch {ATOMIC_FLAG_INIT}
            {}

            void lock()
            {
                while (Latch.test_and_set(std::memory_order_acquire)){
                    continue;
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

    private:
        struct Mailbox
        {
            // status of current mailbox/actor
            //   0 : [r]eady
            //   1 : [b]unning in one thread
            //   2 : [d]etached
            // can change betwwen ready/busy before jumps to detached
            // use MailboxLock for RAII access
            std::atomic<char> Status;

            AvgTimer<32> RunTimer;
            AvgTimer<32> StealTimer;

            ActorPod *Actor;
            SpinLock  NextQLock;

            std::vector<MessagePack> CurrQ;
            std::vector<MessagePack> NextQ;

            Mailbox(ActorPod *pActor)
                : Status('R')
                , RunTimer()
                , StealTimer()
                , Actor(pActor)
                , NextQLock()
                , CurrQ()
                , NextQ()
            {}
        };

        struct MailboxLock
        {
            char Expected;
            Mailbox &MailboxRef;

            MailboxLock(Mailbox &rstMailbox)
                : Expected('R')
                , MailboxRef(rstMailbox)
            {
                MailboxRef.Status.compare_exchange_strong(Expected, 'B');
            }

            ~MailboxLock()
            {
                if(Locked()){
                    // when actor has been detached
                    // here the compare_exchange_strong() will fail
                    Expected = 'B';
                    if(!MailboxRef.Status.compare_exchange_strong(Expected, 'R')){
                        condcheck(Expected == 'D');
                    }
                }
            }

            MailboxLock(const MailboxLock &) = delete;
            MailboxLock &operator = (MailboxLock) = delete;

            bool Locked() const
            {
                return LockType() == 'R';
            }

            char LockType() const
            {
                return Expected;
            }
        };

        struct MailboxBucket
        {
            std::thread::id WorkerID;
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
        ActorPool(uint32_t = 23, uint32_t = 5);

    public:
        ~ActorPool();

    private:
        bool IsActorThread() const
        {
            for(auto p = m_BucketList.begin(); p != m_BucketList.end(); ++p){
                if(std::this_thread::get_id() == p->WorkerID){
                    return true;
                }
            }
            return false;
        }

    private:
        friend class ActorPod;

    private:
        bool Register(ActorPod *);

    private:
        bool Detach(const ActorPod *);

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
        bool PostMessage(uint64_t nUID, const MessagePack &rstMPK)
        {
            return PostMessage(nUID, &rstMPK, 1);
        }

    private:
        bool PostMessage(uint64_t, const MessagePack *, size_t);

    private:
        bool HasWorkSteal() const
        {
            static bool bDisabled = std::getenv("MIR2X_DISABLE_WORK_STEAL");
            return !bDisabled && (m_BucketList.size() > 1);
        }

    private:
        static uint64_t CreateReceiverUID()
        {
            static std::atomic<uint64_t> s_RecvUID(1);
            return 0XFFFF000000000000 + s_RecvUID.fetch_add(1);
        }
};
