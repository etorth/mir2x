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
#include <map>
#include <mutex>
#include <future>
#include <atomic>
#include <vector>
#include <thread>
#include <cstdint>
#include <shared_mutex>
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
            mutable std::shared_mutex BucketLock;
            std::map<uint64_t, std::shared_ptr<ActorMailbox>> MailboxList;
        };

    private:
        const uint32_t m_BucketCount;

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

    private:
        std::atomic<uint64_t> m_InnActorUID;

    public:
        ActorPool(uint32_t = 0);

    public:
        ~ActorPool();

    private:
        bool IsActorThread() const
        {
            for(auto p = m_BucketList.begin(); p != m_BucketList.end(); ++p){
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
};
