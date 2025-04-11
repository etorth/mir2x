#include "sysconst.hpp"
#include "serverbugbatmaggot.hpp"

void ServerBugbatMaggot::addBat()
{
    addMonster(DBCOM_MONSTERID(u8"蝙蝠"), X(), Y() - 1, false, [this](uint64_t uid)
    {
        if(uid){
            m_batUIDList.insert(uid);
        }
    });
}

corof::eval_poller<> ServerBugbatMaggot::updateCoroFunc()
{
    while(m_sdHealth.hp > 0){
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

            co_await corof::async_wait(600);
            addBat();
        }
        co_await corof::async_wait(2000);
    }

    goDie();
    co_return;
}
