/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
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
#include <cinttypes>
#include "uidf.hpp"
#include "totype.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "mathf.hpp"
#include "taodog.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"

extern MonoServer *g_monoServer;
CharObject::COPathFinder::COPathFinder(const CharObject *pCO, int nCheckCO)
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
          return m_CO->OneStepCost(this, m_checkCO, nSrcX, nSrcY, nDstX, nDstY);
      }, pCO->MaxStep())
    , m_CO(pCO)
    , m_checkCO(nCheckCO)
    , m_cache()
{
    if(!m_CO){
        throw fflerror("invalid argument: CO = %p, CheckCO = %d", to_cvptr(m_CO), m_checkCO);
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
                throw fflerror("invalid argument: CO = %p, CheckCO = %d", to_cvptr(m_CO), m_checkCO);
            }
    }

    switch(m_CO->MaxStep()){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid MaxStep provided: %d, should be (1, 2, 3)", m_CO->MaxStep());
            }
    }
}

int CharObject::COPathFinder::GetGrid(int nX, int nY) const
{
    if(!m_CO->GetServerMap()->validC(nX, nY)){
        return PathFind::INVALID;
    }

    const uint32_t nKey = (to_u32(nX) << 16) | to_u32(nY);
    if(auto p = m_cache.find(nKey); p != m_cache.end()){
        return p->second;
    }
    return (m_cache[nKey] = m_CO->CheckPathGrid(nX, nY));
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

bool CharObject::NextLocation(int *pX, int *pY, int nDirection, int nDistance)
{
    int nDX = 0;
    int nDY = 0;
    switch(nDirection){
        case DIR_UP:        { nDX =  0; nDY = -1; break; }
        case DIR_UPRIGHT:   { nDX = +1; nDY = -1; break; }
        case DIR_RIGHT:     { nDX = +1; nDY =  0; break; }
        case DIR_DOWNRIGHT: { nDX = +1; nDY = +1; break; }
        case DIR_DOWN:      { nDX =  0; nDY = +1; break; }
        case DIR_DOWNLEFT:  { nDX = -1; nDY = +1; break; }
        case DIR_LEFT:      { nDX = -1; nDY =  0; break; }
        case DIR_UPLEFT:    { nDX = -1; nDY = -1; break; }
        default:            { return false;              }
    }

    if(pX){ *pX = X() + (nDX * nDistance); }
    if(pY){ *pY = Y() + (nDY * nDistance); }

    return true;
}

void CharObject::dispatchAction(const ActionNode &action)
{
    checkActorPodEx();

    AMAction amA;
    std::memset(&amA, 0, sizeof(amA));

    amA.UID = UID();
    amA.mapID = mapID();
    amA.action = action;

    setLastAction(amA.action.type);
    switch(amA.action.type){
        case ACTION_JUMP:
        case ACTION_MOVE:
        case ACTION_PUSHMOVE:
        case ACTION_SPACEMOVE1:
        case ACTION_SPACEMOVE2:
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
    checkActorPodEx();

    AMAction amA;
    std::memset(&amA, 0, sizeof(amA));

    amA.UID = UID();
    amA.mapID = mapID();
    amA.action = action;
    m_actorPod->forward(uid, {AM_ACTION, amA});
}

bool CharObject::requestJump(int nX, int nY, int nDirection, std::function<void()> onOK, std::function<void()> onError)
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
            case AM_JUMPOK:
                {
                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), AM_ERROR, rmpk.seqID());
                        if(onError){
                            onError();
                        }
                        return;
                    }

                    const auto nOldX = m_X;
                    const auto nOldY = m_Y;
                    const auto amJOK = rmpk.conv<AMJumpOK>();

                    m_X = amJOK.EndX;
                    m_Y = amJOK.EndY;

                    if(directionValid(nDirection)){
                        m_direction = nDirection;
                    }
                    else{
                        m_direction = PathFind::GetDirection(nOldX, nOldY, X(), Y());
                    }

                    m_actorPod->forward(rmpk.from(), AM_OK, rmpk.seqID());
                    dispatchAction(ActionJump
                    {
                        .x = X(),
                        .y = Y(),
                        .direction = Direction(),
                    });

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

bool CharObject::requestMove(int nX, int nY, int nSpeed, bool allowHalfMove, bool removeMonster, std::function<void()> onOK, std::function<void()> onError)
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
            case AM_MOVEOK:
                {
                    const auto amMOK = rmpk.conv<AMMoveOK>();
                    // since we may allow half move
                    // servermap permitted dst may not be (nX, nY)

                    if(!m_map->validC(amMOK.EndX, amMOK.EndY)){
                        throw fflerror("map returns invalid destination: (%" PRIu64 ", %d, %d)", m_map->UID(), amMOK.EndX, amMOK.EndY);
                    }

                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), AM_ERROR, rmpk.seqID());
                        if(onError){
                            onError();
                        }
                        return;
                    }

                    auto nOldX = m_X;
                    auto nOldY = m_Y;

                    m_X = amMOK.EndX;
                    m_Y = amMOK.EndY;

                    m_direction = PathFind::GetDirection(nOldX, nOldY, X(), Y());
                    m_actorPod->forward(rmpk.from(), AM_OK, rmpk.seqID());

                    dispatchAction(ActionMove
                    {
                        .speed = nSpeed,
                        .x = nOldX,
                        .y = nOldY,
                        .aimX = X(),
                        .aimY = Y(),
                        .onHorse = (bool)(Horse()),
                    });

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

bool CharObject::requestSpaceMove(int locX, int locY, bool strictMove, std::function<void()> onOK, std::function<void()> onError)
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
            case AM_SPACEMOVEOK:
                {
                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), AM_ERROR, rmpk.seqID());
                        if(onError){
                            onError();
                        }
                        return;
                    }

                    // dispatch space move part 1 on old place
                    dispatchAction(ActionSpaceMove1
                    {
                        .x = X(),
                        .y = Y(),
                        .direction = Direction(),
                    });

                    // setup new map
                    // don't use the requested location
                    const auto amSMOK = rmpk.conv<AMSpaceMoveOK>();
                    m_X = amSMOK.EndX;
                    m_Y = amSMOK.EndY;

                    m_actorPod->forward(rmpk.from(), AM_OK, rmpk.seqID());

                    // clean the InViewCO list
                    // report new location explicitly to map
                    // don't use ActionStand since it forces client to parse for a path to dst location
                    m_inViewCOList.clear();
                    const ActionSpaceMove2 spaceMove2
                    {
                        .x = X(),
                        .y = Y(),
                        .direction = Direction(),
                    };

                    dispatchAction(m_map->UID(), spaceMove2);
                    if(uidf::getUIDType(UID()) == UID_PLY){
                        dynamic_cast<Player *>(this)->reportAction(UID(), spaceMove2);
                        dynamic_cast<Player *>(this)->PullRectCO(10, 10);
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

bool CharObject::requestMapSwitch(uint32_t argMapID, int locX, int locY, bool strictMove, std::function<void()> onOK, std::function<void()> onError)
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

    AMQueryMapUID amQMUID;
    std::memset(&amQMUID, 0, sizeof(amQMUID));

    amQMUID.mapID = argMapID;
    return m_actorPod->forward(uidf::getServiceCoreUID(), {AM_QUERYMAPUID, amQMUID}, [locX, locY, strictMove, onOK, onError, this](const ActorMsgPack &mpk)
    {
        switch(mpk.type()){
            case AM_UID:
                {
                    AMTryMapSwitch amTMS;
                    std::memset(&amTMS, 0, sizeof(amTMS));

                    amTMS.X = locX;
                    amTMS.Y = locY;
                    amTMS.strictMove = strictMove;

                    // send request to the new map
                    // if request rejected then it stays in current map

                    m_moveLock = true;
                    m_actorPod->forward(mpk.conv<AMUID>().UID, {AM_TRYMAPSWITCH, amTMS}, [mpk, onOK, onError, this](const ActorMsgPack &rmpk)
                    {
                        fflassert(m_moveLock);
                        m_moveLock = false;

                        switch(rmpk.type()){
                            case AM_MAPSWITCHOK:
                                {
                                    // new map accepts this switch request
                                    // new map will guarantee to outlive current object

                                    if(!canMove()){
                                        m_actorPod->forward(rmpk.from(), AM_ERROR, rmpk.seqID());
                                        if(onError){
                                            onError();
                                        }
                                        return;
                                    }

                                    const auto amMSOK = rmpk.conv<AMMapSwitchOK>();
                                    const auto newMapPtr = (ServerMap *)(amMSOK.Ptr);

                                    if(!(true && newMapPtr
                                              && newMapPtr->ID()
                                              && newMapPtr->UID()
                                              && newMapPtr->validC(amMSOK.X, amMSOK.Y))){

                                        // fake map
                                        // invalid argument, this is not good place to call onError()

                                        m_actorPod->forward(rmpk.from(), AM_ERROR, rmpk.seqID());
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
                                    m_actorPod->forward(m_map->UID(), {AM_TRYLEAVE, amTL}, [this, amMSOK, rmpk, onOK, onError](const ActorMsgPack &leavermpk)
                                    {
                                        fflassert(m_moveLock);
                                        m_moveLock = false;

                                        switch(leavermpk.type()){
                                            case AM_OK:
                                                {
                                                    const auto amMSOK = rmpk.conv<AMMapSwitchOK>();
                                                    if(!canMove()){
                                                        m_actorPod->forward(rmpk.from(), AM_ERROR, rmpk.seqID());
                                                        if(onError){
                                                            onError();
                                                        }
                                                        return;
                                                    }

                                                    // 1. response to new map ``I am here"
                                                    m_map = (ServerMap *)(amMSOK.Ptr);
                                                    m_X = amMSOK.X;
                                                    m_Y = amMSOK.Y;
                                                    m_actorPod->forward(m_map->UID(), AM_OK, rmpk.seqID());

                                                    // 2. notify all players on the new map
                                                    //    need to explicitly send to the map, not InViewCO since it's not valid anymore
                                                    m_inViewCOList.clear();
                                                    dispatchAction(m_map->UID(), makeActionStand());

                                                    // 3. inform the client for map swith
                                                    // 4. get neighbors

                                                    if(uidf::getUIDType(UID()) == UID_PLY){
                                                        dynamic_cast<Player *>(this)->reportStand();
                                                        dynamic_cast<Player *>(this)->PullRectCO(10, 10);
                                                    }

                                                    if(onOK){
                                                        onOK();
                                                    }
                                                    return;
                                                }
                                            default:
                                                {
                                                    // can't leave, illegal response
                                                    // any sane implementation should allow an UID to leave

                                                    // if an UID can't move
                                                    // then we shouldn't call this function
                                                    m_actorPod->forward(((ServerMap *)(amMSOK.Ptr))->UID(), AM_ERROR, rmpk.seqID());
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

bool CharObject::canAct() const
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

bool CharObject::canMove() const
{
    return canAct() && !m_moveLock;
}

bool CharObject::canAttack() const
{
    return canAct() && !m_attackLock;
}

void CharObject::getCOLocation(uint64_t nUID, std::function<void(const COLocation &)> onOK, std::function<void()> onError)
{
    fflassert(nUID);
    fflassert(nUID != UID());

    // CO dispatches location changes automatically
    // always trust the InViewCOList, can even skip the expiration now

    if(auto p = getInViewCOPtr(nUID)){
        onOK(*p);
        return;
    }

    // can't find uid or expired
    // query the location and put to InViewCOList if applicable

    AMQueryLocation amQL;
    std::memset(&amQL, 0, sizeof(amQL));

    amQL.UID   = UID();
    amQL.mapID = mapID();

    m_actorPod->forward(nUID, {AM_QUERYLOCATION, amQL}, [this, nUID, onOK, onError](const ActorMsgPack &rstRMPK)
    {
        switch(rstRMPK.type()){
            case AM_LOCATION:
                {
                    AMLocation amL;
                    std::memcpy(&amL, rstRMPK.data(), sizeof(amL));

                    // TODO when we get this response
                    // it's possible that the co has switched map or dead

                    const COLocation coLoc
                    {
                        .uid       = amL.UID,
                        .mapID     = amL.mapID,
                        .x         = amL.X,
                        .y         = amL.Y,
                        .direction = amL.Direction
                    };

                    updateInViewCO(coLoc);
                    onOK(coLoc);
                    return;
                }
            default:
                {
                    // TODO dangerous part here
                    // when nUID is not detached ActorPod::forward receives AM_BADACTORPOD immedately
                    // then this branch get called, then m_inViewCOList get updated implicitly

                    m_inViewCOList.erase(nUID);
                    onError();
                    return;
                }
        }
    });
}

void CharObject::addOffenderDamage(uint64_t nUID, int nDamage)
{
    if(!nUID){
        throw fflerror("offender with zero UID");
    }

    if(nDamage < 0){
        throw fflerror("invalid offender damage: %d", nDamage);
    }

    for(auto p = m_offenderList.begin(); p != m_offenderList.end(); ++p){
        if(p->UID == nUID){
            p->Damage += nDamage;
            p->ActiveTime = g_monoServer->getCurrTick();
            return;
        }
    }
    m_offenderList.emplace_back(nUID, nDamage, g_monoServer->getCurrTick());
}

void CharObject::dispatchOffenderExp()
{
    for(auto p = m_offenderList.begin(); p != m_offenderList.end();){
        if(g_monoServer->getCurrTick() >= p->ActiveTime + 2 * 60 * 3600){
            p = m_offenderList.erase(p);
        }else{
            p++;
        }
    }

    if(m_offenderList.empty()){
        return;
    }

    auto fnCalcExp = [this](int nDamage) -> int
    {
        return nDamage * m_offenderList.size();
    };

    for(const auto &rstOffender: m_offenderList){
        if(auto nType = uidf::getUIDType(rstOffender.UID); nType == UID_MON || nType == UID_PLY){
            AMExp amE;
            std::memset(&amE, 0, sizeof(amE));

            amE.exp = fnCalcExp(rstOffender.Damage);
            m_actorPod->forward(rstOffender.UID, {AM_EXP, amE});
        }
    }
}

int CharObject::OneStepReach(int nDirection, int nMaxDistance, int *pX, int *pY)
{
    if(m_map){
        if(nMaxDistance > 0){
            int nLastX = -1;
            int nLastY = -1;
            int nLastD =  0;
            for(int nDistance = 1; nDistance <= nMaxDistance; ++nDistance){
                int nX = -1;
                int nY = -1;
                if(true
                        && NextLocation(&nX, &nY, nDirection, nDistance)
                        && m_map->groundValid(nX, nY)){
                    nLastX = nX;
                    nLastY = nY;
                    nLastD = nDistance;
                }else{ break; }
            }

            if(nLastD && pX){ *pX = nLastX; }
            if(nLastD && pY){ *pY = nLastY; }

            // return could be 0 : keep pX and pY un-touched
            //                 N : return (*pX, *pY) and maximal distance
            return nLastD;
        }
        return nMaxDistance;
    }
    return -1;
}

void CharObject::dispatchHealth()
{
    dispatchInViewCONetPackage(SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void CharObject::dispatchAttackDamage(uint64_t nUID, int nDC)
{
    if(nUID && DCValid(nDC, true)){
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

int CharObject::Speed(int nSpeedType) const
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

int CharObject::estimateHop(int nX, int nY)
{
    condcheck(m_map);
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
                condcheck(false
                        || nMaxStep == 1
                        || nMaxStep == 2
                        || nMaxStep == 3);

                if(false
                        || nLDistance2 == 1 * nMaxStep * nMaxStep
                        || nLDistance2 == 2 * nMaxStep * nMaxStep){

                    auto nDir = PathFind::GetDirection(X(), Y(), nX, nY);
                    auto nMaxReach = OneStepReach(nDir, nMaxStep, nullptr, nullptr);

                    if(nMaxReach == nMaxStep){
                        return 1;
                    }
                }
                return 2;
            }
    }
}

int CharObject::CheckPathGrid(int nX, int nY) const
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

std::array<PathFind::PathNode, 3> CharObject::GetChaseGrid(int nX, int nY, int nDLen) const
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

std::vector<PathFind::PathNode> CharObject::GetValidChaseGrid(int nX, int nY, int nDLen) const
{
    std::vector<PathFind::PathNode> result;
    for(const auto &node: GetChaseGrid(nX, nY, nDLen)){
        if(m_map->groundValid(node.X, node.Y)){
            result.push_back(node);
        }
    }
    return result;
}

void CharObject::GetValidChaseGrid(int nX, int nY, int nDLen, scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3> &buf) const
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

double CharObject::OneStepCost(const CharObject::COPathFinder *pFinder, int nCheckCO, int nX0, int nY0, int nX1, int nY1) const
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
                throw fflerror("invalid argument: COPathFinder = %p, CheckCO = %d", to_cvptr(pFinder), nCheckCO);
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

void CharObject::setLastAction(int type)
{
    m_lastAction = type;
    m_lastActionTime.at(type) = g_monoServer->getCurrTick();
}

bool CharObject::inView(uint32_t argMapID, int argX, int argY) const
{
    return m_map->In(argMapID, argX, argY) && mathf::LDistance2<int>(X(), Y(), argX, argY) <= SYS_VIEWR * SYS_VIEWR;
}

void CharObject::foreachInViewCO(std::function<void(const COLocation &)> fnOnLoc)
{
    // the updateInViewCO() may get called in fnOnLoc
    // which changes m_inViewCOList

    scoped_alloc::svobuf_wrapper<COLocation, 128> coLocList;
    for(const auto &[uid, coLoc]: m_inViewCOList){
        coLocList.c.push_back(coLoc);
    }

    for(const auto &coLoc: coLocList.c){
        fnOnLoc(coLoc);
    }
}

bool CharObject::updateInViewCO(const COLocation &coLoc, bool forceDelete)
{
    if(!forceDelete && inView(coLoc.mapID, coLoc.x, coLoc.y)){
        m_inViewCOList[coLoc.uid] = coLoc;
        return true;
    }
    else{
        m_inViewCOList.erase(coLoc.uid);
        return false;
    }
}

bool CharObject::isOffender(uint64_t nUID)
{
    for(auto &rstOffender: m_offenderList){
        if(rstOffender.UID == nUID){
            return true;
        }
    }
    return false;
}

void CharObject::queryHealth(uint64_t uid, std::function<void(uint64_t, SDHealth)> fnOp)
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

void CharObject::queryFinalMaster(uint64_t nUID, std::function<void(uint64_t)> fnOp)
{
    if(!nUID){
        throw fflerror("invalid zero UID");
    }

    if(uidf::getUIDType(nUID) != UID_MON){
        throw fflerror("%s can't have master", uidf::getUIDTypeCStr(nUID));
    }

    auto fnQuery = [this, fnOp](uint64_t nQueryUID)
    {
        m_actorPod->forward(nQueryUID, AM_QUERYFINALMASTER, [this, nQueryUID, fnOp](const ActorMsgPack &rstRMPK)
        {
            switch(rstRMPK.type()){
                case AM_UID:
                    {
                        AMUID amUID;
                        std::memcpy(&amUID, rstRMPK.data(), sizeof(amUID));

                        fnOp(amUID.UID);
                        return;
                    }
                default:
                    {
                        fnOp(0);
                        if(isMonster() && (nQueryUID == dynamic_cast<Monster *>(this)->masterUID())){
                            goDie();
                        }
                        return;
                    }
            }
        });
    };

    // check self type
    // we allow self-query for monster

    switch(uidf::getUIDType(UID())){
        case UID_MON:
            {
                if(nUID != UID()){
                    fnQuery(nUID);
                    return;
                }

                // querying self
                // this is ok for monster

                if(auto nMasterUID = dynamic_cast<Monster *>(this)->masterUID()){
                    switch(uidf::getUIDType(nMasterUID)){
                        case UID_PLY:
                            {
                                fnOp(nMasterUID);
                                return;
                            }
                        case UID_MON:
                            {
                                fnQuery(nMasterUID);
                                return;
                            }
                        default:
                            {
                                throw fflerror("invalid master type: %s", uidf::getUIDTypeCStr(nMasterUID));
                            }
                    }
                }

                // querying self
                // and doesn't have master
                fnOp(UID());
                return;
            }
        case UID_PLY:
        default:
            {
                if(nUID != UID()){
                    fnQuery(nUID);
                    return;
                }
                throw fflerror("%s can't query self for final master", uidf::getUIDTypeCStr(UID()));
            }
    }
}

bool CharObject::isPlayer() const
{
    return uidf::getUIDType(UID()) == UID_PLY;
}

bool CharObject::isMonster() const
{
    return uidf::getUIDType(UID()) == UID_MON;
}

bool CharObject::isMonster(const char8_t *name) const
{
    return isMonster() && dynamic_cast<const Monster * const>(this)->monsterID() == DBCOM_MONSTERID(name);
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
        .x = X(),
        .y = Y(),
        .direction = Direction(),
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
