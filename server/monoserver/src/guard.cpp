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

Guard::Guard(uint32_t monID, ServiceCore *corePtr, ServerMap *mapPtr, int argX, int argY, int argDir)
    : Monster(monID, corePtr, mapPtr, argX, argY, argDir, 0)
    , m_standX(argX)
    , m_standY(argY)
    , m_standDirection(argDir)
{}

corof::long_jmper Guard::updateCoroFunc()
{
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_getProperTarget()){
            co_await coro_jumpAttackUID(targetUID);
            co_await coro_jumpBack();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

void Guard::checkFriend(uint64_t uid, std::function<void(int)> fnOp)
{
    if(!uid){
        throw fflerror("invalid zero UID");
    }

    if(uid == UID()){
        throw fflerror("check friend type to self");
    }

    if(uidf::getUIDType(uid) == UID_MON){
        fnOp(FT_ENEMY);
    }
    else{
        fnOp(FT_NEUTRAL);
    }
}
