#include "zumamonster.hpp"

corof::long_jmper ZumaMonster::updateCoroFunc()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(m_sdHealth.HP > 0){
        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            m_inViewCOList.erase(targetUID);
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            idleTime.reset();
            setStandMode(true);
            co_await coro_trackAttackUID(targetUID);
        }

        else if(masterUID()){
            setStandMode(true);
            co_await coro_followMaster();
        }

        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }
        }
    }

    goDie();
    co_return true;
}

void ZumaMonster::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        Monster::onAMAttack(mpk);
    }
}
