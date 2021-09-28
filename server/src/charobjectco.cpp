/*
 * =====================================================================================
 *
 *       Filename: charobjectco.cpp
 *        Created: 04/10/2016 12:05:22
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

#include "corof.hpp"
#include "charobject.hpp"

corof::eval_poller::eval_awaiter<std::tuple<uint32_t, int, int>> CharObject::coro_getCOPLoc(uint64_t targetUID)
{
    const auto fnwait = +[](CharObject *p, uint64_t targetUID) -> corof::eval_poller
    {
        int x = -1;
        int y = -1;
        uint32_t mapID = 0;
        corof::async_variable<bool> done;

        p->getCOLocation(targetUID, [&x, &y, &mapID, &done](const COLocation &coLoc)
        {
            x = coLoc.x;
            y = coLoc.y;
            mapID = coLoc.mapID;
            done.assign(true);
        },

        [&done]
        {
            done.assign(false);
        });

        if(co_await done){
            co_return std::tuple<uint32_t, int, int>(mapID, x, y);
        }
        else{
            co_return std::tuple<uint32_t, int, int>(0, -1, -1);
        }
    };

    return fnwait(this, targetUID).to_awaiter<std::tuple<uint32_t, int, int>>();
}
