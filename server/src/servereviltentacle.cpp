#include "servereviltentacle.hpp"

corof::awaitable<> ServerEvilTentacle::runAICoro()
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
            const auto coLocOpt = co_await getCOLocation(targetUID);
            if(!coLocOpt.has_value()){
                continue;
            }

            const auto &coLoc = coLocOpt.value();
            if(inView(coLoc.mapUID, coLoc.x, coLoc.y)){
                if(mathf::CDistance<int>(coLoc.x, coLoc.y, X(), Y()) <= 3){
                    co_await trackAttackUID(targetUID);
                }
                else{
                    co_await jumpAttackUID(targetUID);
                }
            }
            else{
                targetUID = 0;
                m_inViewCOList.erase(targetUID);
            }
        }
        else{
            co_await randomMove();
        }

        co_await asyncIdleWait(1000);
    }
}
