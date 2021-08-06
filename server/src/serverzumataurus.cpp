#include "serverzumataurus.hpp"

corof::long_jmper ServerZumaTaurus::updateCoroFunc()
{
    uint64_t targetUID = 0;
    while(m_sdHealth.HP > 0){
        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            m_inViewCOList.erase(targetUID);
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            setStandMode(true);
            co_await coro_trackAttackUID(targetUID);
        }

        else if(masterUID()){
            setStandMode(true);
            co_await coro_followMaster();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

void ServerZumaTaurus::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        Monster::onAMAttack(mpk);
    }
}
