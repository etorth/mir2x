#include "serverzumamonster.hpp"

corof::awaitable<> ServerZumaMonster::runAICoro()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            idleTime.reset();
            setStandMode(true);
            co_await trackAttackUID(targetUID);
        }

        else if(masterUID()){
            setStandMode(true);
            co_await followMaster();
        }

        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }
        }

        co_await asyncIdleWait(1000);
    }
}

corof::awaitable<> ServerZumaMonster::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        co_await Monster::onAMAttack(mpk);
    }
}
