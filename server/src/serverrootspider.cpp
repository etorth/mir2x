#include "pathf.hpp"
#include "sysconst.hpp"
#include "serverrootspider.hpp"

void ServerRootSpider::addBombSpider()
{
    const auto [spawnGX, spawnGY] = pathf::getFrontGLoc(X(), Y(), Direction() + 3, 1);

    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_MON;
    amACO.x = spawnGX;
    amACO.y = spawnGY;
    amACO.mapID = mapID();
    amACO.strictLoc = false;

    amACO.monster.monsterID = DBCOM_MONSTERID(u8"爆裂蜘蛛");
    amACO.monster.masterUID = 0;

    m_actorPod->forward(m_map->UID(), {AM_ADDCO, amACO}, [this](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_UID:
                {
                    if(const auto amUID = rmpk.conv<AMUID>(); amUID.UID){
                        m_batUIDList.insert(amUID.UID);
                    }
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

corof::eval_poller ServerRootSpider::updateCoroFunc()
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
            for(auto p = m_batUIDList.begin(); p != m_batUIDList.end();){
                if(m_actorPod->checkUIDValid(*p)){
                    p++;
                }
                else{
                    p = m_batUIDList.erase(p);
                }
            }

            if(m_batUIDList.size() < m_maxBatCount){
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
    co_return true;
}
