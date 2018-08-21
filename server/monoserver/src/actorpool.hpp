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
        constexpr uint32_t m_UIDArrayNum = 23;

    public:
        bool LinkUID(uint32_t nUID, ActorPod *pActor)
        {
            if(nUID && pActor){
                auto &rstBucket = m_UIDArray[nUID % m_UIDArrayNum];
                {
                    std::unique_lock<std::shared_mutex> stLock(rstBucket.Lock);
                    if(rstBucket.List.find(nUID) != rstBucket.end()){
                        rstBucket.List[nUID] = pActor;
                        return true;
                    }
                }
            }
            return false;
        }

    public:
        void EraseUID(uint32_t nUID)
        {


            if(auto nChainNode = InnGet(nUID); nChainNode >= 0){
                delete m_UIDArray[nChainNode].MailboxPtr.exchange(nullptr);
            }
        }
};
