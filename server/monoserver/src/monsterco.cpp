/*
 * =====================================================================================
 *
 *       Filename: monsterco.cpp
 *        Created: 03/19/2019 06:43:21
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
#include "monster.hpp"
#include "monoserver.hpp"

corof::long_jmper<bool> Monster::coro_followMaster()
{
    corof::async_variable<bool> done;
    FollowMaster([&done](){ done.assign(true); }, [&done](){ done.assign(false); });

    if(done.wait()){
        co_await coro_wait(1200);
        co_return true;
    }

    co_await coro_wait(200);
    co_return false;
}

corof::long_jmper<bool> Monster::coro_wait(uint64_t ms)
{
    if(ms == 0){
        co_await cppcoro::suspend_always();
        co_return true;
    }

    hres_timer timer;
    while(timer.diff_msec() < ms){
        co_await cppcoro::suspend_always();
    }
    co_return true;
}

corof::long_jmper<bool> Monster::coro_randomMove()
{
    if(std::rand() % 10 < 2){
        if(RandomTurn()){
            co_await coro_wait(200);
        }
        else{
            co_await coro_wait(1000);
        }
    }

    else{
        if(coro_moveForward()){
            co_await coro_wait(1200);
        }
        else{
            co_await coro_wait(200);
        }
    }
    co_return true;
}

corof::long_jmper<bool> Monster::coro_moveForward()
{
    int nextX = -1;
    int nextY = -1;

    if(OneStepReach(Direction(), 1, &nextX, &nextY) != 1){
        co_return false;
    }

    corof::async_variable<bool> done;
    requestMove(nextX, nextY, MoveSpeed(), false, false, [&done](){ done.assign(true); }, [&done](){ done.assign(false); });
    co_return done.wait();
}

corof::long_jmper<uint64_t> Monster::coro_getProperTarget()
{
    corof::async_variable<uint64_t> targetUID;
    GetProperTarget([&targetUID](uint64_t uid){ targetUID.assign(uid); });
    co_return targetUID.wait();
}

corof::long_jmper<bool> Monster::coro_trackAttackUID(uint64_t targetUID)
{
    corof::async_variable<bool> done;
    TrackAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

    if(done.wait()){
        co_await coro_wait(1200);
        co_return true;
    }

    co_await coro_wait(200);
    co_return false;
}
