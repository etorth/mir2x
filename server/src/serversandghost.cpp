/*
 * =====================================================================================
 *
 *       Filename: serversandghost.cpp
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
#include "serversandghost.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::eval_poller ServerSandGhost::updateCoroFunc()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(m_sdHealth.HP > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOGLoc(targetUID);
            if(inView(targetMapID, targetX, targetY)){
                idleTime.reset();
                setStandMode(true);
                if(mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 1){
                    co_await coro_attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
                }
                else{
                    co_await coro_trackUID(targetUID, DBCOM_MAGICRECORD(u8"物理攻击").castRange);
                }
            }
            else{
                idleTime = hres_tstamp().to_sec();
                m_inViewCOList.erase(targetUID);
                targetUID = 0;
            }
        }
        else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
            if(m_standMode){
                co_await coro_randomMove();
            }
        }
        else if(!idleTime.has_value()){
            idleTime = hres_tstamp().to_sec();
        }
        else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
            setStandMode(false);
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}
