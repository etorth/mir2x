#include <memory>
#include <cinttypes>
#include "pathf.hpp"
#include "uidf.hpp"
#include "totype.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "mathf.hpp"
#include "servertaodog.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"
#include "buff.hpp"
#include "bufflist.hpp"

extern MonoServer *g_monoServer;

BattleObject::LuaThreadRunner::LuaThreadRunner(BattleObject *battleObjectPtr)
    : CharObject::LuaThreadRunner(battleObjectPtr)
{
    bindFunction("getHealth", [this]()
    {
        std::unordered_map<std::string, int> result;
        result["hp"] = getBO()->m_sdHealth.hp;
        result["mp"] = getBO()->m_sdHealth.mp;

        result["maxHP"] = getBO()->m_sdHealth.maxHP;
        result["maxMP"] = getBO()->m_sdHealth.maxMP;

        return sol::as_table(result);
    });
}

BattleObject::BOPathFinder::BOPathFinder(const BattleObject *boPtr, int checkCO)
    : pathf::AStarPathFinder(0, boPtr->maxStep(), [this](int srcX, int srcY, int srcDir, int dstX, int dstY) -> std::optional<double>
      {
          fflassert(pathf::hopValid(maxStep(), srcX, srcY, dstX, dstY), maxStep(), srcX, srcY, dstX, dstY);
          return m_BO->oneStepCost(this, m_checkCO, srcX, srcY, srcDir, dstX, dstY);
      })

    , m_BO(boPtr)
    , m_checkCO(checkCO)
{
    fflassert(m_BO);

    fflassert(m_checkCO >= 0, m_checkCO);
    fflassert(m_checkCO <= 2, m_checkCO);

    fflassert(m_BO->maxStep() >= 1, m_BO->maxStep());
    fflassert(m_BO->maxStep() <= 3, m_BO->maxStep());
}

int BattleObject::BOPathFinder::getGrid(int nX, int nY) const
{
    if(!m_BO->GetServerMap()->validC(nX, nY)){
        return PF_NONE;
    }

    const uint32_t nKey = (to_u32(nX) << 16) | to_u32(nY);
    if(auto p = m_cache.find(nKey); p != m_cache.end()){
        return p->second;
    }
    return (m_cache[nKey] = m_BO->checkPathGrid(nX, nY));
}

BattleObject::BattleObject(
        const ServerMap *mapCPtr,
        uint64_t         uid,
        int              mapX,
        int              mapY,
        int              direction)
    : CharObject(mapCPtr, uid, mapX, mapY, direction)
    , m_moveLock(false)
    , m_attackLock(false)
    , m_lastAction(ACTION_NONE)
{
    fflassert(m_map);
    m_lastActionTime.fill(0);
    m_stateTrigger.install([ptimer = std::make_shared<hres_timer>(), this]() mutable -> bool
    {
        if(hasActorPod() && (ptimer->diff_secf() >= 1.0)){
            updateHealth(m_sdHealth.getHPRecover(), m_sdHealth.getMPRecover());
            ptimer->reset();
        }
        return false;
    });
}

bool BattleObject::requestJump(int nX, int nY, int nDirection, std::function<void()> onOK, std::function<void()> onError)
{
    if(!m_map->groundValid(nX, nY)){
        throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_lld(mapID()), nX, nY);
    }

    if(X() == nX && Y() == nY){
        if(onError){
            onError();
        }
        return false;
    }

    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    AMTryJump amTJ;
    std::memset(&amTJ, 0, sizeof(amTJ));

    amTJ.X    = X();
    amTJ.Y    = Y();
    amTJ.EndX = nX;
    amTJ.EndY = nY;

    m_moveLock = true;
    return m_actorPod->forward(mapUID(), {AM_TRYJUMP, amTJ}, [this, nX, nY, nDirection, onOK, onError](const ActorMsgPack &rmpk)
    {
        fflassert(m_moveLock);
        m_moveLock = false;

        // handle jump, CO may be dead
        // need to check if current CO can jump

        switch(rmpk.type()){
            case AM_ALLOWJUMP:
                {
                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), AM_JUMPERROR, rmpk.seqID());
                        if(onError){
                            onError();
                        }
                        return;
                    }

                    const auto oldX = m_X;
                    const auto oldY = m_Y;
                    const auto amAJ = rmpk.conv<AMAllowJump>();

                    m_X = amAJ.EndX;
                    m_Y = amAJ.EndY;

                    if(pathf::dirValid(nDirection)){
                        m_direction = nDirection;
                    }
                    else{
                        m_direction = pathf::getOffDir(oldX, oldY, X(), Y());
                    }

                    AMJumpOK amJOK;
                    std::memset(&amJOK, 0, sizeof(amJOK));

                    amJOK.uid = UID();
                    amJOK.mapID = mapID();
                    amJOK.action = ActionJump
                    {
                        .direction = Direction(),
                        .x = X(),
                        .y = Y(),
                    };

                    m_actorPod->forward(rmpk.from(), {AM_JUMPOK, amJOK}, rmpk.seqID());
                    trimInViewCO();

                    if(isPlayer()){
                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                    }

                    m_buffList.runOnTrigger(BATGR_MOVE);
                    m_buffList.runOnBOMove();
                    m_buffList.dispatchAura();

                    if(onOK){
                        onOK();
                    }
                    return;
                }
            default:
                {
                    if(onError){
                        onError();
                    }
                    return;
                }
        }
    });
}

bool BattleObject::requestMove(int dstX, int dstY, int speed, bool allowHalfMove, bool removeMonster, std::function<void()> onOK, std::function<void()> onError)
{
    if(!m_map->groundValid(dstX, dstY)){
        throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_lld(mapID()), dstX, dstY);
    }

    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    if(estimateHop(dstX, dstY) != 1){
        if(onError){
            onError();
        }
        return false;
    }

    if(removeMonster){
        throw fflerror("RemoveMonster in requestMove() not implemented yet");
    }

    switch(mathf::LDistance2(X(), Y(), dstX, dstY)){
        case 1:
        case 2:
            {
                switch(checkPathGrid(dstX, dstY)){
                    case PF_FREE:
                        {
                            break;
                        }
                    case PF_OCCUPIED:
                    default:
                        {
                            if(onError){
                                onError();
                            }
                            return false;
                        }
                }
                break;
            }
        default:
            {
                // one-hop distance but has internal steps
                // need to check if there is co blocking the path
                const auto [nXm, nYm] = pathf::getFrontGLoc(X(), Y(), pathf::getOffDir(X(), Y(), dstX, dstY), 1);

                // for strict co check
                // need to skip the current (X(), Y())

                if(!oneStepCost(nullptr, 2, nXm, nYm, Direction(), dstX, dstY).has_value()){
                    if(!allowHalfMove){
                        if(onError){
                            onError();
                        }
                        return false;
                    }
                }
                break;
            }
    }

    AMTryMove amTM;
    std::memset(&amTM, 0, sizeof(amTM));

    amTM.UID           = UID();
    amTM.mapID         = mapID();
    amTM.X             = X();
    amTM.Y             = Y();
    amTM.EndX          = dstX;
    amTM.EndY          = dstY;
    amTM.AllowHalfMove = allowHalfMove;
    amTM.RemoveMonster = removeMonster;

    m_moveLock = true;
    return m_actorPod->forward(mapUID(), {AM_TRYMOVE, amTM}, [this, dstX, dstY, speed, onOK, onError](const ActorMsgPack &rmpk)
    {
        fflassert(m_moveLock);
        m_moveLock = false;

        // handle move, CO may be dead
        // need to check if current CO can move

        switch(rmpk.type()){
            case AM_ALLOWMOVE:
                {
                    // since we may allow half move
                    // servermap permitted dst may not be (dstX, dstY)

                    const auto amAM = rmpk.conv<AMAllowMove>();
                    fflassert(m_map->validC(amAM.EndX, amAM.EndY));

                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), AM_MOVEERROR, rmpk.seqID());
                        if(onError){
                            onError();
                        }
                        return;
                    }

                    const auto oldX = m_X;
                    const auto oldY = m_Y;

                    m_X = amAM.EndX;
                    m_Y = amAM.EndY;

                    m_direction = pathf::getOffDir(oldX, oldY, X(), Y());

                    AMMoveOK amMOK;
                    std::memset(&amMOK, 0, sizeof(amMOK));
                    amMOK.uid = UID();
                    amMOK.mapID = mapID();
                    amMOK.action = ActionMove
                    {
                        .speed = speed,
                        .x = oldX,
                        .y = oldY,
                        .aimX = X(),
                        .aimY = Y(),
                    };

                    m_actorPod->forward(rmpk.from(), {AM_MOVEOK, amMOK}, rmpk.seqID());
                    trimInViewCO();

                    // here firstly we make map to boardcast the ActionMove
                    // then if BO is a player, notify all its slaves with ActionStand

                    // however the order of these two actions reaches neighbor can switch
                    // from neighbor's view it firstly get an ActionStand but location changed, then get an ActionMove but destination is current location

                    if(isPlayer()){
                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                    }

                    m_buffList.runOnTrigger(BATGR_MOVE);
                    m_buffList.runOnBOMove();
                    m_buffList.dispatchAura();

                    if(onOK){
                        onOK();
                    }
                    return;
                }
            default:
                {
                    if(onError){
                        onError();
                    }
                    return;
                }
        }
    });
}

bool BattleObject::requestSpaceMove(int locX, int locY, bool strictMove, std::function<void()> onOK, std::function<void()> onError)
{
    if(strictMove){
        if(!m_map->groundValid(locX, locY)){
            throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_llu(mapID()), locX, locY);
        }
    }
    else{
        if(!m_map->validC(locX, locY)){
            throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_llu(mapID()), locX, locY);
        }
    }

    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    AMTrySpaceMove amTSM;
    std::memset(&amTSM, 0, sizeof(amTSM));

    amTSM.X = X();
    amTSM.Y = Y();
    amTSM.EndX = locX;
    amTSM.EndY = locY;
    amTSM.StrictMove = strictMove;

    m_moveLock = true;
    return m_actorPod->forward(mapUID(), {AM_TRYSPACEMOVE, amTSM}, [this, onOK, onError](const ActorMsgPack &rmpk)
    {
        fflassert(m_moveLock);
        m_moveLock = false;

        // handle move, CO can be dead already
        // check if current CO can move even we checked before

        switch(rmpk.type()){
            case AM_ALLOWSPACEMOVE:
                {
                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), AM_SPACEMOVEERROR, rmpk.seqID());
                        if(onError){
                            onError();
                        }
                        return;
                    }

                    // setup new map
                    // don't use the requested location
                    const auto amASM = rmpk.conv<AMAllowSpaceMove>();

                    const auto oldX = m_X;
                    const auto oldY = m_Y;

                    m_X = amASM.EndX;
                    m_Y = amASM.EndY;

                    AMSpaceMoveOK amSMOK;
                    std::memset(&amSMOK, 0, sizeof(amSMOK));

                    amSMOK.uid = UID();
                    amSMOK.mapID = mapID();
                    amSMOK.action = ActionSpaceMove
                    {
                        .direction = Direction(),
                        .x = oldX,
                        .y = oldY,
                        .aimX = X(),
                        .aimY = Y(),
                    };

                    m_actorPod->forward(rmpk.from(), {AM_SPACEMOVEOK, amSMOK}, rmpk.seqID());
                    trimInViewCO();

                    if(isPlayer()){
                        dynamic_cast<Player *>(this)->reportAction(UID(), mapID(), amSMOK.action);
                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                    }

                    m_buffList.runOnTrigger(BATGR_MOVE);
                    m_buffList.runOnBOMove();
                    m_buffList.dispatchAura();

                    if(onOK){
                        onOK();
                    }
                    break;
                }
            default:
                {
                    if(onError){
                        onError();
                    }
                    break;
                }
        }
    });
}

bool BattleObject::requestMapSwitch(uint32_t argMapID, int locX, int locY, bool strictMove, std::function<void()> onOK, std::function<void()> onError)
{
    if(argMapID == mapID()){
        throw fflerror("request to switch on same map: mapID = %llu", to_llu(argMapID));
    }

    if(locX < 0 || locY < 0){
        throw fflerror("invalid argument: mapID = %llu, locX = %d, locY = %d", to_llu(argMapID), locX, locY);
    }

    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    AMLoadMap amLM;
    std::memset(&amLM, 0, sizeof(amLM));

    amLM.mapID = argMapID;
    return m_actorPod->forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}, [argMapID, locX, locY, strictMove, onOK, onError, this](const ActorMsgPack &mpk)
    {
        switch(mpk.type()){
            case AM_LOADMAPOK:
                {
                    AMTryMapSwitch amTMS;
                    std::memset(&amTMS, 0, sizeof(amTMS));

                    amTMS.X = locX;
                    amTMS.Y = locY;
                    amTMS.strictMove = strictMove;

                    // send request to the new map
                    // if request rejected then it stays in current map

                    m_moveLock = true;
                    m_actorPod->forward(uidf::getMapBaseUID(argMapID), {AM_TRYMAPSWITCH, amTMS}, [mpk, onOK, onError, this](const ActorMsgPack &rmpk)
                    {
                        fflassert(m_moveLock);
                        m_moveLock = false;

                        switch(rmpk.type()){
                            case AM_ALLOWMAPSWITCH:
                                {
                                    // new map accepts this switch request
                                    // new map will guarantee to outlive current object

                                    if(!canMove()){
                                        m_actorPod->forward(rmpk.from(), AM_MAPSWITCHERROR, rmpk.seqID());
                                        if(onError){
                                            onError();
                                        }
                                        return;
                                    }

                                    const auto amAMS = rmpk.conv<AMAllowMapSwitch>();
                                    const auto newMapPtr = static_cast<const ServerMap *>(amAMS.Ptr);

                                    if(!(true && newMapPtr
                                              && newMapPtr->ID()
                                              && newMapPtr->UID()
                                              && newMapPtr->validC(amAMS.X, amAMS.Y))){

                                        // fake map
                                        // invalid argument, this is not good place to call onError()

                                        m_actorPod->forward(rmpk.from(), AM_MAPSWITCHERROR, rmpk.seqID());
                                        if(onError){
                                            onError();
                                        }
                                        return;
                                    }

                                    AMTryLeave amTL;
                                    std::memset(&amTL, 0, sizeof(amTL));

                                    amTL.X = X();
                                    amTL.Y = Y();

                                    // current map respond for the leave request
                                    // dangerous here, we should keep m_map always valid

                                    m_moveLock = true;
                                    m_actorPod->forward(m_map->UID(), {AM_TRYLEAVE, amTL}, [this, rmpk, onOK, onError](const ActorMsgPack &leavermpk)
                                    {
                                        fflassert(m_moveLock);
                                        m_moveLock = false;

                                        const auto amAMS = rmpk.conv<AMAllowMapSwitch>();
                                        const auto newMapPtr = static_cast<const ServerMap *>(amAMS.Ptr);

                                        switch(leavermpk.type()){
                                            case AM_ALLOWLEAVE:
                                                {
                                                    if(!canMove()){
                                                        m_actorPod->forward(rmpk.from(), AM_MAPSWITCHERROR, rmpk.seqID());
                                                        m_actorPod->forward(leavermpk.from(), AM_LEAVEERROR, leavermpk.seqID());
                                                        if(onError){
                                                            onError();
                                                        }
                                                        return;
                                                    }

                                                    m_map = newMapPtr;
                                                    m_X = amAMS.X;
                                                    m_Y = amAMS.Y;

                                                    AMLeaveOK amLOK;
                                                    std::memset(&amLOK, 0, sizeof(amLOK));

                                                    amLOK.uid = UID();
                                                    amLOK.mapID = mapID();
                                                    amLOK.action = makeActionStand();

                                                    m_moveLock = true;
                                                    m_actorPod->forward(leavermpk.from(), {AM_LEAVEOK, amLOK}, leavermpk.seqID(), [rmpk, onOK, this](const ActorMsgPack &finishrmpk)
                                                    {
                                                        fflassert(m_moveLock);
                                                        m_moveLock = false;

                                                        switch(finishrmpk.type()){
                                                            case AM_FINISHLEAVE:
                                                            default:
                                                                {
                                                                    // no matter what returned form server map, we always call onOK
                                                                    // because the map switch has already been done

                                                                    AMMapSwitchOK amMSOK;
                                                                    std::memset(&amMSOK, 0, sizeof(amMSOK));

                                                                    amMSOK.uid = UID();
                                                                    amMSOK.mapID = mapID();
                                                                    amMSOK.action = makeActionStand();
                                                                    m_actorPod->forward(m_map->UID(), {AM_MAPSWITCHOK, amMSOK}, rmpk.seqID());

                                                                    trimInViewCO();
                                                                    if(isPlayer()){
                                                                        dynamic_cast<Player *>(this)->reportStand();
                                                                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                                                                    }

                                                                    m_buffList.runOnTrigger(BATGR_MOVE);
                                                                    m_buffList.runOnBOMove();
                                                                    m_buffList.dispatchAura();

                                                                    if(onOK){
                                                                        onOK();
                                                                    }
                                                                    return;
                                                                }
                                                        }
                                                    });
                                                    return;
                                                }
                                            default:
                                                {
                                                    m_actorPod->forward(newMapPtr->UID(), AM_MAPSWITCHERROR, rmpk.seqID());
                                                    if(onError){
                                                        onError();
                                                    }
                                                    return;
                                                }
                                        }
                                    });
                                    return;
                                }
                            default:
                                {
                                    // do nothing
                                    // new map reject this switch request
                                    if(onError){
                                        onError();
                                    }
                                    return;
                                }
                        }
                    });
                    return;
                }
            default:
                {
                    if(onError){
                        onError();
                    }
                    return;
                }
        }
    });
}

bool BattleObject::canAct() const
{
    if(m_dead.get()){
        return false;
    }

    switch(m_lastAction){
        case ACTION_SPAWN:
            {
                switch(uidf::getUIDType(UID())){
                    case UID_MON:
                        {
                            switch(uidf::getMonsterID(UID())){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        return g_monoServer->getCurrTick() > m_lastActionTime.at(ACTION_SPAWN) + 600;
                                    }
                                case DBCOM_MONSTERID(u8"神兽"):
                                    {
                                        return g_monoServer->getCurrTick() > m_lastActionTime.at(ACTION_SPAWN) + 400;
                                    }
                                case DBCOM_MONSTERID(u8"食人花"):
                                    {
                                        return g_monoServer->getCurrTick() > m_lastActionTime.at(ACTION_SPAWN) + 400;
                                    }
                                default:
                                    {
                                        return true;
                                    }
                            }
                        }
                    default:
                        {
                            return true;
                        }
                }
                return true;
            }
        default:
            {
                break;
            }
    }
    return true;
}

bool BattleObject::canMove() const
{
    return canAct() && !m_moveLock;
}

bool BattleObject::canAttack() const
{
    return canAct() && !m_attackLock;
}

std::optional<std::tuple<int, int, int>> BattleObject::oneStepReach(int dir, int maxDistance) const
{
    fflassert(pathf::dirValid(dir));
    fflassert(maxDistance >= 1);

    std::optional<std::tuple<int, int, int>> result;
    for(int d = 1; d <= maxDistance; ++d){
        const auto [dstX, dstY] = pathf::getFrontGLoc(X(), Y(), dir, d);
        if(!m_map->groundValid(dstX, dstY)){
            break;
        }
        result = std::make_tuple(dstX, dstY, d);
    }
    return result;
}

void BattleObject::dispatchHealth()
{
    dispatchInViewCONetPackage(SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void BattleObject::dispatchHealth(uint64_t uid)
{
    fflassert(uidf::isPlayer(uid));
    forwardNetPackage(uid, SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void BattleObject::dispatchAttackDamage(uint64_t nUID, int nDC, int modifierID)
{
    if(nUID && dcValid(nDC, true)){
        AMAttack amA;
        std::memset(&amA, 0, sizeof(amA));

        amA.UID   = UID();
        amA.mapID = mapID();

        amA.X = X();
        amA.Y = Y();

        amA.damage = getAttackDamage(nDC, modifierID);
        m_actorPod->forward(nUID, {AM_ATTACK, amA});
    }
}

int BattleObject::Speed(int nSpeedType) const
{
    switch(nSpeedType){
        case SPEED_MOVE:
            {
                return SYS_DEFSPEED;
            }
        case SPEED_ATTACK:
            {
                return SYS_DEFSPEED;
            }
        default:
            {
                return -1;
            }
    }
}

void BattleObject::addMonster(uint32_t monsterID, int x, int y, bool strictLoc)
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

    m_actorPod->forward(uidf::getServiceCoreUID(), {AM_ADDCO, amACO}, [](const ActorMsgPack &rstRMPK)
    {
        switch(rstRMPK.type()){
            default:
                {
                    break;
                }
        }
    });
}

int BattleObject::estimateHop(int nX, int nY)
{
    fflassert(m_map);
    if(!m_map->validC(nX, nY)){
        return -1;
    }

    switch(const auto distance2 = mathf::LDistance2<int>(nX, nY, X(), Y())){
        case 0:
            {
                return 0;
            }
        case 1:
        case 2:
            {
                return 1;
            }
        default:
            {
                fflassert(maxStep() >= 1, maxStep());
                fflassert(maxStep() <= 3, maxStep());

                if(false
                        || distance2 == 1 * maxStep() * maxStep()
                        || distance2 == 2 * maxStep() * maxStep()){

                    const auto nDir = pathf::getOffDir(X(), Y(), nX, nY);
                    const auto nMaxReach = oneStepReach(nDir, maxStep());

                    if(nMaxReach.has_value() && std::get<2>(nMaxReach.value()) == maxStep()){
                        return 1;
                    }
                }
                return 2;
            }
    }
}

int BattleObject::checkPathGrid(int argX, int argY) const
{
    fflassert(m_map);
    if(!m_map->getMapData().validC(argX, argY)){
        return PF_NONE;
    }

    if(!m_map->getMapData().cell(argX, argY).land.canThrough()){
        return PF_OBSTACLE;
    }

    if(X() == argX && Y() == argY){
        return PF_OCCUPIED;
    }

    for(const auto &[uid, coLoc]: m_inViewCOList){
        if(coLoc.x == argX && coLoc.y == argY){
            return PF_OCCUPIED;
        }
    }
    return PF_FREE;
}

std::array<pathf::PathNode, 3> BattleObject::getChaseGrid(int nX, int nY, int nDLen) const
{
    // always get the next step to chase
    // this function won't check if (nX, nY) is valid

    const int nX0 = X();
    const int nY0 = Y();

    std::array<pathf::PathNode, 3> pathNodeList
    {{
        {-1, -1},
        {-1, -1},
        {-1, -1},
    }};

    const int nDX = ((nX > nX0) - (nX < nX0));
    const int nDY = ((nY > nY0) - (nY < nY0));

    switch(std::abs(nDX) + std::abs(nDY)){
        case 1:
            {
                if(nDY){
                    pathNodeList[0] = {nX0        , nY0 + nDY * nDLen};
                    pathNodeList[1] = {nX0 - nDLen, nY0 + nDY * nDLen};
                    pathNodeList[2] = {nX0 + nDLen, nY0 + nDY * nDLen};
                }
                else{
                    pathNodeList[0] = {nX0 + nDX * nDLen, nY0        };
                    pathNodeList[1] = {nX0 + nDX * nDLen, nY0 - nDLen};
                    pathNodeList[2] = {nX0 + nDX * nDLen, nY0 + nDLen};
                }
                break;
            }
        case 2:
            {
                pathNodeList[0] = {nX0 + nDX * nDLen, nY0 + nDY * nDLen};
                pathNodeList[1] = {nX0              , nY0 + nDY * nDLen};
                pathNodeList[2] = {nX0 + nDX * nDLen, nY0              };
                break;
            }
        default:
            {
                break;
            }
    }
    return pathNodeList;
}

std::vector<pathf::PathNode> BattleObject::getValidChaseGrid(int nX, int nY, int nDLen) const
{
    std::vector<pathf::PathNode> result;
    for(const auto &node: getChaseGrid(nX, nY, nDLen)){
        if(m_map->groundValid(node.X, node.Y)){
            result.push_back(node);
        }
    }
    return result;
}

void BattleObject::getValidChaseGrid(int nX, int nY, int nDLen, scoped_alloc::svobuf_wrapper<pathf::PathNode, 3> &buf) const
{
    buf.c.clear();
    for(const auto &node: getChaseGrid(nX, nY, nDLen)){
        if(m_map->groundValid(node.X, node.Y)){
            buf.c.push_back(node);
        }
    }

    if(buf.c.size() > buf.svocap()){
        throw fflerror("more than 3 valid chase grid found");
    }
}

std::optional<double> BattleObject::oneStepCost(const BattleObject::BOPathFinder *finder, int checkCO, int srcX, int srcY, int srcDir, int dstX, int dstY) const
{
    fflassert(checkCO >= 0, checkCO);
    fflassert(checkCO <= 2, checkCO);

    int hopSize = -1;
    switch(mathf::LDistance2(srcX, srcY, dstX, dstY)){
        case  1:
        case  2: hopSize = 1; break;
        case  4:
        case  8: hopSize = 2; break;
        case  9:
        case 18: hopSize = 3; break;
        case  0: return .0;
        default: return {};
    }

    double gridExtraPen = 0.00;
    const auto hopDir = pathf::getOffDir(srcX, srcY, dstX, dstY);

    for(int stepSize = 1; stepSize <= hopSize; ++stepSize){
        const auto [currX, currY] = pathf::getFrontGLoc(srcX, srcY, hopDir, stepSize);
        switch(const auto pfGrid = finder ? finder->getGrid(currX, currY) : checkPathGrid(currX, currY)){
            case PF_FREE:
                {
                    break;
                }
            case PF_OCCUPIED:
                {
                    switch(checkCO){
                        case 0:
                            {
                                break;
                            }
                        case 1:
                            {
                                gridExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return {};
                            }
                        default:
                            {
                                throw fflvalue(checkCO);
                            }
                    }
                    break;
                }
            case PF_NONE:
            case PF_OBSTACLE:
                {
                    return {};
                }
            default:
                {
                    throw fflvalue(currX, currY, pfGrid);
                }
        }
    }

    return 1.00 + hopSize * 0.10 + gridExtraPen + pathf::getDirAbsDiff(srcDir, hopDir) * 0.01;
}

void BattleObject::setLastAction(int type)
{
    m_lastAction = type;
    m_lastActionTime.at(type) = g_monoServer->getCurrTick();
}

bool BattleObject::isOffender(uint64_t nUID)
{
    for(auto &offender: m_offenderList){
        if(offender.uid == nUID){
            return true;
        }
    }
    return false;
}

void BattleObject::queryHealth(uint64_t uid, std::function<void(uint64_t, SDHealth)> fnOp)
{
    fflassert(uid);
    m_actorPod->forward(uid, AM_QUERYHEALTH, [uid, fnOp = std::move(fnOp)](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_HEALTH:
                {
                    fnOp(uid, rmpk.deserialize<SDHealth>());
                    break;
                }
            default:
                {
                    fnOp(0, {});
                    break;
                }
        }
    });
}

void BattleObject::queryFinalMaster(uint64_t targetUID, std::function<void(uint64_t)> fnOp)
{
    fflassert(targetUID);
    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
            {
                // here don't use mr.tameable
                // monsters not tameable means wizard can't tame it
                // but can be created by GM command, or buy from monster merchant

                const auto fnQuery = [this, fnOp](uint64_t targetUID)
                {
                    m_actorPod->forward(targetUID, AM_QUERYFINALMASTER, [this, targetUID, fnOp](const ActorMsgPack &rmpk)
                    {
                        switch(rmpk.type()){
                            case AM_UID:
                                {
                                    const auto amUID = rmpk.conv<AMUID>();
                                    if(fnOp){
                                        fnOp(amUID.UID);
                                    }
                                    return;
                                }
                            default:
                                {
                                    if(fnOp){
                                        fnOp(0);
                                    }

                                    if(isMonster()){
                                        if(targetUID == dynamic_cast<Monster *>(this)->masterUID()){
                                            goDie();
                                        }
                                    }
                                    return;
                                }
                        }
                    });
                };

                if(targetUID == UID()){
                    if(const auto masterUID = dynamic_cast<Monster *>(this)->masterUID()){
                        switch(uidf::getUIDType(masterUID)){
                            case UID_PLY:
                                {
                                    if(fnOp){
                                        fnOp(masterUID);
                                    }
                                    return;
                                }
                            case UID_MON:
                                {
                                    fnQuery(masterUID);
                                    return;
                                }
                            default:
                                {
                                    throw fflreach();
                                }
                        }
                    }
                    else{
                        if(fnOp){
                            fnOp(UID());
                        }
                        return;
                    }
                }
                else{
                    fnQuery(targetUID);
                    return;
                }
            }
        case UID_PLY:
        case UID_NPC:
            {
                if(fnOp){
                    fnOp(targetUID);
                }
                return;
            }
        default:
            {
                throw fflerror("invalid uid: %s", to_cstr(uidf::getUIDString(targetUID)));
            }
    }
}

void BattleObject::dispatchBuffIDList()
{
    const auto sdBuf = cerealf::serialize(SDBuffIDList
    {
        .uid = UID(),
        .idList = m_buffList.getIDList(),
    });

    dispatchInViewCONetPackage(SM_BUFFIDLIST, sdBuf);
    if(isPlayer()){
        dynamic_cast<Player *>(this)->postNetMessage(SM_BUFFIDLIST, sdBuf);
    }
}

void BattleObject::updateBuffList()
{
    if(m_buffList.update()){
        dispatchBuffIDList();
    }
}

void BattleObject::removeBuff(uint64_t buffSeq, bool dispatch)
{
    if(auto buffp = m_buffList.hasBuffSeq(buffSeq)){
        const auto fromUID = buffp->fromUID();
        m_buffList.erase(buffSeq);

        AMRemoveBuff amRB;
        std::memset(&amRB, 0, sizeof(amRB));

        amRB.fromUID = UID();
        amRB.fromBuffSeq = buffSeq;

        for(const auto uid: getInViewUIDList()){
            switch(uidf::getUIDType(uid)){
                case UID_PLY:
                case UID_MON:
                    {
                        m_actorPod->forward(uid, {AM_REMOVEBUFF, amRB});
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }

        for(auto fromBuff: m_buffList.hasFromBuff(fromUID, buffSeq)){
            removeBuff(fromBuff->buffSeq(), false);
        }

        if(dispatch){
            dispatchBuffIDList();
        }
    }
}

void BattleObject::removeFromBuff(uint64_t fromUID, uint64_t fromBuffSeq, bool dispatch)
{
    for(auto pbuff: m_buffList.hasFromBuff(fromUID, fromBuffSeq)){
        removeBuff(pbuff->buffSeq(), false);
    }

    if(dispatch){
        dispatchBuffIDList();
    }
}

BaseBuff *BattleObject::addBuff(uint64_t fromUID, uint64_t fromBuffSeq, uint32_t buffID)
{
    const auto fnAddBuff = [fromUID, fromBuffSeq, buffID, this]() -> BaseBuff *
    {
        if(auto buff = m_buffList.addBuff(std::make_unique<BaseBuff>(this, fromUID, fromBuffSeq, buffID, m_buffList.rollSeqID(buffID)))){
            for(const auto paura: buff->getAuraList()){
                paura->dispatch();
                if(paura->getBAR().aura.self){
                    addBuff(buff->fromUID(), buff->buffSeq(), DBCOM_BUFFID(paura->getBAR().aura.buff));
                }
            }

            if(buff->getBR().icon.show){
                dispatchBuffIDList();
            }
            return buff;
        }
        return nullptr;
    };

    // NOTE currently only *replace* buff added by self
    // can not remove/override buffs added by other person to avoid cheating

    if(const auto &br = DBCOM_BUFFRECORD(buffID); br){
        auto pbuffList = m_buffList.hasBuff(br.name);
        auto pbuffListFromUID = m_buffList.hasFromBuff(fromUID, fromBuffSeq, buffID);

        fflassert(pbuffList.size() >= pbuffListFromUID.size());
        if(to_d(pbuffList.size()) < br.stackCount + 1){
            return fnAddBuff();
        }
        else if(!pbuffListFromUID.empty() && br.stackReplace){
            // total number has reach max stack count
            // but there are buffs from fromUID and can replace, remove the oldest and add new buff
            m_buffList.erase(pbuffListFromUID.front()->buffSeq());
            return fnAddBuff();
        }
    }
    return nullptr;
}

bool BattleObject::updateHealth(int addHP, int addMP, int addMaxHP, int addMaxMP)
{
    if(m_sdHealth.updateHealth(addHP, addMP, addMaxHP, addMaxMP)){
        dispatchInViewCONetPackage(SM_HEALTH, cerealf::serialize(m_sdHealth));
        return true;
    }
    return false;
}

std::pair<int, SDTaggedValMap &> BattleObject::updateBuffedAbility(uint32_t buffActID, int percentage, int value)
{
    fflassert(std::abs(percentage) >= 0);
    fflassert(std::abs(percentage) <= 100);

    const auto fnAddValue = [percentage, value](int currValue) -> int
    {
        return std::lround(currValue * percentage / 100.0) + value;
    };

    switch(buffActID){
        case DBCOM_BUFFACTID(u8"DC下限"):
            {
                return {m_sdBuffedAbility.dc[0].add(fnAddValue(m_sdBuffedAbility.dc[0].sum())), m_sdBuffedAbility.dc[0]};
            }
        case DBCOM_BUFFACTID(u8"DC上限"):
            {
                return {m_sdBuffedAbility.dc[1].add(fnAddValue(m_sdBuffedAbility.dc[1].sum())), m_sdBuffedAbility.dc[1]};
            }
        case DBCOM_BUFFACTID(u8"AC下限"):
            {
                return {m_sdBuffedAbility.ac[0].add(fnAddValue(m_sdBuffedAbility.ac[0].sum())), m_sdBuffedAbility.ac[0]};
            }
        case DBCOM_BUFFACTID(u8"AC上限"):
            {
                return {m_sdBuffedAbility.ac[1].add(fnAddValue(m_sdBuffedAbility.ac[1].sum())), m_sdBuffedAbility.ac[1]};
            }
        case DBCOM_BUFFACTID(u8"MAC下限"):
            {
                return {m_sdBuffedAbility.mac[0].add(fnAddValue(m_sdBuffedAbility.mac[0].sum())), m_sdBuffedAbility.mac[0]};
            }
        case DBCOM_BUFFACTID(u8"MAC上限"):
            {
                return {m_sdBuffedAbility.mac[1].add(fnAddValue(m_sdBuffedAbility.mac[1].sum())), m_sdBuffedAbility.mac[1]};
            }
        default:
            {
                throw fflvalue(buffActID, percentage, value);
            }
    }
}

void BattleObject::sendBuff(uint64_t uid, uint64_t fromBuffSeq, uint32_t buffID)
{
    AMAddBuff amAB;
    std::memset(&amAB, 0, sizeof(amAB));

    amAB.id = buffID;
    amAB.fromUID = UID();
    amAB.fromBuffSeq = fromBuffSeq;
    m_actorPod->forward(uid, {AM_ADDBUFF, amAB});
}
