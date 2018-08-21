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

bool ActorPool::LinkUID(uint32_t nUID, ActorPod *pActor)
{
    if(nUID && pActor && (nUID == pActor->UID())){
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

void ActorPool::EraseUID(uint32_t nUID)
{
    auto &rstBucket = m_UIDArray[nUID % m_UIDArrayNum];
    {
        std::unique_lock<std::shared_mutex> stLock(rstBucket.Lock);
        rstBucket.erase(nUID);
    }
}

void ActorPool::PostMessage(uint32_t nUID, const MessagePack *pMPK, size_t nMPKLen)
{
    auto &rstBucket = m_UIDArray[nUID % m_UIDArrayNum];
    {
        std::shared_lock<std::shared_mutex> stLock(rstBucket.Lock);
        if(auto pActor = rstBucket.find(nUID); pActor != rstBucket.end()){
            pActor->PushMessage(pMPK, nMPKLen);
            return;
        }
    }

    for(size_t nIndex = 0; nIndex < nMPKLen; ++nIndex){
        PostMessage(pMPK[nIndex].From(), {MPK_BADACTORPOD, 0, pMPK[nIndex].Resp()});
    }
}

void ActorPool::Launch()
{
    for(int nIndex = 0; nIndex < m_UIDArrayNum; ++nIndex){
        m_Feature[nIndex] = std::async(std::launch::async, [nIndex, this]()
        {
            auto &rstBucket = m_UIDArray[nUID % m_UIDArrayNum];
            while(true){
                std::shared_lock<std::shared_mutex> stLock(rstBucket.Lock);
                {
                    for(auto pActor = rstBucket.List.begin(); pActor != rstBucket.List.end(); ++pActor){
                        pActor->second.Execute();
                    }
                }
            }
        });
    }
}
