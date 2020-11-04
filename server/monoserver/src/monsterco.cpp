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

corof::long_jmper::eval_op<bool> Monster::coro_followMaster()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->FollowMaster([&done](){ done.assign(true); }, [&done](){ done.assign(false); });

        if(co_await done.wait()){
            co_await corof::async_wait(1200);
            co_return true;
        }

        co_await corof::async_wait(200);
        co_return false;
    };

    return fnwait(this).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_randomMove()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        if(std::rand() % 10 < 2){
            if(p->RandomTurn()){
                co_await corof::async_wait(200);
            }
            else{
                co_await corof::async_wait(1000);
            }
        }

        else{
            if(co_await p->coro_moveForward()){
                co_await corof::async_wait(1200);
            }
            else{
                co_await corof::async_wait(200);
            }
        }
        co_return true;
    };

    return fnwait(this).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_moveForward()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        int nextX = -1;
        int nextY = -1;

        if(p->OneStepReach(p->Direction(), 1, &nextX, &nextY) != 1){
            co_return false;
        }

        corof::async_variable<bool> done;
        p->requestMove(nextX, nextY, p->MoveSpeed(), false, false, [&done](){ done.assign(true); }, [&done](){ done.assign(false); });
        const auto result = co_await done.wait();
        co_return result;
    };

    return fnwait(this).eval<bool>();
}

corof::long_jmper::eval_op<uint64_t> Monster::coro_getProperTarget()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        corof::async_variable<uint64_t> targetUID;
        p->GetProperTarget([&targetUID](uint64_t uid){ targetUID.assign(uid); });
        const auto result = co_await targetUID.wait();
        co_return result;
    };
    return fnwait(this).eval<uint64_t>();
}

corof::long_jmper::eval_op<bool> Monster::coro_trackAttackUID(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->trackAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

        if(co_await done.wait()){
            co_await corof::async_wait(1200);
            co_return true;
        }

        co_await corof::async_wait(200);
        co_return false;
    };

    return fnwait(this, targetUID).eval<bool>();
}
