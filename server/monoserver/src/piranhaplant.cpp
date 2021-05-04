/*
 * =====================================================================================
 *
 *       Filename: piranhaplant.cpp
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

#include "mathf.hpp"
#include "piranhaplant.hpp"

corof::long_jmper PiranhaPlant::updateCoroFunc()
{
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_pickTarget()){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOPLoc(targetUID);
            if(mapID() == targetMapID && mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 1){
                co_await coro_attackUID(targetUID, DC_PHY_PLAIN);
            }
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}
