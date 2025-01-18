#include "uidf.hpp"
#include "pathf.hpp"
#include "corof.hpp"
#include "monster.hpp"
#include "monoserver.hpp"

corof::eval_poller<bool> Monster::coro_followMaster()
{
    corof::async_variable<bool> done;
    followMaster([&done](){ done.assign(true); }, [&done](){ done.assign(false); });

    if(co_await done){
        co_await corof::async_wait(getMR().walkWait);
        co_return true;
    }
    else{
        co_await corof::async_wait(20);
        co_return false;
    }
}

corof::eval_poller<bool> Monster::coro_randomMove()
{
    if(std::rand() % 10 < 2){
        if(randomTurn()){
            co_await corof::async_wait(getMR().walkWait);
            co_return true;
        }
    }
    else{
        if(co_await coro_moveForward()){
            co_await corof::async_wait(getMR().walkWait);
            co_return true;
        }
    }
    co_return false;
}

corof::eval_poller<bool> Monster::coro_moveForward()
{
    const auto reachRes = oneStepReach(Direction(), 1);
    if(!reachRes.has_value()){
        co_return false;
    }

    const auto [nextX, nextY, reachDist] = reachRes.value();
    if(reachDist != 1){
        co_return false;
    }

    corof::async_variable<bool> done;
    requestMove(nextX, nextY, moveSpeed(), false, false, [&done](){ done.assign(true); }, [&done](){ done.assign(false); });
    co_return (co_await done);
}

corof::eval_poller<bool> Monster::coro_needHeal(uint64_t uid)
{
    switch(uidf::getUIDType(uid)){
        case UID_MON:
        case UID_PLY:
            {
                const auto health = co_await coro_queryHealth(uid);
                if(health.has_value() && health.value().hp < health.value().maxHP){
                    co_return true;
                }
                break;
            }
        default:
            {
                break;
            }
    }
    co_return false;
}

corof::eval_poller<uint64_t> Monster::coro_pickTarget()
{
    corof::async_variable<uint64_t> targetUID;
    pickTarget([&targetUID](uint64_t uid){ targetUID.assign(uid); });
    co_return (co_await targetUID);
}

corof::eval_poller<uint64_t> Monster::coro_pickHealTarget()
{
    if(masterUID() && m_inViewCOList.count(masterUID()) && (co_await coro_needHeal(masterUID()))){
        co_return masterUID();
    }

    // coroutine bug, a reference before and after a coroutine switch can become dangling
    // following code is bad:
    //
    //     for(const auto &[uid, coLoc]: m_inViewCOList){
    //         if((uid != masterUID()) && (co_await coro_needHeal(uid))){
    //             co_return uid;
    //         }
    //     }
    //
    // in this code either the reference to [uid, coLoc] can be released because of co_await
    // or the m_inViewCOList itself can have been changed, which makes the range-based-loop to be invalid

    for(const auto uid: getInViewUIDList()){
        if((uid != masterUID()) && (co_await coro_needHeal(uid))){
            co_return uid;
        }
    }
    co_return 0;
}

corof::eval_poller<int> Monster::coro_checkFriend(uint64_t uid)
{
    corof::async_variable<int> friendType;
    checkFriend(uid, [&friendType](uint64_t type){ friendType.assign(type); });
    co_return (co_await friendType);
}

corof::eval_poller<bool> Monster::coro_trackAttackUID(uint64_t targetUID)
{
    corof::async_variable<bool> done;
    trackAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

    if(co_await done){
        co_await corof::async_wait(getMR().attackWait);
        co_return true;
    }
    else{
        co_await corof::async_wait(20);
        co_return false;
    }
}

corof::eval_poller<bool> Monster::coro_trackUID(uint64_t targetUID, DCCastRange r)
{
    corof::async_variable<bool> done;
    trackUID(targetUID, r, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
    co_return (co_await done);
}

corof::eval_poller<bool> Monster::coro_attackUID(uint64_t targetUID, int dcType)
{
    corof::async_variable<bool> done;
    attackUID(targetUID, dcType, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
    co_return (co_await done);
}

corof::eval_poller<bool> Monster::coro_jumpGLoc(int dstX, int dstY, int newDir)
{
    corof::async_variable<bool> done;
    requestJump(dstX, dstY, newDir, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
    co_return (co_await done);
}

corof::eval_poller<bool> Monster::coro_jumpUID(uint64_t targetUID)
{
    corof::async_variable<bool> done;
    jumpUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });
    co_return (co_await done);
}

corof::eval_poller<bool> Monster::coro_jumpAttackUID(uint64_t targetUID)
{
    corof::async_variable<bool> done;
    jumpAttackUID(targetUID, [&done]{ done.assign(true); }, [&done]{ done.assign(false); });

    if(co_await done){
        co_await corof::async_wait(getMR().attackWait);
        co_return true;
    }
    else{
        co_await corof::async_wait(20);
        co_return false;
    }
}

corof::eval_poller<bool> Monster::coro_inDCCastRange(uint64_t targetUID, DCCastRange r)
{
    fflassert(targetUID);
    fflassert(r);

    corof::async_variable<bool> done;
    getCOLocation(targetUID, [r, &done, this](const COLocation &coLoc)
    {
        if(m_map->in(coLoc.mapID, coLoc.x, coLoc.y)){
            done.assign(pathf::inDCCastRange(r, X(), Y(), coLoc.x, coLoc.y));
        }
        else{
            done.assign(false);
        }
    },

    [&done]()
    {
        done.assign(false);
    });

    co_return (co_await done);
}

corof::eval_poller<std::optional<SDHealth>> Monster::coro_queryHealth(uint64_t uid)
{
    corof::async_variable<std::optional<SDHealth>> health;
    queryHealth(uid, [uid, &health](uint64_t argUID, SDHealth argHealth)
    {
        if(argUID == uid){
            health.assign(std::move(argHealth));
        }
        else{
            health.assign({});
        }
    });
    co_return (co_await health);
}

corof::eval_poller<std::tuple<uint32_t, int, int>> Monster::coro_getCOGLoc(uint64_t targetUID)
{
    int x = -1;
    int y = -1;
    uint32_t mapID = 0;
    corof::async_variable<bool> done;

    getCOLocation(targetUID, [&x, &y, &mapID, &done](const COLocation &coLoc)
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
}

corof::eval_poller<bool> Monster::coro_validTarget(uint64_t targetUID)
{
    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
        case UID_PLY:
            {
                break;
            }
        default:
            {
                co_return false;
            }
    }

    if(!m_actorPod->checkUIDValid(targetUID)){
        co_return false;
    }

    const auto [locMapID, locX, locY] = co_await coro_getCOGLoc(targetUID);
    if(!inView(locMapID, locX, locY)){
        co_return false;
    }

    const auto viewDistance = getMR().view;
    if(viewDistance <= 0){
        co_return false;
    }

    if(mathf::LDistance2<int>(X(), Y(), locX, locY) > viewDistance * viewDistance){
        co_return false;
    }
    co_return true;
}
