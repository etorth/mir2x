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

#include "pathf.hpp"
#include "corof.hpp"
#include "monster.hpp"
#include "monoserver.hpp"

corof::long_jmper::eval_op<bool> Monster::coro_followMaster()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->followMaster([&done](){ done.assign(true); }, [&done](){ done.assign(false); });

        if(co_await done){
            co_await corof::async_wait(p->getMR().walkWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(p->isPet(p->UID()) ? 20 : 200);
            co_return false;
        }
    };
    return fnwait(this).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_randomMove()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        if(std::rand() % 10 < 2){
            if(p->randomTurn()){
                co_await corof::async_wait(p->getMR().walkWait);
            }
            else{
                co_await corof::async_wait(200);
            }
        }

        else{
            if(co_await p->coro_moveForward()){
                co_await corof::async_wait(p->getMR().walkWait);
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
        const auto result = co_await done;
        co_return result;
    };
    return fnwait(this).eval<bool>();
}

corof::long_jmper::eval_op<uint64_t> Monster::coro_pickTarget()
{
    const auto fnwait = +[](Monster *p) -> corof::long_jmper
    {
        corof::async_variable<uint64_t> targetUID;
        p->pickTarget([&targetUID](uint64_t uid){ targetUID.assign(uid); });
        const auto result = co_await targetUID;
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

        if(co_await done){
            co_await corof::async_wait(p->getMR().attackWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(p->isPet(p->UID()) ? 20 : 200);
            co_return false;
        }
    };
    return fnwait(this, targetUID).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_trackUID(uint64_t targetUID, DCCastRange r)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID, DCCastRange r) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->trackUID(targetUID, r, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

        if(co_await done){
            co_return true;
        }
        else{
            co_return false;
        }
    };
    return fnwait(this, targetUID, r).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_attackUID(uint64_t targetUID, int dcType)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID, int dcType) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->attackUID(targetUID, dcType, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

        if(co_await done){
            co_await corof::async_wait(p->getMR().attackWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(p->isPet(p->UID()) ? 20 : 200);
            co_return false;
        }
    };
    return fnwait(this, targetUID, dcType).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_jumpAttackUID(uint64_t targetUID)
{
    const auto fnwait = +[](Monster *p, uint64_t targetUID) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->jumpAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

        if(co_await done){
            co_await corof::async_wait(p->getMR().attackWait);
            co_return true;
        }
        else{
            co_await corof::async_wait(p->isPet(p->UID()) ? 20 : 200);
            co_return false;
        }
    };
    return fnwait(this, targetUID).eval<bool>();
}

corof::long_jmper::eval_op<bool> Monster::coro_inDCCastRange(uint64_t targetUID, DCCastRange r)
{
    fflassert(targetUID);
    fflassert(r);

    const auto fnwait = +[](Monster *p, uint64_t targetUID, DCCastRange r) -> corof::long_jmper
    {
        corof::async_variable<bool> done;
        p->getCOLocation(targetUID, [p, r, &done](const COLocation &coLoc)
        {
            if(p->m_map->In(coLoc.mapID, coLoc.x, coLoc.y)){
                done.assign(pathf::inDCCastRange(r, p->X(), p->Y(), coLoc.x, coLoc.y));
            }
            else{
                done.assign(false);
            }
        },

        [&done]()
        {
            done.assign(false);
        });

        if(co_await done){
            co_return true;
        }
        else{
            co_return false;
        }
    };
    return fnwait(this, targetUID, r).eval<bool>();
}
