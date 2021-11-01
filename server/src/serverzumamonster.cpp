#include "serverzumamonster.hpp"

corof::eval_poller ServerZumaMonster::updateCoroFunc()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(m_sdHealth.HP > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
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
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

void ServerZumaMonster::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        Monster::onAMAttack(mpk);
    }
}
