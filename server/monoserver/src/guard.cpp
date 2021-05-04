/*
 * =====================================================================================
 *
 *       Filename: guard.cpp
 *        Created: 04/26/2021 02:32:45
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

#include "guard.hpp"
#include "friendtype.hpp"

Guard::Guard(uint32_t monID, ServerMap *mapPtr, int argX, int argY, int argDir)
    : Monster(monID, mapPtr, argX, argY, argDir, 0)
    , m_standX(argX)
    , m_standY(argY)
    , m_standDirection(argDir)
{}

corof::long_jmper Guard::updateCoroFunc()
{
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_getProperTarget()){
            const auto [targetMapID, targetX, targetY] = co_await coro_getUIDPLoc(targetUID);
            if(mapID() == targetMapID && mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 1){
                co_await coro_attackUID(targetUID, DC_PHY_PLAIN);
            }
            else{
                co_await coro_jumpAttackUID(targetUID);
            }
        }
        else{
            co_await coro_jumpBack();
        }
    }

    goDie();
    co_return true;
}

void Guard::jumpBack(std::function<void()> onOK, std::function<void()> onError)
{
    if(X() == m_standX && Y() == m_standY){
        if(Direction() != m_standDirection){
            m_direction = m_standDirection;
            dispatchAction(makeActionStand());
        }

        if(onOK){
            onOK();
        }
        return;
    }

    requestJump(m_standX, m_standY, m_standDirection, onOK, onError);
}

void Guard::checkFriend(uint64_t uid, std::function<void(int)> fnOp)
{
    if(!uid){
        throw fflerror("invalid zero UID");
    }

    if(uid == UID()){
        throw fflerror("check friend type to self");
    }

    switch(uidf::getUIDType(uid)){
        case UID_MON:
            {
                if(DBCOM_MONSTERRECORD(uidf::getMonsterID(uid)).guard){
                    fnOp(FT_FRIEND);
                }
                else{
                    fnOp(FT_ENEMY);
                }
                return;
            }
        case UID_PLY:
            {
                fnOp(FT_NEUTRAL);
                return;
            }
        default:
            {
                fnOp(FT_FRIEND);
                return;
            }
    }
}
