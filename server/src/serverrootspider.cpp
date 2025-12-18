#include "pathf.hpp"
#include "sysconst.hpp"
#include "serverrootspider.hpp"

corof::awaitable<> ServerRootSpider::addBombSpider()
{
    const auto [spawnGX, spawnGY] = pathf::getFrontGLoc(X(), Y(), Direction() + 3, 1);
    const auto uid = co_await addMonster(DBCOM_MONSTERID(u8"爆裂蜘蛛"), spawnGX, spawnGY, false);
    if(uid){
        m_childUIDList.insert(uid);
    }
}

corof::awaitable<> ServerRootSpider::runAICoro()
{
    uint64_t targetUID = 0;
    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            for(auto p = m_childUIDList.begin(); p != m_childUIDList.end();){
                if(m_actorPod->checkUIDValid(*p)){
                    p++;
                }
                else{
                    p = m_childUIDList.erase(p);
                }
            }

            if(m_childUIDList.size() < m_maxBatCount){
                dispatchAction(ActionAttack
                {
                    .x = X(),
                    .y = Y(),
                });

                co_await asyncWait(300);
                co_await addBombSpider();
            }
        }

        co_await asyncIdleWait(1000);
    }
}
