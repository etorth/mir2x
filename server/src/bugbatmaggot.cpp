/*
 * =====================================================================================
 *
 *       Filename: bugbatmaggot.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "sysconst.hpp"
#include "bugbatmaggot.hpp"

void BugbatMaggot::addBat()
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

corof::long_jmper BugbatMaggot::updateCoroFunc()
{
    while(HP() > 0){
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
    co_return true;
}
