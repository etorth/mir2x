#include "pathf.hpp"
#include "sysconst.hpp"
#include "serverrootspider.hpp"

void ServerRootSpider::addBombSpider()
{
    const auto [spawnGX, spawnGY] = pathf::getFrontGLoc(X(), Y(), Direction() + 3, 1);
    addMonster(DBCOM_MONSTERID(u8"爆裂蜘蛛"), spawnGX, spawnGY, false, [this](uint64_t uid)
    {
        if(uid){
            m_childUIDList.insert(uid);
        }
    });
}

corof::awaitable<> ServerRootSpider::runAICoro()
{
    uint64_t targetUID = 0;
    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
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

                co_await corof::async_wait(300);
                addBombSpider();
            }
        }
        co_await corof::async_wait(2000);
    }

    goDie();
}
