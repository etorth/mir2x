/*
 * =====================================================================================
 *
 *       Filename: actorpool.hpp
 *        Created: 08/16/2018 23:44:37
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

#include <atomic>
#include <vector>
#include <cstdint>

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
        int HashCode(uint32_t nUID)
        {
        }

    private:
        struct Mailbox final
        { 
            SpinLock Lock;
            std::vector<MessagePack> CurrQ;
            std::vector<MessagePack> NextQ;

            AcotorPod *Actor;
            std::atomic<int> Reg;

            Mailbox(ActorPod *pActor)
                : Lock()
                , CurrQ()
                , NextQ()
                , Actor(pActor)
                , Reg{0}
            {}

            void PushMessage(const MessagePack *pMPK, size_t nNum)
            {
            }

            void Execute()
            {
                if(CurrQ.empty()){
                    std::lock_guard<SpinLock> stLockGuard(Lock);
                    if(NextQ.empty()){
                        return;
                    }
                    std::swap(CurrQ, NextQ);
                }

                for(auto p = CurrQ.begin(); p != CurrQ.end(); ++p){
                    if(Reg.fetch_and(REG_STOPPING)){
                        Actor->PostMessage(p->From(), {MPK_BADACTORPOD});
                        continue;
                    }

                    Reg.fetch_or(REG_OPERATING);
                    Actor->OperateAM(*p);
                    Reg.fetch_xor(REG_OPERATING);
                }
            }
        };

    public:
        void LinkUID(uint32_t nUID, ActorPod *pActor)
        {
            if(!nUID || !pActor){
                return;
            }

            auto nChainNode = HashCode(nUID);
            if(m_UIDArray[nChainNode].Loc0.compare_exchange_strong(-1, nChainNode)){
                m_UIDArray[nChainNode].MailboxPtr.load(new Mailbox(pActor));
                return;
            }

            // else go through the whole chain
        }

    public:
        void EraseUID(uint32_t nUID)
        {
            if(auto nChainNode = InnGet(nUID); nChainNode >= 0){
                delete m_UIDArray[nChainNode].MailboxPtr.exchange(nullptr);
            }
        }
};
