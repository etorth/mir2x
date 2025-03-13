#include "sysconst.hpp"
#include "serverbugbatmaggot.hpp"

void ServerBugbatMaggot::addBat()
{
    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_MON;
    amACO.x = X();
    amACO.y = Y() - 1;
    amACO.mapID = mapID();
    amACO.strictLoc = false;

    amACO.monster.monsterID = DBCOM_MONSTERID(u8"蝙蝠");
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
