#include "sysconst.hpp"
#include "serverbugbatmaggot.hpp"

corof::awaitable<> ServerBugbatMaggot::addBat()
{
    if(const auto uid = co_await addMonster(DBCOM_MONSTERID(u8"蝙蝠"), X(), Y() - 1, false)){
        m_batUIDList.insert(uid);
    }
}

corof::awaitable<> ServerBugbatMaggot::runAICoro()
{
    while(!m_sdHealth.dead()){
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

            co_await asyncWait(600);
            co_await addBat();
            continue;
        }

        co_await asyncIdleWait(1000);
    }
}
