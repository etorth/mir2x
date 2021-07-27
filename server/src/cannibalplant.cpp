/*
 * =====================================================================================
 *
 *       Filename: cannibalplant.cpp
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

#include <optional>
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "raiitimer.hpp"
#include "cannibalplant.hpp"

corof::long_jmper CannibalPlant::updateCoroFunc()
{
    std::optional<uint64_t> idleTime;
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_pickTarget()){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOPLoc(targetUID);
            if((mapID() == targetMapID) && (mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 1)){
                idleTime.reset();
                setStandMode(true);
                co_await coro_attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
            }
        }
        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }
            co_await corof::async_wait(200);
        }
    }

    goDie();
    co_return true;
}
