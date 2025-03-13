#include <cinttypes>
#include "uidf.hpp"
#include "totype.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "servermap.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "battleobject.hpp"
#include "scopedalloc.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"

extern MonoServer *g_monoServer;

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
        const ServerMap *mapCPtr,
        uint64_t         uid,
        int              mapX,
        int              mapY,
        int              direction)
    : ServerObject(uid)
    , m_map(mapCPtr)
    , m_X(mapX)
    , m_Y(mapY)
    , m_direction(direction)
{
    fflassert(m_map);
    fflassert(m_map->validC(X(), Y()), X(), Y());
    fflassert(pathf::dirValid(Direction()), Direction()); // for NPC direction is gfxDir + DIR_BEGIN
}

void CharObject::getCOLocation(uint64_t uid, std::function<void(const COLocation &)> onOK, std::function<void()> onError)
{
    fflassert(uid);
    fflassert(uid != UID());

    // CO dispatches location changes automatically
    // always trust the InViewCOList, can even skip the expiration now

    if(auto p = getInViewCOPtr(uid)){
        if(onOK){
            onOK(*p);
        }
        return;
    }

    // can't find uid or expired
    // query the location and put to InViewCOList if applicable

    AMQueryLocation amQL;
    std::memset(&amQL, 0, sizeof(amQL));

    amQL.UID = UID();
    amQL.mapID = mapID();

    m_actorPod->forward(uid, {AM_QUERYLOCATION, amQL}, [this, uid, onOK, onError](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_LOCATION:
                {
                    // TODO when we get this response
                    // it's possible that the co has switched map or dead

                    const auto amL = rmpk.conv<AMLocation>();
                    const COLocation coLoc
                    {
                        .uid       = amL.UID,
                        .mapID     = amL.mapID,
                        .x         = amL.X,
                        .y         = amL.Y,
                        .direction = amL.Direction
                    };

                    if(updateInViewCO(coLoc) > 0){
                        dispatchAction(coLoc.uid, makeActionStand());
                    }

                    if(onOK){
                        onOK(coLoc);
                    }
                    return;
                }
            default:
                {
                    m_inViewCOList.erase(uid);
                    if(onError){
                        onError();
                    }
                    return;
                }
        }
    });
}

void CharObject::addMonster(uint32_t monsterID, int x, int y, bool strictLoc)
{
    AMAddCharObject amACO;
    std::memset(&amACO, 0, sizeof(amACO));

    amACO.type = UID_MON;
    amACO.x = x;
    amACO.y = y;
    amACO.mapID = m_map->ID();
    amACO.strictLoc = strictLoc;

    amACO.monster.monsterID = monsterID;
    amACO.monster.masterUID = UID();

    m_actorPod->forward(uidf::getServiceCoreUID(), {AM_ADDCO, amACO}, [](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            default:
                {
                    break;
                }
        }
    });
}

bool CharObject::inView(uint32_t argMapID, int argX, int argY) const
{
    return m_map->in(argMapID, argX, argY) && mathf::LDistance2<int>(X(), Y(), argX, argY) <= SYS_VIEWR * SYS_VIEWR;
}

void CharObject::trimInViewCO()
{
    for(auto p = m_inViewCOList.begin(); p != m_inViewCOList.end();){
        if(inView(p->second.mapID, p->second.x, p->second.y)){
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
    for(const auto &[uid, coLoc]: m_inViewCOList){
        coLocList.c.push_back(coLoc);
    }

    for(const auto &coLoc: coLocList.c){
        fnOnLoc(coLoc);
    }
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

    if(!forceDelete && inView(coLoc.mapID, coLoc.x, coLoc.y)){
        m_inViewCOList.insert_or_assign(coLoc.uid, COLocation
        {
            .uid   = coLoc.uid,
            .mapID = coLoc.mapID,

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

void CharObject::notifyDead(uint64_t uid)
{
    fflassert(uid);
    fflassert(m_dead.get());

    AMNotifyDead amND;
    std::memset(&amND, 0, sizeof(amND));

    amND.UID = UID();
    m_actorPod->forward(uid, {AM_NOTIFYDEAD, amND});
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
    amA.mapID = mapID();
    amA.action = action;

    if(auto boPtr = dynamic_cast<BattleObject *>(this)){
        boPtr->setLastAction(amA.action.type);
    }

    if(forceMap){
        m_actorPod->forward(m_map->UID(), {AM_ACTION, amA});
        return;
    }

    switch(amA.action.type){
        case ACTION_JUMP:
        case ACTION_MOVE:
        case ACTION_PUSHMOVE:
        case ACTION_SPACEMOVE:
        case ACTION_SPAWN:
            {
                m_actorPod->forward(m_map->UID(), {AM_ACTION, amA});
                return;
            }
        default:
            {
                foreachInViewCO([this, amA](const COLocation &rstLocation)
                {
                    const auto nX = rstLocation.x;
                    const auto nY = rstLocation.y;
                    const auto nMapID = rstLocation.mapID;
                    if(inView(nMapID, nX, nY)){
                        m_actorPod->forward(rstLocation.uid, {AM_ACTION, amA});
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
    amA.mapID = mapID();
    amA.action = action;
    m_actorPod->forward(uid, {AM_ACTION, amA});
}
