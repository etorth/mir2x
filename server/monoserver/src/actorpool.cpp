/*
 * =====================================================================================
 *
 *       Filename: actorpool.cpp
 *        Created: 08/16/2018 23:44:41
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

ActorPool::ActorPool()
{
}

bool ActorPool::PostMessage(uint32_t nUID, const MessagePack &rstMPK)
{
    return PostMessage(nUID, &rstMPK, 1);
}

void ActorPool::PostMessage(uint32_t nUID, const MessagePack *pMPK, size_t nMPKNum)
{
    for(auto nChainNode = m_UIDArray[InnHashCode(nUID)].Loc0.load(); nChainNode >= 0;){
        if(auto pActor = m_UIDArray[nChainNode].Actor.load(); pActor && pActor->UID() == nUID){
            if(pActor->Dead()){
                for(size_t nIndex = 0; nIndex < nMPKNum; ++nIndex){
                    PostMessage(pMPK[nIndex].From(), {MPK_BADACTORPOD, 0, pMPK[nIndex].Resp()});
                }
                return;
            }
            return pActor->PushMPK(pMPK, nMPKNum);
        }

        // else
        // 1. current slot is nullptr
        // 2. the uid doesn't match
        nChainNode = m_UIDArray[nChainNode].Loc.load().Loc1;
    }
    return -1;
}

int ActorPool::InnGet(uint32_t nUID)
{
    for(auto nChainNode = m_UIDArray[InnHashCode(nUID)].Loc.load().Loc0; nChainNode >= 0;){
        if(auto pActor = m_UIDArray[nChainNode].Actor.load(); pActor && pActor->UID() == nUID){
            return nChainNode;
        }

        // else
        // 1. current slot is nullptr
        // 2. the uid doesn't match
        nChainNode = m_UIDArray[nChainNode].Loc.load().Loc1;
    }
    return -1;
}

void ActorPool::LinkUID(uint32_t nUID, ServerObject *pObject)
{
}

void ActorPool::ExecuteChain(int nChainHead)
{
    for(auto nChainNode = m_UIDArray[nChainHead].Loc0.load(); nChainNode >= 0;){
        if(auto pActor = m_UIDArray[nChainNode].Actor.load()){
            pActor->
        }

        // else
        // 1. current slot is nullptr
        // 2. the uid doesn't match
        nChainNode = m_UIDArray[nChainNode].Loc.load().Loc1;
    }
}

void ActorPool::DetachUID(uint32_t nUID)
{
    if(auto nChainNode = InnGet(nUID); nChainNode >= 0){
        m_UIDArray[nChainNode].Actor.load()->Die();
    }
}

// QSBR update
// there is only one thread calling delete
// other thread can find/access the pod but can't remove it
void ActorPool::EraseChainDead(int nChainHead)
{
    for(auto nCurrNode = nChainHead, nLastNode = -1; nCurrNode >= 0;){
        if(auto pActor = m_UIDArray[nCurrNode].Actor.load(); pActor && pActor->Dead()){
            if(nLastNode >= 0){
                m_UIDArray[nLastNode].Loc1.store(m_UIDArray[nCurrNode].Loc1.load());
            }
            delete m_UIDArray[nCurr].Data.exchange(nullptr);
        }

        nLastNode = nCurrNode;
        nCurrNode = m_UIDArray[nCurrNode].Loc1.load();
    }
}

void ActorPool::Launch()
{
    for(int nIndex = 0; nIndex < m_ThreadCount; ++nIndex){
        m_Feature[nIndex] = std::async(std::launch::async, [nIndex, this]()
        {
            const int nChainPerThread = m_UIDArray.size() / m_ThreadCount;
            const int nChainHead0     = nIndex * nChainPerThread;
            const int nChainHead1     = std::min<int>((nIndex + 1) * nChainPerThread, m_UIDArray.size());

            while(m_Running.load()){
                for(auto nChainHead = nChainHead0; nChainHead < nChainHead1; ++nChainHead){
                    ExecuteChain(nChainHead);
                    EraseChainDead(nChainHead);
                }
            }
        });
    }
}
