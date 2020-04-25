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
#include "motion.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "svobuffer.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"
#include "eventtaskhub.hpp"

extern MonoServer *g_MonoServer;

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

                    g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", MaxStep());
                    return 10000.00;
              }

              int nDistance2 = mathf::LDistance2(nSrcX, nSrcY, nDstX, nDstY);
              if(true
                      && nDistance2 != 1
                      && nDistance2 != 2
                      && nDistance2 != MaxStep() * MaxStep()
                      && nDistance2 != MaxStep() * MaxStep() * 2){

                  g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid step checked: (%d, %d) -> (%d, %d)", nSrcX, nSrcY, nDstX, nDstY);
                  return 10000.00;
              }
          }

          return m_CO->OneStepCost(this, m_CheckCO, nSrcX, nSrcY, nDstX, nDstY);
      }, pCO->MaxStep())
    , m_CO(pCO)
    , m_CheckCO(nCheckCO)
    , m_Cache()
{
    if(!m_CO){
        g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid argument: CO = %p, CheckCO = %d", m_CO, m_CheckCO);
    }

    switch(m_CheckCO){
        case 0:
        case 1:
        case 2:
            {
                break;
            }
        default:
            {
                g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid argument: CO = %p, CheckCO = %d", m_CO, m_CheckCO);
                break;
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
                g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid MaxStep provided: %d, should be (1, 2, 3)", m_CO->MaxStep());
                break;
            }
    }
}

int CharObject::COPathFinder::GetGrid(int nX, int nY) const
{
    if(!m_CO->GetServerMap()->ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    uint32_t nKey = ((uint32_t)(nX) << 16) | (uint32_t)(nY);
    if(auto p = m_Cache.find(nKey); p != m_Cache.end()){
        return p->second;
    }

    auto nGrid = m_CO->CheckPathGrid(nX, nY);
    m_Cache[nKey] = nGrid;
    return nGrid;
}

CharObject::CharObject(ServiceCore *pServiceCore,
        ServerMap                  *pServerMap,
        uint64_t                    nUID,
        int                         nMapX,
        int                         nMapY,
        int                         nDirection)
    : ServerObject(nUID)
    , m_ServiceCore(pServiceCore)
    , m_Map(pServerMap)
    , m_InViewCOList()
    , m_X(nMapX)
    , m_Y(nMapY)
    , m_Direction(nDirection)
    , m_HP(0)
    , m_HPMax(0)
    , m_MP(0)
    , m_MPMax(0)
    , m_MoveLock(false)
    , m_AttackLock(false)
    , m_LastMoveTime(0)
    , m_LastAttackTime(0)
    , m_LastAction(ACTION_NONE)
    , m_LastActionTime(0)
    , m_Target()
    , m_OffenderList()
    , m_Ability()
    , m_WAbility()
    , m_AddAbility()
{
    condcheck(m_Map);
    m_StateHook.Install("RemoveDeadUIDLocation", [this, nLastCheckTick = (uint32_t)(0)]() mutable -> bool
    {
        if(auto nCurrCheckTick = g_MonoServer->getCurrTick(); nLastCheckTick + 5000 < nCurrCheckTick){
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

uint64_t CharObject::Activate()
{
    if(auto nUID = ServerObject::Activate(); nUID){
        DispatchAction(ActionSpawn(X(), Y(), Direction()));
        return nUID;
    }
    return 0;
}

void CharObject::ReportAction(uint64_t, const ActionNode &)
{
}

void CharObject::DispatchAction(const ActionNode &rstAction)
{
    // should check to avoid dead CO call this function
    // this would cause zombies

    if(!ActorPodValid()){
        throw fflerror("Can't dispatch action: %s", rstAction.ActionName());
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
                m_actorPod->forward(m_Map->UID(), {MPK_ACTION, stAMA});
                return;
            }
        default:
            {
                ForeachInViewCO([this, stAMA](const COLocation &rstLocation)
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

bool CharObject::RequestMove(int nX, int nY, int nSpeed, bool bAllowHalfMove, bool bRemoveMonster, std::function<void()> fnOnMoveOK, std::function<void()> fnOnMoveError)
{
    if(!CanMove()){
        fnOnMoveError();
        return false;
    }

    if(EstimateHop(nX, nY) != 1){
        fnOnMoveError();
        return false;
    }

    if(bRemoveMonster){
        throw fflerror("RemoveMonster in RequestMove() not implemented yet");
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
                            fnOnMoveError();
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
                    if(!bAllowHalfMove){
                        fnOnMoveError();
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
    stAMTM.AllowHalfMove = bAllowHalfMove;
    stAMTM.RemoveMonster = bRemoveMonster;

    m_MoveLock = true;
    return m_actorPod->forward(MapUID(), {MPK_TRYMOVE, stAMTM}, [this, nX, nY, nSpeed, fnOnMoveOK, fnOnMoveError](const MessagePack &rstMPK)
    {
        if(!m_MoveLock){
            throw fflerror("MoveLock released before map responds: ClassName = %s", UIDName());
        }
        m_MoveLock = false;

        // handle move, CO may be dead
        // need to check if current CO can move

        switch(rstMPK.Type()){
            case MPK_MOVEOK:
                {
                    AMMoveOK stAMMOK;
                    std::memcpy(&stAMMOK, rstMPK.Data(), sizeof(stAMMOK));

                    // since we may allow half move
                    // servermap permitted dst may not be (nX, nY)

                    if(!m_Map->ValidC(stAMMOK.EndX, stAMMOK.EndY)){
                        throw fflerror("Map returns invalid destination: (%" PRIu64 ", %d, %d)", m_Map->UID(), stAMMOK.EndX, stAMMOK.EndY);
                    }

                    if(!CanMove()){
                        m_actorPod->forward(rstMPK.From(), MPK_ERROR, rstMPK.ID());
                        fnOnMoveError();
                        return;
                    }

                    auto nOldX = m_X;
                    auto nOldY = m_Y;

                    m_X = stAMMOK.EndX;
                    m_Y = stAMMOK.EndY;

                    m_Direction = PathFind::GetDirection(nOldX, nOldY, X(), Y());
                    m_LastMoveTime = g_MonoServer->getCurrTick();

                    m_actorPod->forward(rstMPK.From(), MPK_OK, rstMPK.ID());
                    DispatchAction(ActionMove(nOldX, nOldY, X(), Y(), nSpeed, Horse()));
                    SortInViewCO();

                    fnOnMoveOK();
                    return;
                }
            default:
                {
                    fnOnMoveError();
                    return;
                }
        }
    });
}

bool CharObject::RequestSpaceMove(uint32_t nMapID, int nX, int nY, bool bStrictMove, std::function<void()> fnOnMoveOK, std::function<void()> fnOnMoveError)
{
    if(!(uidf::getMapUID(nMapID) && (nX >= 0) && (nY >= 0))){
        throw fflerror("Invalid map destination: (%lld, %d, %d)", toLLD(nMapID), nX, nY);
    }

    if(!CanMove()){
        fnOnMoveError();
        return false;
    }

    AMTrySpaceMove stAMTSM;
    std::memset(&stAMTSM, 0, sizeof(stAMTSM));

    stAMTSM.UID        = UID();
    stAMTSM.X          = nX;
    stAMTSM.Y          = nY;
    stAMTSM.StrictMove = bStrictMove;

    m_MoveLock = true;
    return m_actorPod->forward(uidf::getMapUID(nMapID), {MPK_TRYSPACEMOVE, stAMTSM}, [this, fnOnMoveOK, fnOnMoveError](const MessagePack &rstRMPK)
    {
        if(!m_MoveLock){
            throw std::runtime_error(str_fflprintf("MoveLock released before map responds: ClassName = %s", UIDName()));
        }
        m_MoveLock = false;

        // handle move, CO can be dead already
        // check if current CO can move even we checked before

        switch(rstRMPK.Type()){
            case MPK_SPACEMOVEOK:
                {
                    // need to leave src map
                    // dst map already says OK for current move

                    if(!CanMove()){
                        m_actorPod->forward(rstRMPK.From(), MPK_ERROR, rstRMPK.ID());
                        fnOnMoveError();
                        return;
                    }

                    AMTryLeave stAMTL;
                    std::memset(&stAMTL, 0, sizeof(stAMTL));

                    stAMTL.UID   = UID();
                    stAMTL.MapID = m_Map->ID();
                    stAMTL.X     = X();
                    stAMTL.Y     = Y();

                    m_MoveLock = true;
                    m_actorPod->forward(m_Map->UID(), {MPK_TRYLEAVE, stAMTL}, [this, rstRMPK, fnOnMoveOK, fnOnMoveError](const MessagePack &rstLeaveRMPK)
                    {
                        if(!m_MoveLock){
                            throw std::runtime_error(str_fflprintf("MoveLock released before map responds: ClassName = %s", UIDName()));
                        }
                        m_MoveLock = false;

                        switch(rstLeaveRMPK.Type()){
                            case MPK_OK:
                                {
                                    AMSpaceMoveOK stAMSMOK;
                                    std::memcpy(&stAMSMOK, rstRMPK.Data(), sizeof(stAMSMOK));

                                    if(!CanMove()){
                                        m_actorPod->forward(rstRMPK.From(), MPK_ERROR, rstRMPK.ID());
                                        fnOnMoveError();
                                        return;
                                    }
                                    
                                    // dispatch space move part 1 on old map
                                    DispatchAction(ActionSpaceMove1(X(), Y(), Direction()));

                                    // setup new map
                                    // don't use the requested location
                                    m_X   = stAMSMOK.X;
                                    m_Y   = stAMSMOK.Y;
                                    m_Map = (ServerMap *)(stAMSMOK.Ptr);

                                    m_LastMoveTime = g_MonoServer->getCurrTick();
                                    m_actorPod->forward(rstRMPK.From(), MPK_OK, rstRMPK.ID());

                                    //  dispatch/report space move part 2 on new map
                                    DispatchAction(ActionSpaceMove2(X(), Y(), Direction()));
                                    ReportAction(UID(), ActionSpaceMove2(X(), Y(), Direction()));

                                    fnOnMoveOK();
                                    return;
                                }
                            default:
                                {
                                    m_actorPod->forward(rstRMPK.From(), MPK_ERROR, rstRMPK.ID());
                                    fnOnMoveError();
                                    return;
                                }
                        }
                    });
                    break;
                }
            default:
                {
                    fnOnMoveError();
                    break;
                }
        }
    });
}

bool CharObject::CanMove()
{
    switch(GetState(STATE_DEAD)){
        case 0:
            {
                return !m_MoveLock;
            }
        default:
            {
                return false;
            }
    }
}

bool CharObject::CanAct()
{
    switch(m_LastAction){
        case ACTION_SPAWN:
            {
                switch(uidf::getUIDType(UID())){
                    case UID_MON:
                        {
                            switch(uidf::getMonsterID(UID())){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        return g_MonoServer->getCurrTick() > m_LastActionTime + 600;
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
                return g_MonoServer->getCurrTick() > m_LastActionTime + 600;
            }
        case ACTION_MOVE:
            {
                return g_MonoServer->getCurrTick() > m_LastActionTime + 700;
            }
        default:
            {
                break;
            }
    }
    return true;
}

bool CharObject::CanAttack()
{
    switch(GetState(STATE_DEAD)){
        case 0:
            {
                return !m_AttackLock;
            }
        default:
            {
                return false;
            }
    }
}

void CharObject::RetrieveLocation(uint64_t nUID, std::function<void(const COLocation &)> fnOnOK, std::function<void()> fnOnError)
{
    if(!nUID){
        throw std::invalid_argument(str_ffl() + ": Query location with zero UID");
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_ffl() + ": Query UID to CO itself: " + uidf::getUIDString(nUID));
    }

    // CO dispatches location changes automatically
    // always trust the InViewCOList, can even skip the expiration now

    if(auto p = GetInViewCOPtr(nUID); p && g_MonoServer->getCurrTick() <= p->RecordTime + 2 * 1000){
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
                    }else{
                        RemoveInViewCO(nUID);
                    }

                    fnOnOK(stCOLoccation);
                    return;
                }
            default:
                {
                    // TODO dangerous part here
                    // when nUID is not detached ActorPod::forward receives MPK_BADACTORPOD immedately
                    // then this branch get called, then m_InViewCOList get updated implicitly

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
        throw std::invalid_argument(str_fflprintf(": Offender with zero UID"));
    }

    if(nDamage < 0){
        throw std::invalid_argument(str_fflprintf(": Invalid offender damage: %d", nDamage));
    }

    for(auto p = m_OffenderList.begin(); p != m_OffenderList.end(); ++p){
        if(p->UID == nUID){
            p->Damage += nDamage;
            p->ActiveTime = g_MonoServer->getCurrTick();
            return;
        }
    }
    m_OffenderList.emplace_back(nUID, nDamage, g_MonoServer->getCurrTick());
}

void CharObject::DispatchOffenderExp()
{
    for(auto p = m_OffenderList.begin(); p != m_OffenderList.end();){
        if(g_MonoServer->getCurrTick() >= p->ActiveTime + 2 * 60 * 3600){
            p = m_OffenderList.erase(p);
        }else{
            p++;
        }
    }

    if(m_OffenderList.empty()){
        return;
    }

    auto fnCalcExp = [this](int nDamage) -> int
    {
        return nDamage * m_OffenderList.size();
    };

    for(const auto &rstOffender: m_OffenderList){
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
    if(m_Map){
        if(nMaxDistance > 0){
            int nLastX = -1;
            int nLastY = -1;
            int nLastD =  0;
            for(int nDistance = 1; nDistance <= nMaxDistance; ++nDistance){
                int nX = -1;
                int nY = -1;
                if(true
                        && NextLocation(&nX, &nY, nDirection, nDistance)
                        && m_Map->GroundValid(nX, nY)){
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
            && m_Map
            && m_Map->ActorPodValid()){
        m_actorPod->forward(m_Map->UID(), {MPK_UPDATEHP, stAMUHP});
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

void CharObject::AddMonster(uint32_t nMonsterID, int nX, int nY, bool bStrictLoc)
{
    AMAddCharObject stAMACO;
    std::memset(&stAMACO, 0, sizeof(stAMACO));

    stAMACO.Type = TYPE_MONSTER;

    stAMACO.Common.MapID     = m_Map->ID();
    stAMACO.Common.X         = nX;
    stAMACO.Common.Y         = nY;
    stAMACO.Common.StrictLoc = bStrictLoc;

    stAMACO.Monster.MonsterID = nMonsterID;
    stAMACO.Monster.MasterUID = UID();

    m_actorPod->forward(m_ServiceCore->UID(), {MPK_ADDCHAROBJECT, stAMACO}, [](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.ID()){
            default:
                {
                    break;
                }
        }
    });
}

int CharObject::EstimateHop(int nX, int nY)
{
    condcheck(m_Map);
    if(!m_Map->ValidC(nX, nY)){
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
    condcheck(m_Map);
    if(!m_Map->GetMir2xMapData().ValidC(nX, nY)){
        return PathFind::INVALID;
    }

    if(!m_Map->GetMir2xMapData().Cell(nX, nY).CanThrough()){
        return PathFind::OBSTACLE;
    }

    if(X() == nX && Y() == nY){
        return PathFind::OCCUPIED;
    }

    for(auto &rstLocation: m_InViewCOList){
        if(nTimeOut){
            if(g_MonoServer->getCurrTick() > rstLocation.RecordTime + nTimeOut){
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

    int nX0 = X();
    int nY0 = Y();

    std::array<PathFind::PathNode, 3> stvPathNode
    {{
        {-1, -1},
        {-1, -1},
        {-1, -1},
    }};

    int nDX = ((nX > nX0) - (nX < nX0));
    int nDY = ((nY > nY0) - (nY < nY0));

    switch(std::abs(nDX) + std::abs(nDY)){
        case 1:
            {
                if(nDY){
                    stvPathNode[0] = {nX0        , nY0 + nDY * nDLen};
                    stvPathNode[1] = {nX0 - nDLen, nY0 + nDY * nDLen};
                    stvPathNode[2] = {nX0 + nDLen, nY0 + nDY * nDLen};
                }else{
                    stvPathNode[0] = {nX0 + nDX * nDLen, nY0        };
                    stvPathNode[1] = {nX0 + nDX * nDLen, nY0 - nDLen};
                    stvPathNode[2] = {nX0 + nDX * nDLen, nY0 + nDLen};
                }
                break;
            }
        case 2:
            {
                stvPathNode[0] = {nX0 + nDX * nDLen, nY0 + nDY * nDLen};
                stvPathNode[1] = {nX0              , nY0 + nDY * nDLen};
                stvPathNode[2] = {nX0 + nDX * nDLen, nY0              };
                break;
            }
        default:
            {
                break;
            }
    }
    return stvPathNode;
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
                g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid argument: COPathFinder = %p, CheckCO = %d", pFinder, nCheckCO);
                break;
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
        switch(auto nGrid = pFinder ? pFinder->GetGrid(nCurrX, nCurrY) : CheckPathGrid(nCurrX, nCurrY)){
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
                    g_MonoServer->addLog(LOGTYPE_FATAL, "Invalid grid provided: %d at (%d, %d)", nGrid, nCurrX, nCurrY);
                    break;
                }
        }
    }

    return 1.00 + nMaxIndex * 0.10 + fExtraPen;
}

void CharObject::SetLastAction(int nAction)
{
    m_LastAction = nAction;
    m_LastActionTime = g_MonoServer->getCurrTick();
}

void CharObject::AddInViewCO(const COLocation &rstCOLocation)
{
    if(!InView(rstCOLocation.MapID, rstCOLocation.X, rstCOLocation.Y)){
        return;
    }

    if(auto p = GetInViewCOPtr(rstCOLocation.UID)){
        *p = rstCOLocation;
    }else{
        m_InViewCOList.push_back(rstCOLocation);
    }
    SortInViewCO();
}

void CharObject::ForeachInViewCO(std::function<void(const COLocation &)> fnOnLoc)
{
    // TODO dangerous part
    // check comments in RetrieveLocation

    // RemoveInViewCO() may get called in fnOnLoc
    // RemoveInViewCO() may get called in RetrieveLocation

    svo_buffer<uint64_t, 4> stvUIDList;
    for(const auto &rstCOLoc: m_InViewCOList){
        stvUIDList.push_back(rstCOLoc.UID);
    }

    for(size_t nIndex = 0; nIndex < stvUIDList.size(); ++nIndex){
        RetrieveLocation(stvUIDList.at(nIndex), fnOnLoc);
    }
}

void CharObject::AddInViewCO(uint64_t nUID, uint32_t nMapID, int nX, int nY, int nDirection)
{
    AddInViewCO(COLocation(nUID, nMapID, g_MonoServer->getCurrTick(), nX, nY, nDirection));
}

void CharObject::SortInViewCO()
{
    RemoveInViewCO(0);
    std::sort(m_InViewCOList.begin(), m_InViewCOList.end(), [this](const auto &rstLoc1, const auto &rstLoc2)
    {
        return mathf::LDistance2(rstLoc1.X, rstLoc1.Y, X(), Y()) < mathf::LDistance2(rstLoc2.X, rstLoc2.Y, X(), Y());
    });
}

void CharObject::RemoveInViewCO(uint64_t nUID)
{
    m_InViewCOList.erase(std::remove_if(m_InViewCOList.begin(), m_InViewCOList.end(), [this, nUID](const auto &rstCOLoc)
    {
        return rstCOLoc.UID == nUID || !InView(rstCOLoc.MapID, rstCOLoc.X, rstCOLoc.Y);
    }), m_InViewCOList.end());

    if((m_InViewCOList.size() < m_InViewCOList.capacity() / 2) && (m_InViewCOList.capacity() > 20)){
        m_InViewCOList.shrink_to_fit();
    }

    if(uidf::getUIDType(UID()) == UID_MON){
        dynamic_cast<Monster *>(this)->RemoveTarget(nUID);
    }
}

bool CharObject::InView(uint32_t nMapID, int nX, int nY) const
{
    return m_Map->In(nMapID, nX, nY) && mathf::LDistance2(X(), Y(), nX, nY) <= 10 * 10;
}

COLocation &CharObject::GetInViewCORef(uint64_t nUID)
{
    if(auto p = GetInViewCOPtr(nUID)){
        return *p;
    }
    throw std::runtime_error(str_fflprintf(": Can't find UID in InViewCOList: %" PRIu64, nUID));
}

COLocation *CharObject::GetInViewCOPtr(uint64_t nUID)
{
    for(auto &rstCOLoc: m_InViewCOList){
        if(rstCOLoc.UID == nUID){
            return &rstCOLoc;
        }
    }
    return nullptr;
}

bool CharObject::IsOffender(uint64_t nUID)
{
    for(auto &rstOffender: m_OffenderList){
        if(rstOffender.UID == nUID){
            return true;
        }
    }
    return false;
}

void CharObject::QueryFinalMaster(uint64_t nUID, std::function<void(uint64_t)> fnOp)
{
    if(!nUID){
        throw fflerror("Invalid zero UID");
    }

    if(uidf::getUIDType(nUID) != UID_MON){
        throw std::invalid_argument(str_fflprintf(": %s can't have master", uidf::getUIDTypeString(nUID)));
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
                        if(IsMonster() && (nQueryUID == dynamic_cast<Monster *>(this)->MasterUID())){
                            GoDie();
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

                if(auto nMasterUID = dynamic_cast<Monster *>(this)->MasterUID()){
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
                                throw std::runtime_error(str_fflprintf(": Invalid master type: %s", uidf::getUIDTypeString(nMasterUID)));
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
                throw std::invalid_argument(str_fflprintf(": %s can't query self for final master", uidf::getUIDTypeString(UID())));
            }
    }
}

bool CharObject::IsPlayer() const
{
    return uidf::getUIDType(UID()) == UID_PLY;
}

bool CharObject::IsMonster() const
{
    return uidf::getUIDType(UID()) == UID_MON;
}
