/*
 * =====================================================================================
 *
 *       Filename: charobject.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 07/06/2017 11:09:15
 *
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
#include "actorpod.hpp"
#include "monoserver.hpp"
#include "charobject.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"
#include "eventtaskhub.hpp"

CharObject::CharObject(ServiceCore *pServiceCore,
        ServerMap                  *pServerMap,
        int                         nMapX,
        int                         nMapY,
        int                         nDirection,
        uint8_t                     nLifeState)
    : ActiveObject()
    , m_ServiceCore(pServiceCore)
    , m_Map(pServerMap)
    , m_LocationRecord()
    , m_CurrX(nMapX)
    , m_CurrY(nMapY)
    , m_Direction(nDirection)
    , m_HP(0)
    , m_MP(0)
    , m_MoveLock(false)
    , m_AttackLock(false)
    , m_LastMoveTime(0)
    , m_LastAttackTime(0)
    , m_TargetQ()
    , m_Ability()
    , m_WAbility()
    , m_AddAbility()
{
    assert(m_Map);
    SetState(STATE_LIFECYCLE, nLifeState);

    auto fnRegisterClass = [this]() -> void {
        if(!RegisterClass<CharObject, ActiveObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <CharObject, ActiveObject> failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);
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
        DispatchAction({ACTION_STAND, 0, Direction(), X(), Y(), MapID()});
    }
    return stAddress;
}

void CharObject::DispatchAction(const ActionNode &rstAction)
{
    if(true
            && ActorPodValid()
            && m_Map
            && m_Map->ActorPodValid()){

        AMAction stAMA;
        std::memset(&stAMA, 0, sizeof(stAMA));

        stAMA.UID   = UID();
        stAMA.MapID = MapID();

        stAMA.Action      = rstAction.Action;
        stAMA.ActionParam = rstAction.ActionParam;
        stAMA.Speed       = rstAction.Speed;
        stAMA.Direction   = rstAction.Direction;

        stAMA.X    = rstAction.X;
        stAMA.Y    = rstAction.Y;
        stAMA.AimX = rstAction.AimX;
        stAMA.AimY = rstAction.AimY;

        m_ActorPod->Forward({MPK_ACTION, stAMA}, m_Map->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't dispatch action: %p", &rstAction);
}

bool CharObject::RequestMove(int nMoveMode, int nX, int nY, bool bAllowHalfMove, std::function<void()> fnOnMoveOK, std::function<void()> fnOnMoveError)
{
    int nStepLen = 0;
    switch(nMoveMode){
        case MOTION_WALK      : nStepLen = 1; break;    // human, monster
        case MOTION_RUN       : nStepLen = 2; break;    // human, monster
        case MOTION_HORSEWALK : nStepLen = 1; break;    // human
        case MOTION_HORSERUN  : nStepLen = 3; break;    // human
        default               : return false;
    }

    int nDX = std::abs<int>(X() - nX);
    int nDY = std::abs<int>(Y() - nY);

    if(true
            && ((CanMove()))
            && ((std::max<int>(nDX, nDY) == nStepLen))
            && ((std::min<int>(nDX, nDY) == 0) || (nDX == nDY))){

        // argument ok and can move
        // check if the ground is valid for the move request
        static const int nDirV[][3] = {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

        int nXLoc = (nX > X()) - (nX < X()) + 1;
        int nYLoc = (nY > Y()) - (nY < Y()) + 1;

        int nNewDirection = nDirV[nYLoc][nXLoc];
        int nMostStep = OneStepReach(nNewDirection, nStepLen, nullptr, nullptr);
        if(false
                || (nMostStep == nStepLen)
                || (bAllowHalfMove && (nMostStep > 0))){

            // OK we should try move
            // move request will be anwsered asynchronously
            // so record the location when move request sent for verification
            AMTryMove stAMTM;
            stAMTM.UID           = UID();
            stAMTM.MapID         = MapID();
            stAMTM.X             = X();
            stAMTM.Y             = Y();
            stAMTM.EndX          = nX;
            stAMTM.EndY          = nY;
            stAMTM.AllowHalfMove = bAllowHalfMove;

            int nOrigX = X();
            int nOrigY = Y();

            // TODO
            // be careful of the situation that new client motion request comes
            // but the server map is so busy that last motion request is even not handled yet
            // then all client request would be rejected and force client to do pull-back before handled
            auto fnOP = [this, nMoveMode, nX, nY, nOrigX, nOrigY, nNewDirection, bAllowHalfMove, fnOnMoveOK, fnOnMoveError](const MessagePack &rstMPK, const Theron::Address &rstAddr){
                if(!m_MoveLock){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "MoveLock released before server responds: ClassName = %s", ClassName());
                    g_MonoServer->Restart();
                }

                if(false
                        || (X() != nOrigX)
                        || (Y() != nOrigY)){
                    extern MonoServer *g_MonoServer;
                    g_MonoServer->AddLog(LOGTYPE_WARNING, "Motion detected during MoveLock set: ClassName = %s", ClassName());
                    g_MonoServer->Restart();
                }

                switch(rstMPK.Type()){
                    case MPK_MOVEOK:
                        {
                            AMMoveOK stAMMOK;
                            std::memcpy(&stAMMOK, rstMPK.Data(), sizeof(stAMMOK));

                            bool bValidMove = false;
                            if(false
                                    || ((bAllowHalfMove))
                                    || ((nX == stAMMOK.EndX) && (nY == stAMMOK.EndY))){

                                int nDX = std::abs<int>(X() - stAMMOK.EndX);
                                int nDY = std::abs<int>(Y() - stAMMOK.EndY);

                                int nNewX = -1;
                                int nNewY = -1;

                                if(true
                                        && NextLocation(&nNewX, &nNewY, nNewDirection, std::max<int>(nDX, nDY))
                                        && (nNewX == stAMMOK.EndX)
                                        && (nNewY == stAMMOK.EndY)){ bValidMove = true; }
                            }

                            if(bValidMove){
                                m_CurrX     = nX;
                                m_CurrY     = nY;
                                m_Direction = nNewDirection;

                                extern MonoServer *g_MonoServer;
                                m_LastMoveTime = g_MonoServer->GetTimeTick();

                                m_ActorPod->Forward(MPK_OK, rstAddr, rstMPK.ID());
                                DispatchAction({ACTION_MOVE, nMoveMode, SYS_DEFSPEED, Direction(), nOrigX, nOrigY, X(), Y(), MapID()});

                                if(fnOnMoveOK){ fnOnMoveOK(); }
                            }else{
                                m_ActorPod->Forward(MPK_ERROR, rstAddr, rstMPK.ID());
                                if(fnOnMoveError){ fnOnMoveError(); }
                            }

                            m_MoveLock = false;
                            break;
                        }
                    case MPK_ERROR:
                        {
                            m_Direction = nNewDirection;
                            DispatchAction({ACTION_STAND, 0, Direction(), X(), Y(), MapID()});

                            if(fnOnMoveError){ fnOnMoveError(); }
                            m_MoveLock = false;
                            break;
                        }
                    default:
                        {
                            extern MonoServer *g_MonoServer;
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "Unsupported response type for MPK_TRYMOVE: %s", rstMPK.Name());
                            m_MoveLock = false;
                            break;
                        }
                }
            };

            // send request and lock the co
            // 1. shouldn't have motion
            // 2. shouldn't release the lock
            m_MoveLock = true;
            return m_ActorPod->Forward({MPK_TRYMOVE, stAMTM}, m_Map->GetAddress(), fnOP);
        }
    }
    // currently not
    // should I call fnOnMoveError here ?
    return false;
}

bool CharObject::CanMove()
{
    return !m_MoveLock;
}

bool CharObject::CanAttack()
{
    return !m_AttackLock;
}

bool CharObject::MoveOneStep(int nMoveMode, int nX, int nY)
{
    int nMaxStep = 0;
    switch(nMoveMode){
        case MOTION_WALK      : nMaxStep = 1; break;    // human, monster
        case MOTION_RUN       : nMaxStep = 2; break;    // human, monster
        case MOTION_HORSEWALK : nMaxStep = 1; break;    // human
        case MOTION_HORSERUN  : nMaxStep = 3; break;    // human
        default               : return false;
    }

    int nDX = std::abs<int>(X() - nX);
    int nDY = std::abs<int>(Y() - nY);

    // +---+---+---+---+---+
    // |   |   |   |   |   |    one-hop motion can reach:
    // +---+---+---+---+---+
    // | 3 |   |   | 3 |   |    1: walk, horse_walk
    // +---+---+---+---+---+    2: run
    // | 2 |   | 2 |   |   |    3: horse_run
    // +---+---+---+---+---+
    // | 1 | 1 |   |   |   |
    // +---+---+---+---+---+
    // | O | 1 | 2 | 3 |   |
    // +---+---+---+---+---+

    if((nDX == 0) && (nDY == 0)){ return true; }
    if(true
            && ((std::max<int>(nDX, nDY) <= nMaxStep))
            && ((std::min<int>(nDX, nDY) == 0) || (nDX == nDY))){
        // can reach (nX, nY) in one-hop
        // possible reuslt for the motion request:
        //      1. completed fully      : MPK_MOVEOK
        //      2. completed partially  : MPK_MOVEOK
        //      3. failed               : MPK_ERROR
        // DON'T issue next motion if not complete the request
        // no matter how far it's from the destination, it only do one step and stop
        return RequestMove(nMoveMode, nX, nY, true, [](){}, [](){});
    }

    // can't reach in one hop
    // need firstly do path finding by server map

    AMPathFind stAMPF;
    stAMPF.UID     = UID();
    stAMPF.MapID   = MapID();
    stAMPF.MaxStep = 1;
    stAMPF.CheckCO = true;
    stAMPF.X       = X();
    stAMPF.Y       = Y();
    stAMPF.EndX    = nX;
    stAMPF.EndY    = nY;

    auto fnOnResp = [this, nMoveMode, nMaxStep](const MessagePack &rstRMPK, const Theron::Address &){
        switch(rstRMPK.Type()){
            case MPK_PATHFINDOK:
                {
                    AMPathFindOK stAMPFOK;
                    std::memcpy(&stAMPFOK, rstRMPK.Data(), sizeof(stAMPFOK));

                    // 1. for all received result
                    //    perform strict argument checking

                    constexpr auto nPathNodeCount = sizeof(stAMPFOK.Point) / sizeof(stAMPFOK.Point[0]);
                    static_assert(nPathNodeCount >= 2, "Invalid structure AMPathFindOK: Path node count should be at least 2");

                    auto fnReportBadPath = [&stAMPFOK, nPathNodeCount](size_t nBadNodeIndex){
                        extern MonoServer *g_MonoServer;
                        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid path finding result: %p", &stAMPFOK);
                        for(size_t nIndex = 0; nIndex <= std::min<size_t>(nPathNodeCount - 1, nBadNodeIndex); ++nIndex){
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::Point[%d]::X = %d", &stAMPFOK, (int)(nIndex), stAMPFOK.Point[nIndex].X);
                            g_MonoServer->AddLog(LOGTYPE_WARNING, "[%p]::Point[%d]::Y = %d", &stAMPFOK, (int)(nIndex), stAMPFOK.Point[nIndex].Y);
                        }
                    };

                    for(size_t nIndex = 0; nIndex < nPathNodeCount; ++nIndex){
                        if(true
                                && m_Map
                                && m_Map->In(stAMPFOK.MapID, stAMPFOK.Point[nIndex].X, stAMPFOK.Point[nIndex].Y)){
                            if(nIndex > 0){
                                int nX0 = stAMPFOK.Point[nIndex - 1].X;
                                int nY0 = stAMPFOK.Point[nIndex - 1].Y;
                                int nX1 = stAMPFOK.Point[nIndex    ].X;
                                int nY1 = stAMPFOK.Point[nIndex    ].Y;
                                switch(LDistance2(nX0, nY0, nX1, nY1)){
                                    case 1:
                                    case 2:
                                        {
                                            break;
                                        }
                                    default:
                                        {
                                            // nMaxStep could be more than 1
                                            // but path finding result should always be one-grid distance
                                            fnReportBadPath(nIndex);
                                            return;
                                        }
                                }
                            }
                        }else{
                            // we allow invalid point (-1, -1) to indicate end
                            // but we should at least have [0] and [1] valid in the result
                            if(nIndex < 2){ fnReportBadPath(nIndex); return; }
                            break;
                        }
                    }

                    // 2. minimize the hop count
                    //    start to check from Point[2]
                    //    Point[0] is the staring place and Point[1] is always OK

                    auto nMaxStepCheck = std::min<int>(nMaxStep, nPathNodeCount - 1);
                    auto nMaxStepReach = nMaxStepCheck;

                    for(int nStep = 1; nStep <= nMaxStepCheck; ++nStep){
                        int nDX = std::abs<int>(stAMPFOK.Point[nStep].X - stAMPFOK.Point[0].X);
                        int nDY = std::abs<int>(stAMPFOK.Point[nStep].Y - stAMPFOK.Point[0].Y);
                        if(true
                                // 1. current point is valid
                                //    invalid (-1, -1) stays at the ending part
                                && m_Map
                                && m_Map->In(stAMPFOK.MapID, stAMPFOK.Point[nStep].X, stAMPFOK.Point[nStep].Y)

                                // 2. current path stays straight
                                && ((std::max<int>(nDX, nDY) == nStep))
                                && ((std::min<int>(nDX, nDY) == 0) || (nDX == nDY))){ continue; }

                        nMaxStepReach = nStep - 1;
                        break;
                    }

                    RequestMove(nMoveMode, stAMPFOK.Point[nMaxStepReach].X, stAMPFOK.Point[nMaxStepReach].Y, true, [](){}, [](){});
                    break;
                }
            default:
                {
                    break;
                }
        }
    };
    return m_ActorPod->Forward({MPK_PATHFIND, stAMPF}, m_Map->GetAddress(), fnOnResp);
}

bool CharObject::RetrieveLocation(uint32_t nUID, std::function<void(int, int)> fnOnLocationOK)
{
    // TODO
    // I take a long time to design the prototype
    // 1. RetrieveLocation(UID, OnLocationOK)
    // 2. if(RetrieveLocation(UID, pX, pY)){
    //        OnLocationOK();
    //    }else{
    //        Forward(MPK_QUERYLOCATION, ..., OnLocationOK);
    //    }
    //
    //    actually these two are equal, reason
    //    1.     valid cache : the same
    //    2. not valid cache : we can choose to register OnLocationOK for MPK_QUERYLOCATION
    //                         but it's not necessary because if cache not valid we always have to query and register the handler
    //                         because we have no reason to not do it
    // so always do query and registration

    if(nUID){
        extern MonoServer *g_MonoServer;
        if(auto stRecord = g_MonoServer->GetUIDRecord(nUID)){
            // current the UID is valid
            // lambda captured fnOnLocationOK then can be delayed by one step
            // 1.     valid cache : call fnOnLocationOK
            // 2. not valid cache : call fnOnLocationOK after refresh
            auto fnQueryLocation = [this, stRecord, nUID, fnOnLocationOK]() -> bool
            {
                AMQueryLocation stAMQL;
                stAMQL.UID   = UID();
                stAMQL.MapID = MapID();

                auto fnOnResp = [this, nUID, fnOnLocationOK](const MessagePack &rstRMPK, const Theron::Address &){
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
                                    m_LocationRecord[nUID].UID        = UID();
                                    m_LocationRecord[nUID].MapID      = MapID();
                                    m_LocationRecord[nUID].RecordTime = stAML.RecordTime;
                                    m_LocationRecord[nUID].X          = stAML.X;
                                    m_LocationRecord[nUID].Y          = stAML.Y;
                                    if(fnOnLocationOK){ fnOnLocationOK(stAML.X, stAML.Y); }
                                }else{
                                    m_LocationRecord.erase(nUID);
                                }
                                break;
                            }
                        default:
                            {
                                m_LocationRecord.erase(nUID);
                                break;
                            }
                    }
                };
                return m_ActorPod->Forward({MPK_QUERYLOCATION, stAMQL}, stRecord.Address, fnOnResp);
            };

            // no entry found
            // do query and invocation, delay fnOnLocationOK by one step
            if(m_LocationRecord.find(nUID) == m_LocationRecord.end()){
                return fnQueryLocation();
            }

            // we find an entry
            // valid the entry, could be expired
            auto &rstRecord = m_LocationRecord[nUID];
            if(true
                    && m_Map
                    && m_Map->In(rstRecord.MapID, rstRecord.X, rstRecord.Y)
                    && (rstRecord.RecordTime + 2 * 1000 < g_MonoServer->GetTimeTick())){
                if(fnOnLocationOK){ fnOnLocationOK(rstRecord.X, rstRecord.Y); }
                return true;
            }

            return fnQueryLocation();
        }
    }
    return false;
}

bool CharObject::StruckDamage(int)
{
    m_HP = std::max<int>(0, m_HP - 1);
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
