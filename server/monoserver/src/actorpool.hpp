/*
 * =====================================================================================
 *
 *       Filename: actorpool.hpp
 *        Created: 08/16/2018 23:44:37
 *    Description: simple actor model to replace libtheron
 *                 1. use uint32_t as address
 *                 2. use MessagePack as message type
 *                 3. support efficient call to CheckInvalid()
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
#include <atomic>
#include <vector>
#include <cstdint>

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
        struct ActorMailbox
        {
            std::vector<MessagePack> CurrQ;
            std::vector<MessagePack> NextQ;

            SpinLock  NextQLock;
            ActorPod *Actor;

            ActorMailbox(ActorPod *pActor)
                : Actor(pActor)
            {}
        };

        struct MailboxBucket
        {
            std::thread::id ID;
            std::shared_lock BucketLock;
            std::map<uint64_t, std::shared_ptr<ActorMailbox>> MailboxList;
        };

    private:
        const uint32_t m_BucketCount;

    private:
        std::atomic<bool> m_Terminated;

    private:
        std::vector<std::shared_future<bool>> m_Futures;

    private:
        MailboxBucket m_BucketArray[m_BucketCount];

    private:
        std::mutex m_ReceiverLock;
        std::map<uint64_t, SyncDriver *> m_ReceiverList;

    public:
        ActorPool(int = 0);

    public:
        ~ActorPod();

    private:
        bool IsActorThread() const
        {
            for(auto p = m_BucketArray; p != m_BucketArray + m_BucketCount; ++p){
                if(std::this_thread::get_id() == p->ID){
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
        void Detach(const ActorPod *);

    public:
        void Launch();
        bool CheckInvalid(uint64_t) const;

    private:
        static bool IsReceiver(uint64_t nUID)
        {
            return nUID & 0XFFFF000000000000;
        }
};
