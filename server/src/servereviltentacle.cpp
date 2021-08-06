/*
 * =====================================================================================
 *
 *       Filename: servereviltentacle.cpp
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

#include "servereviltentacle.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::long_jmper ServerEvilTentacle::updateCoroFunc()
{
    uint64_t targetUID = 0;
    while(m_sdHealth.HP > 0){
        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOPLoc(targetUID);
            if(inView(targetMapID, targetX, targetY)){
                if(mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 3){
                    co_await coro_trackAttackUID(targetUID);
                }
                else{
                    co_await coro_jumpAttackUID(targetUID);
                }
            }
            else{
                targetUID = 0;
                m_inViewCOList.erase(targetUID);
            }
        }
        else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
            co_await coro_randomMove();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}
