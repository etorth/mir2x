/*
 * =====================================================================================
 *
 *       Filename: eviltentacle.cpp
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

#include "eviltentacle.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::long_jmper EvilTentacle::updateCoroFunc()
{
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_pickTarget()){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOPLoc(targetUID);
            if(mapID() == targetMapID){
                if(mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 3){
                    co_await coro_trackAttackUID(targetUID);
                }
                else{
                    co_await coro_jumpAttackUID(targetUID);
                }
            }
            else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
                co_await coro_randomMove();
            }
            else{
                co_await corof::async_wait(200);
            }
        }
        else{
            co_await corof::async_wait(200);
        }
    }

    goDie();
    co_return true;
}
