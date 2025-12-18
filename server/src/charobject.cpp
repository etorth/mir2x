#include <cinttypes>
#include "uidf.hpp"
#include "totype.hpp"
#include "mathf.hpp"
#include "uidsf.hpp"
#include "mapbindb.hpp"
#include "fflerror.hpp"
#include "servermap.hpp"
#include "server.hpp"
#include "charobject.hpp"
#include "battleobject.hpp"
#include "scopedalloc.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"

extern MapBinDB *g_mapBinDB;
extern Server *g_server;

CharObject::LuaThreadRunner::LuaThreadRunner(CharObject *charObjectPtr)
    : ServerObject::LuaThreadRunner(charObjectPtr)
{
    bindFunction("getMap", [this]()
    {
        return getCO()->mapID();
    });

    bindFunction("getMapLoc", [this]()
    {
        return std::make_tuple(getCO()->X(), getCO()->Y());
    });

    pfrCheck(execRawString(BEGIN_LUAINC(char)
#include "charobject.lua"
    END_LUAINC()));
}

CharObject::CharObject(
        uint64_t uid,
        uint64_t argMapUID,
        int      mapX,
        int      mapY,
        int      direction)
    : ServerObject(uid)
    , m_mapUID(argMapUID)
    , m_mapBinPtr(g_mapBinDB->retrieve(mapID()))
    , m_X(mapX)
    , m_Y(mapY)
    , m_direction(direction)
{
    fflassert(mapBin()->validC(X(), Y()), X(), Y());
    fflassert(pathf::dirValid(Direction()), Direction()); // for NPC direction is gfxDir + DIR_BEGIN
}

corof::awaitable<std::optional<COLocation>> CharObject::getCOLocation(uint64_t uid)
{
    fflassert(uid);
    fflassert(uid != UID());

    // CO dispatches location changes automatically
    // always trust the InViewCOList, can even skip the expiration now

    if(auto p = getInViewCOPtr(uid)){
        co_return *p;
    }

    // can't find uid or expired
    // query the location and put to InViewCOList if applicable

    AMQueryLocation amQL;
    std::memset(&amQL, 0, sizeof(amQL));

    amQL.UID = UID();
    amQL.mapUID = mapUID();

    switch(const auto rmpk = co_await m_actorPod->send(uid, {AM_QUERYLOCATION, amQL}); rmpk.type()){
        case AM_LOCATION:
            {
                // TODO when we get this response
                // it's possible that the co has switched map or dead

                const auto amL = rmpk.conv<AMLocation>();
                const COLocation coLoc
                {
                    .uid       = amL.UID,
                    .mapUID    = amL.mapUID,
                    .x         = amL.X,
                    .y         = amL.Y,
                    .direction = amL.Direction
                };

                if(updateInViewCO(coLoc) > 0){
                    dispatchAction(coLoc.uid, makeActionStand());
                }

                co_return coLoc;
            }
        default:
            {
                m_inViewCOList.erase(uid);
                co_return std::nullopt;
            }
    }
}

bool CharObject::inView(uint64_t argMapUID, int argX, int argY) const
{
    if(argMapUID != mapUID()){
        return false;
    }

    if(!mapBin()->validC(argX, argY)){
        return false;
    }

    const auto r = [this]() -> int
    {
        if(isMonster()){
            if(const auto r = DBCOM_MONSTERRECORD(uidf::getMonsterID(UID()))){
                return r.view;
            }
        }
        return SYS_VIEWR;
    }();

    if(r <= 0){
        return false; // always blind
    }

    return mathf::LDistance2<int>(X(), Y(), argX, argY) <= r * r;
}

void CharObject::trimInViewCO()
{
    for(auto p = m_inViewCOList.begin(); p != m_inViewCOList.end();){
        if(inView(p->second.mapUID, p->second.x, p->second.y)){
            p++;
        }
        else{
            p = m_inViewCOList.erase(p);
        }
    }
}

void CharObject::foreachInViewCO(std::function<void(const COLocation &)> fnOnLoc)
{
    // updateInViewCO() may get called in fnOnLoc
    // it may change m_inViewCOList

    scoped_alloc::svobuf_wrapper<COLocation, 128> coLocList;
    for(const auto &[_, coLoc]: m_inViewCOList){
        coLocList.c.push_back(coLoc);
    }

    for(const auto &coLoc: coLocList.c){
        fnOnLoc(coLoc);
    }
}

bool CharObject::removeInViewCO(uint64_t uid)
{
    if(auto p = m_inViewCOList.find(uid); p != m_inViewCOList.end()){
        m_inViewCOList.erase(p);
        return true;
    }
    return false;
}

int CharObject::updateInViewCO(const COLocation &coLoc, bool forceDelete)
{
    const auto oldSize = to_d(m_inViewCOList.size());
    const auto oldDirection = [&coLoc, this]() -> int
    {
        if(const auto p = getInViewCOPtr(coLoc.uid)){
            return p->direction;
        }
        return DIR_NONE;
    }();

    if(!forceDelete && inView(coLoc.mapUID, coLoc.x, coLoc.y)){
        m_inViewCOList.insert_or_assign(coLoc.uid, COLocation
        {
            .uid    = coLoc.uid,
            .mapUID = coLoc.mapUID,

            .x = coLoc.x,
            .y = coLoc.y,

            // for CO mov: 1. send ACTION_MOVE to map, map then does boardcasts for the ACTION_MOVE
            //             2. send ACTION_STAND to all its cached neighbors

            // this makes the possibility:
            //
            //     1. neighbor firstly received ACTION_STAND, which has a valid direction
            //     2. neighbor then received ACTION_MOVE from map, which doesn't have an direction
            //
            // if 2) overwrites 1), neighbor has no valid direction for the moving CO
            // current solution is always reusing last direction if new coming action doesn't have one

            // if this is the first action, and without a valid direction
            // then we keep the cached direction as DIR_NONE
            .direction = pathf::dirValid(coLoc.direction) ? coLoc.direction : oldDirection,
        });
    }
    else{
        m_inViewCOList.erase(coLoc.uid);
    }
    return to_d(m_inViewCOList.size()) - oldSize;
}

ActionNode CharObject::makeActionStand() const
{
    ActionNode stand = ActionStand
    {
        .direction = Direction(),
        .x = X(),
        .y = Y(),
    };

    switch(uidf::getUIDType(UID())){
        case UID_MON:
            {
                break;
            }
        case UID_NPC:
            {
                // x: stand
                // 1: act
                // 2: actEx
                stand.extParam.stand.npc.act = 0;
                break;
            }
        default:
            {
                break;
            }
    }
    return stand;
}

void CharObject::dispatchAction(const ActionNode &action, bool forceMap)
{
    fflassert(hasActorPod());
    AMAction amA;
    std::memset(&amA, 0, sizeof(amA));

    amA.UID = UID();
    amA.mapUID = mapUID();
    amA.action = action;

    if(auto boPtr = dynamic_cast<BattleObject *>(this)){
        boPtr->setLastAction(amA.action.type);
    }

    if(forceMap){
        m_actorPod->post(mapUID(), {AM_ACTION, amA});
        return;
    }

    switch(amA.action.type){
        case ACTION_JUMP:
        case ACTION_MOVE:
        case ACTION_PUSHMOVE:
        case ACTION_SPACEMOVE:
        case ACTION_SPAWN:
            {
                m_actorPod->post(mapUID(), {AM_ACTION, amA});
                return;
            }
        default:
            {
                foreachInViewCO([this, amA](const COLocation &coLoc)
                {
                    if(inView(coLoc.mapUID, coLoc.x, coLoc.y)){
                        m_actorPod->post(coLoc.uid, {AM_ACTION, amA});
                    }
                });
                return;
            }
    }
}

void CharObject::dispatchAction(uint64_t uid, const ActionNode &action)
{
    fflassert(hasActorPod());
    AMAction amA;
    std::memset(&amA, 0, sizeof(amA));

    amA.UID = UID();
    amA.mapUID = mapUID();
    amA.action = action;
    m_actorPod->post(uid, {AM_ACTION, amA});
}
