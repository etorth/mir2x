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
#include "motion.hpp"
#include "player.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "actorpod.hpp"
#include "condcheck.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"
#include "eventtaskhub.hpp"

CharObject::CharObject(ServiceCore *pServiceCore,
        ServerMap                  *pServerMap,
        uint32_t                    nUID,
        int                         nMapX,
        int                         nMapY,
        int                         nDirection,
        uint8_t                     nLifeState)
    : ActiveObject(nUID)
    , m_ServiceCore(pServiceCore)
    , m_Map(pServerMap)
    , m_LocationList()
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
    , m_HitterUIDRecord()
    , m_TargetQueue()
    , m_Ability()
    , m_WAbility()
    , m_AddAbility()
{
    condcheck(m_Map);
    SetState(STATE_LIFECYCLE, nLifeState);
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

Theron::Address CharObject::Activate()
{
    auto stAddress = ActiveObject::Activate();
    if(ActorPodValid()){
        DispatchAction(ActionStand(X(), Y(), Direction()));
    }

    AddTick();
    return stAddress;
}

void CharObject::ReportAction(uint32_t, const ActionNode &)
{
}

void CharObject::DispatchAction(const ActionNode &rstAction)
{
    // should check to avoid dead CO call this function
    // this would cause zombies

    if(true
            && ActorPodValid()
            && m_Map
            && m_Map->ActorPodValid()){

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

        m_ActorPod->Forward({MPK_ACTION, stAMA}, m_Map->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't dispatch action: %p", &rstAction);
}

bool CharObject::RequestMove(int nX, int nY, int nSpeed, bool bAllowHalfMove, std::function<void()> fnOnMoveOK, std::function<void()> fnOnMoveError)
{
    if(true
            && CanMove()
            && EstimateHop(nX, nY) == 1){

        AMTryMove stAMTM;
        std::memset(&stAMTM, 0, sizeof(stAMTM));

        stAMTM.UID           = UID();
        stAMTM.MapID         = MapID();
        stAMTM.X             = X();
        stAMTM.Y             = Y();
        stAMTM.EndX          = nX;
        stAMTM.EndY          = nY;
        stAMTM.AllowHalfMove = bAllowHalfMove;

        auto fnOnResp = [this, nX, nY, nSpeed, bAllowHalfMove, fnOnMoveOK, fnOnMoveError](const MessagePack &rstMPK, const Theron::Address &rstAddr)
        {
            if(!m_MoveLock){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "MoveLock released before server responds: ClassName = %s", ClassName());
                g_MonoServer->Restart();
            }

            // 1. release the move lock no matter what kind of message we get
            m_MoveLock = false;

            // 2. handle move, CO may be dead
            //    need to check if current CO can move

            auto nDir = PathFind::GetDirection(X(), Y(), nX, nY);
            switch(rstMPK.Type()){
                case MPK_MOVEOK:
                    {
                        AMMoveOK stAMMOK;
                        std::memcpy(&stAMMOK, rstMPK.Data(), sizeof(stAMMOK));

                        // since we may allow half move
                        // then servermap permitted dst may not be (nX, nY)

                        if(true
                                && CanMove()
                                && m_Map->ValidC(stAMMOK.EndX, stAMMOK.EndY)){

                            auto nOldX = m_X;
                            auto nOldY = m_Y;

                            m_X = stAMMOK.EndX;
                            m_Y = stAMMOK.EndY;

                            m_Direction = nDir;

                            extern MonoServer *g_MonoServer;
                            m_LastMoveTime = g_MonoServer->GetTimeTick();

                            m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());
                            DispatchAction(ActionMove(nOldX, nOldY, X(), Y(), nSpeed, Horse()));

                            if(fnOnMoveOK){
                                fnOnMoveOK();
                            }
                        }else{
                            m_ActorPod->Forward(MPK_ERROR, rstAddr, rstMPK.ID());
                            if(fnOnMoveError){
                                fnOnMoveError();
                            }
                        }

                        break;
                    }
                case MPK_ERROR:
                    {
                        // should add a new function: CanTurn()
                        // for stone state we can't even make a turn

                        if(CanMove()){
                            m_Direction = nDir;
                            DispatchAction(ActionStand(X(), Y(), Direction()));
                        }

                        if(fnOnMoveError){
                            fnOnMoveError();
                        }

                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        };

        m_MoveLock = true;
        return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_Map->GetAddress(), fnOnResp);
    }
    return false;
}

bool CharObject::RequestSpaceMove(uint32_t nMapID, int nX, int nY, bool bStrictMove, std::function<void()> fnOnMoveOK, std::function<void()> fnOnMoveError)
{
    if(true
            && nMapID
            && CanMove()){

        // lambda function to do space move
        // accept map actor address, can be current map or another map

        auto fnCOSpaceMove = [this, nMapID, nX, nY, bStrictMove, fnOnMoveOK, fnOnMoveError](const Theron::Address &rstMapAddress) -> bool
        {
            AMTrySpaceMove stAMTSM;
            stAMTSM.UID        = UID();
            stAMTSM.X          = nX;
            stAMTSM.Y          = nY;
            stAMTSM.StrictMove = bStrictMove;

            auto fnMapResp = [this, nX, nY, fnOnMoveOK, fnOnMoveError](const MessagePack &rstRMPK, const Theron::Address &rstRAddress)
            {
                // 1. check if lock released
                //    shouldn't release before get service core's response

                if(!m_MoveLock){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "MoveLock released before service core responds: ClassName = %s", ClassName());
                    g_MonoServer->Restart();
                }

                m_MoveLock = false;

                // 2. handle move
                //    need to check if current CO can move even we checked before

                switch(rstRMPK.Type()){
                    case MPK_SPACEMOVEOK:
                        {
                            // need to leave src map
                            // dst map already says OK for current move

                            // was to decleare a new function CharObject::LeaveMap(fnLeaveOK)
                            // but it could cause the issue that m_Map may stay invalid if after MPK_TRYLEAVE succeeds but
                            // fnLeaveOK doesn't provide a new map pointer

                            if(CanMove()){

                                AMTryLeave stAMTL;
                                stAMTL.UID   = UID();
                                stAMTL.MapID = m_Map->ID();
                                stAMTL.X     = X();
                                stAMTL.Y     = Y();

                                auto fnLeaveOP = [this, rstRMPK, rstRAddress, nX, nY, fnOnMoveOK, fnOnMoveError](const MessagePack &rstLeaveRMPK, const Theron::Address &)
                                {
                                    m_MoveLock = false;

                                    switch(rstLeaveRMPK.Type()){
                                        case MPK_OK:
                                            {
                                                AMSpaceMoveOK stAMSMOK;
                                                std::memcpy(&stAMSMOK, rstRMPK.Data(), sizeof(stAMSMOK));

                                                if(CanMove()){

                                                    // 1. dispatch space move part 1 on old map
                                                    DispatchAction(ActionSpaceMove1(X(), Y(), Direction()));

                                                    // 2. setup new map
                                                    m_X   = nX;
                                                    m_Y   = nY;
                                                    m_Map = (ServerMap *)(stAMSMOK.Data);

                                                    extern MonoServer *g_MonoServer;
                                                    m_LastMoveTime = g_MonoServer->GetTimeTick();
                                                    m_ActorPod->Forward(MPK_OK, rstRAddress, rstRMPK.ID());

                                                    // 3. dispatch/report space move part 2 on new map
                                                    DispatchAction(ActionSpaceMove2(X(), Y(), Direction()));
                                                    ReportAction(UID(), ActionSpaceMove2(X(), Y(), Direction()));

                                                    if(fnOnMoveOK){
                                                        fnOnMoveOK();
                                                    }
                                                }else{
                                                    m_ActorPod->Forward(MPK_ERROR, rstRAddress, rstRMPK.ID());
                                                    if(fnOnMoveError){
                                                        fnOnMoveError();
                                                    }
                                                }

                                                break;
                                            }
                                        default:
                                            {
                                                if(fnOnMoveError){
                                                    fnOnMoveError();
                                                }
                                                break;
                                            }
                                    }
                                };

                                m_MoveLock = true;
                                m_ActorPod->Forward({MPK_TRYLEAVE, stAMTL}, m_Map->GetAddress(), fnLeaveOP);

                            }else{

                                // CanMove() check fails
                                // even we get MPK_SPACEMOVEOK

                                m_ActorPod->Forward(MPK_ERROR, rstRAddress, rstRMPK.ID());
                                if(fnOnMoveError){
                                    fnOnMoveError();
                                }
                            }
                            break;
                        }
                    default:
                        {
                            if(fnOnMoveError){
                                fnOnMoveError();
                            }
                            break;
                        }
                }
            };

            m_MoveLock = true;
            return m_ActorPod->Forward({MPK_TRYSPACEMOVE, stAMTSM}, rstMapAddress, fnMapResp);
        };

        if(nMapID == m_Map->ID()){
            return fnCOSpaceMove(m_Map->GetAddress());
        }else{
            AMQueryMapUID stAMQMUID;
            stAMQMUID.MapID = nMapID;

            auto fnOnMapUID = [fnCOSpaceMove, fnOnMoveError](const MessagePack &rstRMPK, const Theron::Address &)
            {
                switch(rstRMPK.Type()){
                    case MPK_UID:
                        {
                            AMUID stAMUID;
                            std::memcpy(&stAMUID, rstRMPK.Data(), sizeof(stAMUID));

                            extern MonoServer *g_MonoServer;
                            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(stAMUID.UID)){
                                fnCOSpaceMove(stUIDRecord.GetAddress());
                            }else{
                                if(fnOnMoveError){
                                    fnOnMoveError();
                                }
                            }
                            break;
                        }
                    default:
                        {
                            if(fnOnMoveError){
                                fnOnMoveError();
                            }
                            break;
                        }
                }
            };
            return m_ActorPod->Forward({MPK_QUERYMAPUID, stAMQMUID}, m_ServiceCore->GetAddress(), fnOnMapUID);
        }
    }
    return false;
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

bool CharObject::RetrieveLocation(uint32_t nUID, std::function<void(const COLocation &)> fnOnLocationOK)
{
    if(nUID){

        // current the UID is valid
        // lambda captured fnOnLocationOK then can be delayed by one step
        // 1.     valid cache : call fnOnLocationOK
        // 2. not valid cache : call fnOnLocationOK after refresh
        auto fnQueryLocation = [this, nUID, fnOnLocationOK]() -> bool
        {
            AMQueryLocation stAMQL;
            std::memset(&stAMQL, 0, sizeof(stAMQL));

            stAMQL.UID   = UID();
            stAMQL.MapID = MapID();

            auto fnOnResp = [this, nUID, fnOnLocationOK](const MessagePack &rstRMPK, const Theron::Address &)
            {
                switch(rstRMPK.Type()){
                    case MPK_LOCATION:
                        {
                            AMLocation stAML;
                            std::memcpy(&stAML, rstRMPK.Data(), sizeof(stAML));

                            // TODO
                            // when we get this response
                            // it's possible that the co has switched map

                            if(true
                                    && m_Map
                                    && m_Map->In(stAML.MapID, stAML.X, stAML.Y)){

                                m_LocationList[nUID] = COLocation
                                {
                                    UID(),
                                    MapID(),
                                    stAML.RecordTime,
                                    stAML.X,
                                    stAML.Y,
                                    stAML.Direction
                                };

                                if(fnOnLocationOK){
                                    fnOnLocationOK(m_LocationList[nUID]);
                                }
                            }else{
                                m_LocationList.erase(nUID);
                            }
                            break;
                        }
                    default:
                        {
                            m_LocationList.erase(nUID);
                            break;
                        }
                }
            };

            extern MonoServer *g_MonoServer;
            if(auto stRecord = g_MonoServer->GetUIDRecord(nUID)){
                return m_ActorPod->Forward({MPK_QUERYLOCATION, stAMQL}, stRecord.GetAddress(), fnOnResp);
            }

            return false;
        };

        // no entry found
        // do query and invocation, delay fnOnLocationOK by one step
        if(m_LocationList.find(nUID) == m_LocationList.end()){
            return fnQueryLocation();
        }

        // we find an entry
        // valid the entry, could be expired
        extern MonoServer *g_MonoServer;
        auto &rstRecord = m_LocationList[nUID];
        if(true
                && m_Map
                && m_Map->In(rstRecord.MapID, rstRecord.X, rstRecord.Y)
                && g_MonoServer->GetTimeTick() <= rstRecord.RecordTime + 2 * 1000){

            if(fnOnLocationOK){
                fnOnLocationOK(rstRecord);
            }
            return true;
        }

        // do query new location
        // found record is out of time
        return fnQueryLocation();
    }
    return false;
}

bool CharObject::AddHitterUID(uint32_t nUID, int nDamage)
{
    if(nUID){
        for(auto rstRecord: m_HitterUIDRecord){
            if(rstRecord.UID == nUID){
                rstRecord.Damage += std::max<int>(0, nDamage);
                extern MonoServer *g_MonoServer;
                rstRecord.ActiveTime = g_MonoServer->GetTimeTick();
                return true;
            }
        }

        // new entry
        extern MonoServer *g_MonoServer;
        m_HitterUIDRecord.emplace_back(nUID, std::max<int>(0, nDamage), g_MonoServer->GetTimeTick());
        return true;
    }
    return false;
}

bool CharObject::DispatchHitterExp()
{
    extern MonoServer *g_MonoServer;
    auto nNowTick = g_MonoServer->GetTimeTick();
    for(size_t nIndex = 0; nIndex < m_HitterUIDRecord.size();){
        if(true
                && m_HitterUIDRecord[nIndex].UID
                && m_HitterUIDRecord[nIndex].ActiveTime + 2 * 60 * 1000 >= nNowTick){

            extern MonoServer *g_MonoServer;
            if(auto stUIDRecord = g_MonoServer->GetUIDRecord(m_HitterUIDRecord[nIndex].UID)){
                if(auto nType = UIDFunc::GetUIDType(stUIDRecord.UID()); nType == UID_MON || nType == UID_PLY){
                    // record is valid
                    // record is not time-out
                    // record is monster or player
                    nIndex++;
                    continue;
                }
            }
        }

        // remove it
        // we shouldn't cout this record
        m_HitterUIDRecord[nIndex] = m_HitterUIDRecord.back();
        m_HitterUIDRecord.pop_back();
    }

    auto fnCalcExp = [this](int nDamage) -> int
    {
        return nDamage * m_HitterUIDRecord.size();
    };

    for(auto rstRecord: m_HitterUIDRecord){
        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(rstRecord.UID)){
            AMExp stAME;
            stAME.Exp = fnCalcExp(rstRecord.Damage);
            m_ActorPod->Forward({MPK_EXP, stAME}, stUIDRecord.GetAddress());
        }
    }

    return true;
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
        m_ActorPod->Forward({MPK_UPDATEHP, stAMUHP}, m_Map->GetAddress());
    }
}

void CharObject::DispatchAttack(uint32_t nUID, int nDC)
{
    if(nUID && DCValid(nDC, true)){
        extern MonoServer *g_MonoServer;
        if(auto stRecord = g_MonoServer->GetUIDRecord(nUID)){

            AMAttack stAMA;
            stAMA.UID   = UID();
            stAMA.MapID = MapID();

            stAMA.X = X();
            stAMA.Y = Y();

            auto stDamage = GetAttackDamage(nDC);
            stAMA.Type    = stDamage.Type;
            stAMA.Damage  = stDamage.Damage;
            stAMA.Element = stDamage.Element;

            // copy the effect array
            for(size_t nIndex = 0; nIndex < sizeof(stAMA.Effect) / sizeof(stAMA.Effect[0]); ++nIndex){
                if(nIndex < stDamage.EffectArray.EffectLen()){
                    stAMA.Effect[nIndex] = stDamage.EffectArray.Effect()[nIndex];
                }else{
                    stAMA.Effect[nIndex] = EFF_NONE;
                }
            }

            m_ActorPod->Forward({MPK_ATTACK, stAMA}, stRecord.GetAddress());
        }
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

void CharObject::AddMonster(uint32_t nMonsterID, int nX, int nY, bool bRandom)
{
    AMAddCharObject stAMACO;
    stAMACO.Type = TYPE_MONSTER;

    stAMACO.Common.MapID  = m_Map->ID();
    stAMACO.Common.X      = nX;
    stAMACO.Common.Y      = nY;
    stAMACO.Common.Random = bRandom;

    stAMACO.Monster.MonsterID = nMonsterID;
    stAMACO.Monster.MasterUID = UID();

    auto fnOnRet = [](const MessagePack &rstRMPK, const Theron::Address &)
    {
        switch(rstRMPK.ID()){
            default:
                {
                    break;
                }
        }
    };
    m_ActorPod->Forward({MPK_ADDCHAROBJECT, stAMACO}, m_ServiceCore->GetAddress(), fnOnRet);
}

int CharObject::EstimateHop(int nX, int nY)
{
    if(m_Map->ValidC(nX, nY)){

        auto nMaxStep = MaxStep();
        auto nLDistance2 = LDistance2(nX, nY, X(), Y());

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
    return -1;
}

bool CharObject::CheckCacheLocation(int nX, int nY, uint32_t nTimeOut)
{
    // we can check cache location
    // if current location is occupied we *may* not try the move

    for(auto stLocation: m_LocationList){
        if(nTimeOut){
            extern MonoServer *g_MonoServer;
            if(g_MonoServer->GetTimeTick() > stLocation.second.RecordTime + nTimeOut){
                continue;
            }
        }

        if(true
                && stLocation.second.X == nX
                && stLocation.second.Y == nY){
            return false;
        }
    }
    return true;
}
