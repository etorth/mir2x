/*
 * =====================================================================================
 *
 *       Filename: batmother.cpp
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
#include "batmother.hpp"

void BatMother::addBat()
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

    m_actorPod->forward(m_map->UID(), {AM_ADDCHAROBJECT, amACO}, [](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_OK:
                {
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

corof::long_jmper BatMother::updateCoroFunc()
{
    while(HP() > 0){
        dispatchAction(ActionAttack
        {
            .x = X(),
            .y = Y(),
        });

        co_await corof::async_wait(600);
        addBat();

        co_await corof::async_wait(2000);
    }

    goDie();
    co_return true;
}
