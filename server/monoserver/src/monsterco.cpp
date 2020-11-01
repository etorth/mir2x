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
#include "coro.hpp"
#include "monster.hpp"
#include "monoserver.hpp"

bool Monster::CoroNode_FollowMaster()
{
    coro_variable<bool> done;
    FollowMaster([&done](){ done.assign(true); }, [&done](){ done.assign(false); });

    if(done.wait()){
        CoroNode_Wait(1200);
        return true;
    }

    CoroNode_Wait(200);
    return false;
}

void Monster::CoroNode_Wait(uint64_t ms)
{
    if(ms == 0){
        coro_yield();
        return;
    }

    hres_timer timer;
    while(timer.diff_msec() < ms){
        coro_yield();
    }
}

void Monster::CoroNode_RandomMove()
{
    if(std::rand() % 10 < 2){
        if(RandomTurn()){
            CoroNode_Wait(200);
        }
        else{
            CoroNode_Wait(1000);
        }
    }

    else{
        if(CoroNode_MoveForward()){
            CoroNode_Wait(1200);
        }
        else{
            CoroNode_Wait(200);
        }
    }
}

bool Monster::CoroNode_MoveForward()
{
    int nextX = -1;
    int nextY = -1;

    if(OneStepReach(Direction(), 1, &nextX, &nextY) != 1){
        return false;
    }

    coro_variable<bool> done;
    requestMove(nextX, nextY, MoveSpeed(), false, false, [&done](){ done.assign(true); }, [&done](){ done.assign(false); });
    return done.wait();
}

uint64_t Monster::CoroNode_GetProperTarget()
{
    coro_variable<uint64_t> targetUID;
    GetProperTarget([&targetUID](uint64_t uid){ targetUID.assign(uid); });
    return targetUID.wait();
}

bool Monster::CoroNode_TrackAttackUID(uint64_t targetUID)
{
    coro_variable<bool> done;
    TrackAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

    if(done.wait()){
        CoroNode_Wait(1200);
        return true;
    }

    CoroNode_Wait(200);
    return false;
}
