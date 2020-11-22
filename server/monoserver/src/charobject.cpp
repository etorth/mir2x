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
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"
#include "eventtaskhub.hpp"

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
    if(!m_CO->GetServerMap()->ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    const uint32_t nKey = ((uint32_t)(nX) << 16) | (uint32_t)(nY);
    if(auto p = m_cache.find(nKey); p != m_cache.end()){
        return p->second;
    }
    return (m_cache[nKey] = m_CO->CheckPathGrid(nX, nY));
}

CharObject::CharObject(ServiceCore *pServiceCore,
        ServerMap                  *pServerMap,
        uint64_t                    nUID,
        int                         nMapX,
        int                         nMapY,
        int                         nDirection)
    : ServerObject(nUID)
    , m_serviceCore(pServiceCore)
    , m_map(pServerMap)
    , m_X(nMapX)
    , m_Y(nMapY)
    , m_direction(nDirection)
    , m_HP(0)
    , m_HPMax(0)
    , m_MP(0)
    , m_MPMax(0)
    , m_moveLock(false)
    , m_attackLock(false)
    , m_lastMoveTime(0)
    , m_lastAttackTime(0)
    , m_lastAction(ACTION_NONE)
    , m_lastActionTime(0)
    , m_target()
{
    if(!m_map){
        throw fflerror("CO has no assciated map");
    }

    m_stateHook.Install("RemoveDeadUIDLocation", [this, nLastCheckTick = (uint32_t)(0)]() mutable -> bool
    {
        if(auto nCurrCheckTick = g_monoServer->getCurrTick(); nLastCheckTick + 5000 < nCurrCheckTick){
            if(ActorPodValid()){
                // remove all dead ones
                // dispatch action requires check location list
                DispatchAction(ActionStand(X(), Y(), Direction()));
            }
            nLastCheckTick = nCurrCheckTick;
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

uint64_t CharObject::activate()
{
    if(auto nUID = ServerObject::activate(); nUID){
        DispatchAction(ActionSpawn(X(), Y(), Direction()));
        return nUID;
    }
    return 0;
}

void CharObject::DispatchAction(const ActionNode &rstAction)
{
    // should check to avoid dead CO call this function
    // this would cause zombies

    if(!ActorPodValid()){
        throw fflerror("can't dispatch action: %s", rstAction.ActionName());
    }

    AMAction stAMA;
    std::memset(&stAMA, 0, sizeof(stAMA));

    stAMA.UID   = UID();
    stAMA.MapID = MapID();

    stAMA.Action    = rstAction.Action;
    stAMA.Speed     = rstAction.Speed;
    stAMA.Direction = rstAction.Direction;

    stAMA.X    = rstAction.X;
    stAMA.Y    = rstAction.Y;
    stAMA.AimX = rstAction.AimX;
    stAMA.AimY = rstAction.AimY;

    stAMA.AimUID      = rstAction.AimUID;
    stAMA.ActionParam = rstAction.ActionParam;

    switch(stAMA.Action){
        case ACTION_MOVE:
        case ACTION_SPAWN:
        case ACTION_ATTACK:
            {
                SetLastAction(stAMA.Action);
                break;
            }
        default:
            {
                break;
            }
    }

    switch(stAMA.Action){
        case ACTION_MOVE:
        case ACTION_PUSHMOVE:
        case ACTION_SPACEMOVE1:
        case ACTION_SPACEMOVE2:
        case ACTION_SPAWN:
            {
                m_actorPod->forward(m_map->UID(), {MPK_ACTION, stAMA});
                return;
            }
        default:
            {
                foreachInViewCO([this, stAMA](const COLocation &rstLocation)
                {
                    auto nX = rstLocation.X;
                    auto nY = rstLocation.Y;
                    auto nMapID = rstLocation.MapID;
                    if(InView(nMapID, nX, nY)){
                        m_actorPod->forward(rstLocation.UID, {MPK_ACTION, stAMA});
                    }
                });
                return;
            }
    }
}

void CharObject::DispatchAction(uint64_t nUID, const ActionNode &rstAction)
{
    // should check to avoid dead CO call this function
    // this would cause zombies

    condcheck(nUID);
    condcheck(ActorPodValid());

    AMAction stAMA;
    std::memset(&stAMA, 0, sizeof(stAMA));

    stAMA.UID   = UID();
    stAMA.MapID = MapID();

    stAMA.Action    = rstAction.Action;
    stAMA.Speed     = rstAction.Speed;
    stAMA.Direction = rstAction.Direction;

    stAMA.X    = rstAction.X;
    stAMA.Y    = rstAction.Y;
    stAMA.AimX = rstAction.AimX;
    stAMA.AimY = rstAction.AimY;

    stAMA.AimUID      = rstAction.AimUID;
    stAMA.ActionParam = rstAction.ActionParam;

    m_actorPod->forward(nUID, {MPK_ACTION, stAMA});
}

bool CharObject::requestMove(int nX, int nY, int nSpeed, bool allowHalfMove, bool removeMonster, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!m_map->groundValid(nX, nY)){
        throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_lld(MapID()), nX, nY);
    }

    if(!canMove()){
        if(fnOnError){
            fnOnError();
        }
        return false;
    }

    if(estimateHop(nX, nY) != 1){
        if(fnOnError){
            fnOnError();
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
                switch(CheckPathGrid(nX, nY, 0)){
                    case PathFind::FREE:
                        {
                            break;
                        }
                    case PathFind::OCCUPIED:
                    default:
                        {
                            if(fnOnError){
                                fnOnError();
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
                        if(fnOnError){
                            fnOnError();
                        }
                        return false;
                    }
                }
                break;
            }
    }

    AMTryMove stAMTM;
    std::memset(&stAMTM, 0, sizeof(stAMTM));

    stAMTM.UID           = UID();
    stAMTM.MapID         = MapID();
    stAMTM.X             = X();
    stAMTM.Y             = Y();
    stAMTM.EndX          = nX;
    stAMTM.EndY          = nY;
    stAMTM.AllowHalfMove = allowHalfMove;
    stAMTM.RemoveMonster = removeMonster;

    m_moveLock = true;
    return m_actorPod->forward(MapUID(), {MPK_TRYMOVE, stAMTM}, [this, nX, nY, nSpeed, fnOnOK, fnOnError](const MessagePack &rstMPK)
    {
        if(!m_moveLock){
            throw fflerror("moveLock released before map responds: UIDName = %s", UIDName());
        }
        m_moveLock = false;

        // handle move, CO may be dead
        // need to check if current CO can move

        switch(rstMPK.Type()){
            case MPK_MOVEOK:
                {
                    AMMoveOK stAMMOK;
                    std::memcpy(&stAMMOK, rstMPK.Data(), sizeof(stAMMOK));

                    // since we may allow half move
                    // servermap permitted dst may not be (nX, nY)

                    if(!m_map->ValidC(stAMMOK.EndX, stAMMOK.EndY)){
                        throw fflerror("map returns invalid destination: (%" PRIu64 ", %d, %d)", m_map->UID(), stAMMOK.EndX, stAMMOK.EndY);
                    }

                    if(!canMove()){
                        m_actorPod->forward(rstMPK.from(), MPK_ERROR, rstMPK.ID());
                        if(fnOnError){
                            fnOnError();
                        }
                        return;
                    }

                    auto nOldX = m_X;
                    auto nOldY = m_Y;

                    m_X = stAMMOK.EndX;
                    m_Y = stAMMOK.EndY;

                    m_direction = PathFind::GetDirection(nOldX, nOldY, X(), Y());
                    m_lastMoveTime = g_monoServer->getCurrTick();

                    m_actorPod->forward(rstMPK.from(), MPK_OK, rstMPK.ID());
                    DispatchAction(ActionMove(nOldX, nOldY, X(), Y(), nSpeed, Horse()));
                    SortInViewCO();

                    if(fnOnOK){
                        fnOnOK();
                    }
                    return;
                }
            default:
                {
                    if(fnOnError){
                        fnOnError();
                    }
                    return;
                }
        }
    });
}

bool CharObject::requestSpaceMove(int locX, int locY, bool strictMove, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(strictMove){
        if(!m_map->groundValid(locX, locY)){
            throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_llu(MapID()), locX, locY);
        }
    }
    else{
        if(!m_map->ValidC(locX, locY)){
            throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_llu(MapID()), locX, locY);
        }
    }

    if(!canMove()){
        if(fnOnError){
            fnOnError();
        }
        return false;
    }

    AMTrySpaceMove amTSM;
    std::memset(&amTSM, 0, sizeof(amTSM));

    amTSM.UID = UID();
    amTSM.X   = locX;
    amTSM.Y   = locY;
    amTSM.StrictMove = strictMove;

    m_moveLock = true;
    return m_actorPod->forward(MapUID(), {MPK_TRYSPACEMOVE, amTSM}, [this, fnOnOK, fnOnError](const MessagePack &rmpk)
    {
        if(!m_moveLock){
            throw fflerror("moveLock released before map responds: UIDName = %s", UIDName());
        }
        m_moveLock = false;

        // handle move, CO can be dead already
        // check if current CO can move even we checked before

        switch(rmpk.Type()){
            case MPK_SPACEMOVEOK:
                {
                    // need to leave src map
                    // dst map already says OK for current move

                    if(!canMove()){
                        m_actorPod->forward(rmpk.from(), MPK_ERROR, rmpk.ID());
                        if(fnOnError){
                            fnOnError();
                        }
                        return;
                    }

                    AMTryLeave amTL;
                    std::memset(&amTL, 0, sizeof(amTL));

                    amTL.X = X();
                    amTL.Y = Y();

                    m_moveLock = true;
                    m_actorPod->forward(m_map->UID(), {MPK_TRYLEAVE, amTL}, [this, rmpk, fnOnOK, fnOnError](const MessagePack &leavermpk)
                    {
                        if(!m_moveLock){
                            throw fflerror("moveLock released before map responds: UIDName = %s", UIDName());
                        }
                        m_moveLock = false;

                        switch(leavermpk.Type()){
                            case MPK_OK:
                                {
                                    const auto amSMOK = rmpk.conv<AMSpaceMoveOK>();
                                    if(!canMove()){
                                        m_actorPod->forward(rmpk.from(), MPK_ERROR, rmpk.ID());
                                        if(fnOnError){
                                            fnOnError();
                                        }
                                        return;
                                    }

                                    // dispatch space move part 1 on old map
                                    DispatchAction(ActionSpaceMove1(X(), Y(), Direction()));

                                    // setup new map
                                    // don't use the requested location
                                    m_X   = amSMOK.X;
                                    m_Y   = amSMOK.Y;
                                    m_map = (ServerMap *)(amSMOK.Ptr);

                                    m_lastMoveTime = g_monoServer->getCurrTick();
                                    m_actorPod->forward(rmpk.from(), MPK_OK, rmpk.ID());

                                    //  dispatch/report space move part 2 on new map
                                    DispatchAction(ActionSpaceMove2(X(), Y(), Direction()));
                                    if(uidf::getUIDType(UID()) == UID_PLY){
                                        dynamic_cast<Player *>(this)->ReportAction(UID(), ActionSpaceMove2(X(), Y(), Direction()));
                                    }

                                    if(fnOnOK){
                                        fnOnOK();
                                    }
                                    return;
                                }
                            default:
                                {
                                    m_actorPod->forward(rmpk.from(), MPK_ERROR, rmpk.ID());
                                    if(fnOnError){
                                        fnOnError();
                                    }
                                    return;
                                }
                        }
                    });
                    break;
                }
            default:
                {
                    if(fnOnError){
                        fnOnError();
                    }
                    break;
                }
        }
    });
}

bool CharObject::requestMapSwitch(uint32_t mapID, int locX, int locY, bool strictMove, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(mapID == MapID()){
        throw fflerror("request to switch on same map: mapID = %llu", to_llu(mapID));
    }

    if(locX < 0 || locY < 0){
        throw fflerror("invalid argument: mapID = %llu, locX = %d, locY = %d", to_llu(mapID), locX, locY);
    }

    if(!canMove()){
        if(fnOnError){
            fnOnError();
        }
        return false;
    }

    AMQueryMapUID amQMUID;
    std::memset(&amQMUID, 0, sizeof(amQMUID));

    amQMUID.MapID = mapID;
    return m_actorPod->forward(m_serviceCore->UID(), {MPK_QUERYMAPUID, amQMUID}, [locX, locY, strictMove, fnOnOK, fnOnError, this](const MessagePack &mpk)
    {
        switch(mpk.Type()){
            case MPK_UID:
                {
                    AMTryMapSwitch amTMS;
                    std::memset(&amTMS, 0, sizeof(amTMS));

                    amTMS.X = locX;
                    amTMS.Y = locY;
                    amTMS.strictMove = strictMove;

                    // send request to the new map
                    // if request rejected then it stays in current map

                    m_moveLock = true;
                    m_actorPod->forward(mpk.conv<AMUID>().UID, {MPK_TRYMAPSWITCH, amTMS}, [mpk, fnOnOK, fnOnError, this](const MessagePack &rmpk)
                    {
                        if(!m_moveLock){
                            throw fflerror("moveLock released before map responds: UIDName = %s", UIDName());
                        }
                        m_moveLock = false;

                        switch(rmpk.Type()){
                            case MPK_MAPSWITCHOK:
                                {
                                    // new map accepts this switch request
                                    // new map will guarantee to outlive current object

                                    if(!canMove()){
                                        m_actorPod->forward(rmpk.from(), MPK_ERROR, rmpk.ID());
                                        if(fnOnError){
                                            fnOnError();
                                        }
                                        return;
                                    }

                                    const auto amMSOK = rmpk.conv<AMMapSwitchOK>();
                                    const auto newMapPtr = (ServerMap *)(amMSOK.Ptr);

                                    if(!(true && newMapPtr
                                              && newMapPtr->ID()
                                              && newMapPtr->UID()
                                              && newMapPtr->ValidC(amMSOK.X, amMSOK.Y))){

                                        // fake map
                                        // invalid argument, this is not good place to call fnOnError()

                                        m_actorPod->forward(rmpk.from(), MPK_ERROR, rmpk.ID());
                                        if(fnOnError){
                                            fnOnError();
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
                                    m_actorPod->forward(m_map->UID(), {MPK_TRYLEAVE, amTL}, [this, amMSOK, rmpk, fnOnOK, fnOnError](const MessagePack &leavermpk)
                                    {
                                        if(!m_moveLock){
                                            throw fflerror("moveLock released before map responds: UIDName = %s", UIDName());
                                        }
                                        m_moveLock = false;

                                        switch(leavermpk.Type()){
                                            case MPK_OK:
                                                {
                                                    const auto amMSOK = rmpk.conv<AMMapSwitchOK>();
                                                    if(!canMove()){
                                                        m_actorPod->forward(rmpk.from(), MPK_ERROR, rmpk.ID());
                                                        if(fnOnError){
                                                            fnOnError();
                                                        }
                                                        return;
                                                    }

                                                    // 1. response to new map ``I am here"
                                                    m_map = (ServerMap *)(amMSOK.Ptr);
                                                    m_X = amMSOK.X;
                                                    m_Y = amMSOK.Y;
                                                    m_actorPod->forward(m_map->UID(), MPK_OK, rmpk.ID());

                                                    // 2. notify all players on the new map
                                                    DispatchAction(ActionStand(X(), Y(), Direction()));

                                                    // 3. inform the client for map swith
                                                    // 4. get neighbors

                                                    if(uidf::getUIDType(UID()) == UID_PLY){
                                                        dynamic_cast<Player *>(this)->ReportStand();
                                                        dynamic_cast<Player *>(this)->PullRectCO(10, 10);
                                                    }

                                                    if(fnOnOK){
                                                        fnOnOK();
                                                    }
                                                    return;
                                                }
                                            default:
                                                {
                                                    // can't leave, illegal response
                                                    // any sane implementation should allow an UID to leave

                                                    // if an UID can't move
                                                    // then we shouldn't call this function

                                                    m_actorPod->forward(((ServerMap *)(amMSOK.Ptr))->UID(), MPK_ERROR, rmpk.ID());
                                                    g_monoServer->addLog(LOGTYPE_WARNING, "Leave request failed: mapID = %llu, mapUID = %llu", to_llu(m_map->ID()), to_llu(m_map->UID()));
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

                                    g_monoServer->addLog(LOGTYPE_WARNING, "Can't switch to new map: mapUID = %llu", to_llu(mpk.conv<AMUID>().UID));
                                    return;
                                }
                        }
                    });
                    return;
                }
            default:
                {
                    if(fnOnError){
                        fnOnError();
                    }
                    return;
                }
        }
    });
}

bool CharObject::canMove()
{
    switch(GetState(STATE_DEAD)){
        case 0:
            {
                return !m_moveLock;
            }
        default:
            {
                return false;
            }
    }
}

bool CharObject::CanAct()
{
    switch(m_lastAction){
        case ACTION_SPAWN:
            {
                switch(uidf::getUIDType(UID())){
                    case UID_MON:
                        {
                            switch(uidf::getMonsterID(UID())){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        return g_monoServer->getCurrTick() > m_lastActionTime + 600;
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
        case ACTION_ATTACK:
            {
                return g_monoServer->getCurrTick() > m_lastActionTime + 600;
            }
        case ACTION_MOVE:
            {
                return g_monoServer->getCurrTick() > m_lastActionTime + 700;
            }
        default:
            {
                break;
            }
    }
    return true;
}

bool CharObject::canAttack()
{
    switch(GetState(STATE_DEAD)){
        case 0:
            {
                return !m_attackLock;
            }
        default:
            {
                return false;
            }
    }
}

void CharObject::retrieveLocation(uint64_t nUID, std::function<void(const COLocation &)> fnOnOK, std::function<void()> fnOnError)
{
    if(!nUID){
        throw fflerror("query location with zero UID");
    }

    if(nUID == UID()){
        throw fflerror("query UID to CO itself: %llu", to_llu(nUID));
    }

    // CO dispatches location changes automatically
    // always trust the InViewCOList, can even skip the expiration now

    if(auto p = getInViewCOPtr(nUID); p && g_monoServer->getCurrTick() <= p->RecordTime + 2 * 1000){
        fnOnOK(*p);
        return;
    }

    // can't find uid or expired
    // query the location and put to InViewCOList if applicable

    AMQueryLocation stAMQL;
    std::memset(&stAMQL, 0, sizeof(stAMQL));

    stAMQL.UID   = UID();
    stAMQL.MapID = MapID();

    m_actorPod->forward(nUID, {MPK_QUERYLOCATION, stAMQL}, [this, nUID, fnOnOK, fnOnError](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.Type()){
            case MPK_LOCATION:
                {
                    AMLocation stAML;
                    std::memcpy(&stAML, rstRMPK.Data(), sizeof(stAML));

                    // TODO when we get this response
                    // it's possible that the co has switched map or dead

                    COLocation stCOLoccation
                    {
                        stAML.UID,
                        stAML.MapID,
                        stAML.RecordTime,
                        stAML.X,
                        stAML.Y,
                        stAML.Direction
                    };

                    if((stAML.UID == nUID) && InView(stAML.MapID, stAML.X, stAML.Y)){
                        AddInViewCO(stCOLoccation);
                    }
                    else{
                        RemoveInViewCO(nUID);
                    }

                    fnOnOK(stCOLoccation);
                    return;
                }
            default:
                {
                    // TODO dangerous part here
                    // when nUID is not detached ActorPod::forward receives MPK_BADACTORPOD immedately
                    // then this branch get called, then m_inViewCOList get updated implicitly

                    RemoveInViewCO(nUID);
                    if(uidf::getUIDType(UID()) == UID_MON){
                        dynamic_cast<Monster *>(this)->RemoveTarget(nUID);
                    }

                    fnOnError();
                    return;
                }
        }
    });
}

void CharObject::AddOffenderDamage(uint64_t nUID, int nDamage)
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

void CharObject::DispatchOffenderExp()
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
            AMExp stAME;
            std::memset(&stAME, 0, sizeof(stAME));

            stAME.Exp = fnCalcExp(rstOffender.Damage);
            m_actorPod->forward(rstOffender.UID, {MPK_EXP, stAME});
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

void CharObject::DispatchHealth()
{
    AMUpdateHP stAMUHP;
    std::memset(&stAMUHP, 0, sizeof(stAMUHP));

    stAMUHP.UID   = UID();
    stAMUHP.MapID = MapID();
    stAMUHP.X     = X();
    stAMUHP.Y     = Y();
    stAMUHP.HP    = HP();
    stAMUHP.HPMax = HPMax();

    if(true
            && ActorPodValid()
            && m_map
            && m_map->ActorPodValid()){
        m_actorPod->forward(m_map->UID(), {MPK_UPDATEHP, stAMUHP});
    }
}

void CharObject::DispatchAttack(uint64_t nUID, int nDC)
{
    if(nUID && DCValid(nDC, true)){
        AMAttack stAMA;
        std::memset(&stAMA, 0, sizeof(stAMA));

        stAMA.UID   = UID();
        stAMA.MapID = MapID();

        stAMA.X = X();
        stAMA.Y = Y();

        auto stDamage = GetAttackDamage(nDC);
        stAMA.Type    = stDamage.Type;
        stAMA.Damage  = stDamage.Damage;
        stAMA.Element = stDamage.Element;

        for(size_t nIndex = 0; nIndex < sizeof(stAMA.Effect) / sizeof(stAMA.Effect[0]); ++nIndex){
            if(nIndex < stDamage.EffectArray.EffectLen()){
                stAMA.Effect[nIndex] = stDamage.EffectArray.Effect()[nIndex];
            }else{
                stAMA.Effect[nIndex] = EFF_NONE;
            }
        }
        m_actorPod->forward(nUID, {MPK_ATTACK, stAMA});
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
    AMAddCharObject stAMACO;
    std::memset(&stAMACO, 0, sizeof(stAMACO));

    stAMACO.type = UID_MON;
    stAMACO.x = x;
    stAMACO.y = y;
    stAMACO.mapID = m_map->ID();
    stAMACO.strictLoc = strictLoc;

    stAMACO.monster.monsterID = monsterID;
    stAMACO.monster.masterUID = UID();

    m_actorPod->forward(m_serviceCore->UID(), {MPK_ADDCHAROBJECT, stAMACO}, [](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.ID()){
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
    if(!m_map->ValidC(nX, nY)){
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

int CharObject::CheckPathGrid(int nX, int nY, uint32_t nTimeOut) const
{
    if(!m_map){
        throw fflerror("CO has no map associated");
    }

    if(!m_map->GetMir2xMapData().ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_map->GetMir2xMapData().Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    if(X() == nX && Y() == nY){
        return PathFind::OCCUPIED;
    }

    for(auto &rstLocation: m_inViewCOList){
        if(nTimeOut){
            if(g_monoServer->getCurrTick() > rstLocation.RecordTime + nTimeOut){
                continue;
            }
        }

        if(rstLocation.X == nX && rstLocation.Y == nY){
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

void CharObject::SetLastAction(int nAction)
{
    m_lastAction = nAction;
    m_lastActionTime = g_monoServer->getCurrTick();
}

void CharObject::AddInViewCO(const COLocation &rstCOLocation)
{
    if(!InView(rstCOLocation.MapID, rstCOLocation.X, rstCOLocation.Y)){
        return;
    }

    if(auto p = getInViewCOPtr(rstCOLocation.UID)){
        *p = rstCOLocation;
    }
    else{
        m_inViewCOList.push_back(rstCOLocation);
    }
    SortInViewCO();
}

void CharObject::foreachInViewCO(std::function<void(const COLocation &)> fnOnLoc)
{
    // TODO dangerous part
    // check comments in retrieveLocation

    // RemoveInViewCO() may get called in fnOnLoc
    // RemoveInViewCO() may get called in retrieveLocation

    scoped_alloc::svobuf_wrapper<uint64_t, 16> uidList;
    for(const auto &rstCOLoc: m_inViewCOList){
        uidList.c.push_back(rstCOLoc.UID);
    }

    for(size_t i = 0; i < uidList.c.size(); ++i){
        retrieveLocation(uidList.c.at(i), fnOnLoc);
    }
}

void CharObject::AddInViewCO(uint64_t nUID, uint32_t nMapID, int nX, int nY, int nDirection)
{
    AddInViewCO(COLocation(nUID, nMapID, g_monoServer->getCurrTick(), nX, nY, nDirection));
}

void CharObject::SortInViewCO()
{
    RemoveInViewCO(0);
    std::sort(m_inViewCOList.begin(), m_inViewCOList.end(), [this](const auto &rstLoc1, const auto &rstLoc2)
    {
        return mathf::LDistance2(rstLoc1.X, rstLoc1.Y, X(), Y()) < mathf::LDistance2(rstLoc2.X, rstLoc2.Y, X(), Y());
    });
}

void CharObject::RemoveInViewCO(uint64_t nUID)
{
    m_inViewCOList.erase(std::remove_if(m_inViewCOList.begin(), m_inViewCOList.end(), [this, nUID](const auto &rstCOLoc)
    {
        return rstCOLoc.UID == nUID || !InView(rstCOLoc.MapID, rstCOLoc.X, rstCOLoc.Y);
    }), m_inViewCOList.end());

    if((m_inViewCOList.size() < m_inViewCOList.capacity() / 2) && (m_inViewCOList.capacity() > 20)){
        m_inViewCOList.shrink_to_fit();
    }

    if(uidf::getUIDType(UID()) == UID_MON){
        dynamic_cast<Monster *>(this)->RemoveTarget(nUID);
    }
}

bool CharObject::InView(uint32_t nMapID, int nX, int nY) const
{
    return m_map->In(nMapID, nX, nY) && mathf::LDistance2(X(), Y(), nX, nY) <= 10 * 10;
}

COLocation &CharObject::GetInViewCORef(uint64_t nUID)
{
    if(auto p = getInViewCOPtr(nUID)){
        return *p;
    }
    throw fflerror("can't find UID in InViewCOList: %" PRIu64, nUID);
}

COLocation *CharObject::getInViewCOPtr(uint64_t nUID)
{
    for(auto &rstCOLoc: m_inViewCOList){
        if(rstCOLoc.UID == nUID){
            return &rstCOLoc;
        }
    }
    return nullptr;
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

void CharObject::QueryFinalMaster(uint64_t nUID, std::function<void(uint64_t)> fnOp)
{
    if(!nUID){
        throw fflerror("invalid zero UID");
    }

    if(uidf::getUIDType(nUID) != UID_MON){
        throw fflerror("%s can't have master", uidf::getUIDTypeString(nUID));
    }

    auto fnQuery = [this, fnOp](uint64_t nQueryUID)
    {
        m_actorPod->forward(nQueryUID, MPK_QUERYFINALMASTER, [this, nQueryUID, fnOp](const MessagePack &rstRMPK)
        {
            switch(rstRMPK.Type()){
                case MPK_UID:
                    {
                        AMUID stAMUID;
                        std::memcpy(&stAMUID, rstRMPK.Data(), sizeof(stAMUID));

                        fnOp(stAMUID.UID);
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
                                throw fflerror("invalid master type: %s", uidf::getUIDTypeString(nMasterUID));
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
                throw fflerror("%s can't query self for final master", uidf::getUIDTypeString(UID()));
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
