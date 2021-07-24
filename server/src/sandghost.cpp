/*
 * =====================================================================================
 *
 *       Filename: sandghost.cpp
 *        Created: 07/24/2021 03:48:41 AM
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
#include "sandghost.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::long_jmper SandGhost::updateCoroFunc()
{
    std::optional<uint64_t> idleTime;
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_pickTarget()){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOPLoc(targetUID);
            if(mapID() == targetMapID){
                idleTime.reset();
                setStandMode(true);
                if(mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 1){
                    co_await coro_attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
                }
                else{
                    co_await coro_trackUID(targetUID, DBCOM_MAGICRECORD(u8"物理攻击").castRange);
                }
            }
            co_await corof::async_wait(200);
        }
        else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
            if(m_standMode){
                co_await coro_randomMove();
            }
            co_await corof::async_wait(200);
        }
        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_msec();
            }
            else if(hres_tstamp().to_msec() - idleTime.value() > 30ULL * 1000ULL /* n x 1 sec*/){
                setStandMode(false);
            }
            co_await corof::async_wait(200);
        }
    }

    goDie();
    co_return true;
}
