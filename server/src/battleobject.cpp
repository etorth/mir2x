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
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"
#include "basebuff.hpp"
#include "bufflist.hpp"
#include "periodicbuff.hpp"

extern MonoServer *g_monoServer;
BattleObject::BOPathFinder::BOPathFinder(const BattleObject *boPtr, int nCheckCO)
    : AStarPathFinder([this](int nSrcX, int nSrcY, int nDstX, int nDstY) -> double
      {
          // we pass lambda to ctor of AStarPathFinder()
          // only capture *this*, this helps std::function to be not expensive

          if(0){
              if(true
                      && MaxStep() != 1
                      && MaxStep() != 2
                      && MaxStep() != 3){
                    throw fflerror("invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
              }

              const int nDistance2 = mathf::LDistance2(nSrcX, nSrcY, nDstX, nDstY);
              if(true
                      && nDistance2 != 1
                      && nDistance2 != 2
                      && nDistance2 != MaxStep() * MaxStep()
                      && nDistance2 != MaxStep() * MaxStep() * 2){
                  throw fflerror("invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
              }
          }
          return m_BO->OneStepCost(this, m_checkCO, nSrcX, nSrcY, nDstX, nDstY);
      }, boPtr->MaxStep())
    , m_BO(boPtr)
    , m_checkCO(nCheckCO)
    , m_cache()
{
    if(!m_BO){
        throw fflerror("invalid argument: CO = %p, CheckCO = %d", to_cvptr(m_BO), m_checkCO);
    }

    switch(m_checkCO){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid argument: CO = %p, CheckCO = %d", to_cvptr(m_BO), m_checkCO);
            }
    }

    switch(m_BO->MaxStep()){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid MaxStep provided: %d, should be (1, 2, 3)", m_BO->MaxStep());
            }
    }
}

int BattleObject::BOPathFinder::GetGrid(int nX, int nY) const
{
    if(!m_BO->GetServerMap()->validC(nX, nY)){
        return PathFind::INVALID;
    }

    const uint32_t nKey = (to_u32(nX) << 16) | to_u32(nY);
    if(auto p = m_cache.find(nKey); p != m_cache.end()){
        return p->second;
    }
    return (m_cache[nKey] = m_BO->CheckPathGrid(nX, nY));
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
    m_stateTrigger.install([this, lastCheckTick = to_u32(0)]() mutable -> bool
    {
        if(const auto currTick = g_monoServer->getCurrTick(); lastCheckTick + 5000 < currTick){
            if(checkActorPod()){
                // remove all dead ones
                // dispatch action requires check location list
                dispatchAction(makeActionStand());
            }
            lastCheckTick = currTick;
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

                    if(directionValid(nDirection)){
                        m_direction = nDirection;
                    }
                    else{
                        m_direction = PathFind::GetDirection(oldX, oldY, X(), Y());
                    }

                    AMJumpOK amJOK;
                    std::memset(&amJOK, 0, sizeof(amJOK));

                    amJOK.uid = UID();
                    amJOK.mapID = mapID();
                    amJOK.action = ActionJump
                    {
                        .x = X(),
                        .y = Y(),
                        .direction = Direction(),
                    };

                    m_actorPod->forward(rmpk.from(), {AM_JUMPOK, amJOK}, rmpk.seqID());
                    trimInViewCO();

                    if(isPlayer()){
                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                    }

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

bool BattleObject::requestMove(int nX, int nY, int nSpeed, bool allowHalfMove, bool removeMonster, std::function<void()> onOK, std::function<void()> onError)
{
    if(!m_map->groundValid(nX, nY)){
        throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_lld(mapID()), nX, nY);
    }

    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    if(estimateHop(nX, nY) != 1){
        if(onError){
            onError();
        }
        return false;
    }

    if(removeMonster){
        throw fflerror("RemoveMonster in requestMove() not implemented yet");
    }

    switch(mathf::LDistance2(X(), Y(), nX, nY)){
        case 1:
        case 2:
            {
                switch(CheckPathGrid(nX, nY)){
                    case PathFind::FREE:
                        {
                            break;
                        }
                    case PathFind::OCCUPIED:
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
                int nXm = -1;
                int nYm = -1;
                PathFind::GetFrontLocation(&nXm, &nYm, X(), Y(), PathFind::GetDirection(X(), Y(), nX, nY), 1);

                // for strict co check
                // need to skip the current (X(), Y())

                if(OneStepCost(nullptr, 2, nXm, nYm, nX, nY) < 0.00){
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
    amTM.EndX          = nX;
    amTM.EndY          = nY;
    amTM.AllowHalfMove = allowHalfMove;
    amTM.RemoveMonster = removeMonster;

    m_moveLock = true;
    return m_actorPod->forward(mapUID(), {AM_TRYMOVE, amTM}, [this, nX, nY, nSpeed, onOK, onError](const ActorMsgPack &rmpk)
    {
        fflassert(m_moveLock);
        m_moveLock = false;

        // handle move, CO may be dead
        // need to check if current CO can move

        switch(rmpk.type()){
            case AM_ALLOWMOVE:
                {
                    // since we may allow half move
                    // servermap permitted dst may not be (nX, nY)

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

                    m_direction = PathFind::GetDirection(oldX, oldY, X(), Y());

                    AMMoveOK amMOK;
                    std::memset(&amMOK, 0, sizeof(amMOK));
                    amMOK.uid = UID();
                    amMOK.mapID = mapID();
                    amMOK.action = ActionMove
                    {
                        .speed = nSpeed,
                        .x = oldX,
                        .y = oldY,
                        .aimX = X(),
                        .aimY = Y(),
                    };

                    m_actorPod->forward(rmpk.from(), {AM_MOVEOK, amMOK}, rmpk.seqID());
                    trimInViewCO();

                    if(isPlayer()){
                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                    }

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
                        .x = oldX,
                        .y = oldY,
                        .aimX = X(),
                        .aimY = Y(),
                        .direction = Direction(),
                    };

                    m_actorPod->forward(rmpk.from(), {AM_SPACEMOVEOK, amSMOK}, rmpk.seqID());
                    trimInViewCO();

                    if(isPlayer()){
                        dynamic_cast<Player *>(this)->reportAction(UID(), mapID(), amSMOK.action);
                        dynamic_cast<Player *>(this)->notifySlaveGLoc();
                    }

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
                    m_actorPod->forward(uidf::getMapUID(argMapID), {AM_TRYMAPSWITCH, amTMS}, [mpk, onOK, onError, this](const ActorMsgPack &rmpk)
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
    fflassert(directionValid(dir));
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

void BattleObject::dispatchAttackDamage(uint64_t nUID, int nDC)
{
    if(nUID && dcValid(nDC, true)){
        AMAttack amA;
        std::memset(&amA, 0, sizeof(amA));

        amA.UID   = UID();
        amA.mapID = mapID();

        amA.X = X();
        amA.Y = Y();

        amA.damage = getAttackDamage(nDC);
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

    int nLDistance2 = mathf::LDistance2(nX, nY, X(), Y());
    switch(nLDistance2){
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
                auto nMaxStep = MaxStep();
                switch(nMaxStep){
                    case 1:
                    case 2:
                    case 3: break;
                    default: throw bad_value(nMaxStep);
                }

                if(false
                        || nLDistance2 == 1 * nMaxStep * nMaxStep
                        || nLDistance2 == 2 * nMaxStep * nMaxStep){

                    const auto nDir = PathFind::GetDirection(X(), Y(), nX, nY);
                    const auto nMaxReach = oneStepReach(nDir, nMaxStep);

                    if(nMaxReach.has_value() && std::get<2>(nMaxReach.value()) == nMaxStep){
                        return 1;
                    }
                }
                return 2;
            }
    }
}

int BattleObject::CheckPathGrid(int nX, int nY) const
{
    if(!m_map){
        throw fflerror("CO has no map associated");
    }

    if(!m_map->getMapData().validC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_map->getMapData().cell(nX, nY).land.canThrough()){
        return PathFind::OBSTACLE;
    }

    if(X() == nX && Y() == nY){
        return PathFind::OCCUPIED;
    }

    for(const auto &[uid, coLoc]: m_inViewCOList){
        if(coLoc.x == nX && coLoc.y == nY){
            return PathFind::OCCUPIED;
        }
    }
    return PathFind::FREE;
}

std::array<PathFind::PathNode, 3> BattleObject::GetChaseGrid(int nX, int nY, int nDLen) const
{
    // always get the next step to chase
    // this function won't check if (nX, nY) is valid

    const int nX0 = X();
    const int nY0 = Y();

    std::array<PathFind::PathNode, 3> pathNodeList
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

std::vector<PathFind::PathNode> BattleObject::GetValidChaseGrid(int nX, int nY, int nDLen) const
{
    std::vector<PathFind::PathNode> result;
    for(const auto &node: GetChaseGrid(nX, nY, nDLen)){
        if(m_map->groundValid(node.X, node.Y)){
            result.push_back(node);
        }
    }
    return result;
}

void BattleObject::GetValidChaseGrid(int nX, int nY, int nDLen, scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3> &buf) const
{
    buf.c.clear();
    for(const auto &node: GetChaseGrid(nX, nY, nDLen)){
        if(m_map->groundValid(node.X, node.Y)){
            buf.c.push_back(node);
        }
    }

    if(buf.c.size() > buf.svocap()){
        throw fflerror("more than 3 valid chase grid found");
    }
}

double BattleObject::OneStepCost(const BattleObject::BOPathFinder *pFinder, int nCheckCO, int nX0, int nY0, int nX1, int nY1) const
{
    switch(nCheckCO){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid argument: BOPathFinder = %p, CheckCO = %d", to_cvptr(pFinder), nCheckCO);
            }
    }

    int nMaxIndex = -1;
    switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
        case 0:
            {
                nMaxIndex = 0;
                break;
            }
        case 1:
        case 2:
            {
                nMaxIndex = 1;
                break;
            }
        case 4:
        case 8:
            {
                nMaxIndex = 2;
                break;
            }
        case  9:
        case 18:
            {
                nMaxIndex = 3;
                break;
            }
        default:
            {
                return -1.00;
            }
    }

    int nDX = (nX1 > nX0) - (nX1 < nX0);
    int nDY = (nY1 > nY0) - (nY1 < nY0);

    double fExtraPen = 0.00;
    for(int nIndex = 0; nIndex <= nMaxIndex; ++nIndex){
        int nCurrX = nX0 + nDX * nIndex;
        int nCurrY = nY0 + nDY * nIndex;
        switch(const auto nGrid = pFinder ? pFinder->GetGrid(nCurrX, nCurrY) : CheckPathGrid(nCurrX, nCurrY)){
            case PathFind::FREE:
                {
                    break;
                }
            case PathFind::OCCUPIED:
                {
                    switch(nCheckCO){
                        case 1:
                            {
                                fExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return -1.00;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    break;
                }
            case PathFind::INVALID:
            case PathFind::OBSTACLE:
                {
                    return -1.00;
                }
            default:
                {
                    throw fflerror("invalid grid provided: %d at (%d, %d)", nGrid, nCurrX, nCurrY);
                }
        }
    }

    return 1.00 + nMaxIndex * 0.10 + fExtraPen;
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
                                    throw bad_reach();
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

void BattleObject::addBuff(uint32_t buffID)
{
    switch(buffID){
        case DBCOM_BUFFID(u8"治愈术"):
            {
                m_buffList.addBuff(std::unique_ptr<BaseBuff>(new PeriodicBuff
                {
                    buffID,
                    this,
                    [this](PeriodicBuff *)
                    {
                        updateHealth(5, {}, {}, {});
                    },
                }));
                break;
            }
        default:
            {
                break;
            }
    }

    dispatchBuffIDList();
}

bool BattleObject::updateHealth(std::optional<int> hp, std::optional<int> mp, std::optional<int> maxHP, std::optional<int> maxMP)
{
    const auto oldHP    = m_sdHealth.hp;
    const auto oldMP    = m_sdHealth.mp;
    const auto oldMaxHP = m_sdHealth.maxHP;
    const auto oldMaxMP = m_sdHealth.maxMP;

    m_sdHealth.maxHP = std::max<int>(0, m_sdHealth.maxHP + maxHP.value_or(0));
    m_sdHealth.maxMP = std::max<int>(0, m_sdHealth.maxMP + maxMP.value_or(0));

    m_sdHealth.hp = mathf::bound<int>(m_sdHealth.hp + hp.value_or(0), 0, m_sdHealth.maxHP);
    m_sdHealth.mp = mathf::bound<int>(m_sdHealth.mp + mp.value_or(0), 0, m_sdHealth.maxMP);

    const bool changed = false
        || oldHP    != m_sdHealth.hp
        || oldMP    != m_sdHealth.mp
        || oldMaxHP != m_sdHealth.maxHP
        || oldMaxMP != m_sdHealth.maxMP;

    if(changed){
        const auto sdBuf = cerealf::serialize(m_sdHealth);
        dispatchInViewCONetPackage(SM_HEALTH, sdBuf);
        if(isPlayer()){
            dynamic_cast<Player *>(this)->postNetMessage(SM_HEALTH, sdBuf);
        }
    }
    return changed;
}
