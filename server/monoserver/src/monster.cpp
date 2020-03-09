/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
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

#include <tuple>
#include <cinttypes>
#include "player.hpp"
#include "motion.hpp"
#include "uidfunc.hpp"
#include "strfunc.hpp"
#include "dbconst.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "condcheck.hpp"
#include "netdriver.hpp"
#include "actorpod.hpp"
#include "mathfunc.hpp"
#include "memorypn.hpp"
#include "threadpn.hpp"
#include "sysconst.hpp"
#include "friendtype.hpp"
#include "randompick.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"

extern MonoServer *g_MonoServer;

Monster::AStarCache::AStarCache()
    : Time(0)
    , MapID(0)
    , Path()
{}

bool Monster::AStarCache::Retrieve(int *pX, int *pY, int nX0, int nY0, int nX1, int nY1, uint32_t nMapID)
{
    if(g_MonoServer->GetTimeTick() >= (Time + Refresh)){
        Path.clear();
        return false;
    }

    // if cache doesn't match we won't clean it
    // only cleared by timeout

    if((nMapID == MapID) && (Path.size() >= 3)){
        auto fnFindIndex = [this](int nX, int nY) -> int
        {
            for(int nIndex = 0; nIndex < (int)(Path.size()); ++nIndex){
                if(true
                        && Path[nIndex].X == nX
                        && Path[nIndex].Y == nY){
                    return nIndex;
                }
            }
            return -1;
        };

        auto nIndex0 = fnFindIndex(nX0, nY0);
        auto nIndex1 = fnFindIndex(nX1, nY1);

        if(true
                && nIndex0 >= 0
                && nIndex1 >= nIndex0 + 2){

            if(pX){
                *pX = Path[nIndex0 + 1].X;
            }

            if(pY){
                *pY = Path[nIndex0 + 1].Y;
            }
            return true;
        }
    }
    return false;
}

void Monster::AStarCache::Cache(std::vector<PathFind::PathNode> stvPathNode, uint32_t nMapID)
{
    MapID = nMapID;
    Path.swap(stvPathNode);

    Time = g_MonoServer->GetTimeTick();
}

Monster::Monster(uint32_t   nMonsterID,
        ServiceCore        *pServiceCore,
        ServerMap          *pServerMap,
        int                 nMapX,
        int                 nMapY,
        int                 nDirection,
        uint64_t            nMasterUID)
    : CharObject(pServiceCore, pServerMap, UIDFunc::BuildMonsterUID(nMonsterID), nMapX, nMapY, nDirection)
    , m_MonsterID(nMonsterID)
    , m_MasterUID(nMasterUID)
    , m_MonsterRecord(DBCOM_MONSTERRECORD(nMonsterID))
    , m_AStarCache()
    , m_BvTree()
    , m_UpdateCoro([this](){ UpdateCoroFunc(); })
{
    if(!m_MonsterRecord){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "Invalid monster record: MonsterID = %d", (int)(MonsterID()));
        g_MonoServer->Restart();
    }

    SetState(STATE_DEAD    , 0);
    SetState(STATE_NEVERDIE, 0);

    // set attack mode
    // SetState(STATE_ATTACKMODE, STATE_ATTACKMODE_NORMAL);
    SetState(STATE_ATTACKMODE, STATE_ATTACKMODE_ATTACKALL);

    m_HP    = m_MonsterRecord.HP;
    m_HPMax = m_MonsterRecord.HP;
    m_MP    = m_MonsterRecord.MP;
    m_MPMax = m_MonsterRecord.MP;
}

bool Monster::RandomMove()
{
    if(CanMove()){
        auto fnMoveOneStep = [this]() -> bool
        {
            int nX = -1;
            int nY = -1;
            if(OneStepReach(Direction(), 1, &nX, &nY) == 1){
                return RequestMove(nX, nY, MoveSpeed(), false, [](){}, [](){});
            }
            return false;
        };

        auto fnMakeOneTurn = [this]() -> bool
        {
            static const int nDirV[]
            {
                DIR_UP,
                DIR_UPRIGHT,
                DIR_RIGHT,
                DIR_DOWNRIGHT,
                DIR_DOWN,
                DIR_DOWNLEFT,
                DIR_LEFT,
                DIR_UPLEFT,
            };

            auto nDirCount = (int)(std::extent<decltype(nDirV)>::value);
            auto nDirStart = (int)(std::rand() % nDirCount);

            for(int nIndex = 0; nIndex < nDirCount; ++nIndex){
                auto nDirection = nDirV[(nDirStart + nIndex) % nDirCount];
                if(Direction() != nDirection){
                    int nX = -1;
                    int nY = -1;
                    if(OneStepReach(nDirection, 1, &nX, &nY) == 1){
                        // current direction is possible for next move
                        // report the turn and do motion (by chance) in next update
                        m_Direction = nDirection;
                        DispatchAction(ActionStand(X(), Y(), Direction()));

                        // we won't do ReportStand() for monster
                        // monster's moving is only driven by server currently
                        return true;
                    }
                }
            }
            return false;
        };

        // by 20% make a move
        // if move failed we make a turn

        if(std::rand() % 97 < 20){
            if(!fnMoveOneStep()){
                return fnMakeOneTurn();
            }
            return true;
        }

        // by 20% make a turn only
        // if didn't try the move/trun

        if(std::rand() % 97 < 20){
            return fnMakeOneTurn();
        }

        // do nothing
        // stay as idle state
    }
    return false;
}

bool Monster::RandomTurn()
{
    constexpr int dirs[]
    {
        DIR_UP,
        DIR_UPRIGHT,
        DIR_RIGHT,
        DIR_DOWNRIGHT,
        DIR_DOWN,
        DIR_DOWNLEFT,
        DIR_LEFT,
        DIR_UPLEFT,
    };

    const auto dirCount = (int)(std::extent<decltype(dirs)>::value);
    const auto dirStart = (int)(std::rand() % dirCount);

    for(int i = 0; i < dirCount; ++i){
        const auto dir = dirs[(dirStart + i) % dirCount];
        if(Direction() != dir){
            int newX = -1;
            int newY = -1;
            if(OneStepReach(dir, 1, &newX, &newY) == 1){
                // current direction is possible for next move
                // report the turn and do motion (by chance) in next update
                m_Direction = dir;
                DispatchAction(ActionStand(X(), Y(), Direction()));
                return true;
            }
        }
    }
    return false;
}

void Monster::AttackUID(uint64_t nUID, int nDC, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!CanAttack()){
        fnOnError();
        return;
    }

    if(!DCValid(nDC, true)){
        fnOnError();
        return;
    }

    // retrieving could schedule location query
    // before response received we can't allow any attack request

    m_AttackLock = true;
    RetrieveLocation(nUID, [this, nDC, nUID, fnOnOK, fnOnError](const COLocation &stCOLocation)
    {
        if(!m_AttackLock){
            throw std::runtime_error(str_ffl() + "AttackLock released before location query done");
        }
        m_AttackLock = false;

        auto nX = stCOLocation.X;
        auto nY = stCOLocation.Y;

        switch(nDC){
            case DC_PHY_PLAIN:
                {
                    switch(MathFunc::LDistance2(X(), Y(), nX, nY)){
                        case 1:
                        case 2:
                            {
                                m_Direction = PathFind::GetDirection(X(), Y(), nX, nY);
                                if(!CanAttack()){
                                    fnOnError();
                                    return;
                                }

                                SetTarget(nUID);
                                m_LastAttackTime = g_MonoServer->GetTimeTick();
                                DispatchAction(ActionAttack(X(), Y(), DC_PHY_PLAIN, AttackSpeed(), nUID));

                                // 2. send attack message to target
                                //    target can ignore this message directly
                                //
                                //    For mir2 code, at the time when monster attacks
                                //    1. immediately change target CO's HP and MP, but don't report
                                //    2. delay 550ms, then report RM_ATTACK with CO's new HP and MP
                                //    3. target CO reports to client for motion change (_MOTION_HITTED) and new HP/MP
                                Delay(550, [this, nUID]()
                                {
                                    // monster may go dead after this delay
                                    // but don't check CanAttack() since that's for attack lock
                                    if(true){
                                        DispatchAttack(nUID, DC_PHY_PLAIN);
                                    }
                                });

                                fnOnOK();
                                return;
                            }
                        case 0:
                            {
                                // TODO this can happen
                                // should I schedule an random move?
                                fnOnError();
                                return;
                            }
                        default:
                            {
                                fnOnError();
                                return;
                            }
                    }
                }
            case DC_MAG_FIRE:
            default:
                {
                    fnOnError();
                    return;
                }
        }
    },

    [this, nUID]()
    {
        m_AttackLock = false;

        RemoveTarget(nUID);
        RemoveInViewCO(nUID);
    });
}

void Monster::TrackUID(uint64_t nUID, int nMinCDistance, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(nMinCDistance < 1){
        throw std::invalid_argument(str_fflprintf(": Invalid distance: %d", nMinCDistance));
    }

    RetrieveLocation(nUID, [this, nMinCDistance, fnOnOK, fnOnError](const COLocation &rstCOLocation) -> bool
    {
        auto nX     = rstCOLocation.X;
        auto nY     = rstCOLocation.Y;
        auto nMapID = rstCOLocation.MapID;

        if(!m_Map->In(nMapID, nX, nY)){
            fnOnError();
            return false;
        }

        if(MathFunc::CDistance(X(), Y(), nX, nY) <= nMinCDistance){
            fnOnOK();
            return true;
        }

        MoveOneStep(nX, nY, fnOnOK, fnOnError);
        return true;
    }, fnOnError);
}

void Monster::FollowMaster(std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!(MasterUID() && CanMove())){
        fnOnError();
        return;
    }

    // followMaster works almost like TrackUID(), but
    // 1. follower always try to stand at the back of the master
    // 2. when distance is too far or master is on different map, follower takes space move

    RetrieveLocation(MasterUID(), [this, fnOnOK, fnOnError](const COLocation &rstCOLocation) -> bool
    {
        // check if it's still my master?
        // possible during the location query master changed

        if(rstCOLocation.UID != MasterUID()){
            fnOnError();
            return false;
        }

        if(!CanMove()){
            fnOnError();
            return false;
        }

        auto nX         = rstCOLocation.X;
        auto nY         = rstCOLocation.Y;
        auto nMapID     = rstCOLocation.MapID;
        auto nDirection = rstCOLocation.Direction;

        // get back location with different distance
        // here we use different distance if space move and follow

        auto fnGetBack = [](int nX, int nY, int nDirection, int nLD) -> std::tuple<int, int>
        {
            // randomly pick a location
            // for some COs it doesn't have direction
            if(!PathFind::ValidDir(nDirection)){
                nDirection = ((std::rand() % 8) + (DIR_NONE + 1));
            }

            int nBackX = -1;
            int nBackY = -1;

            PathFind::GetBackLocation(&nBackX, &nBackY, nX, nY, nDirection, nLD);
            return {nBackX, nBackY};
        };

        if((nMapID == MapID()) && (MathFunc::LDistance<double>(nX, nY, X(), Y()) < 10.0)){

            // not that long
            // slave should move step by step

            auto [nBackX, nBackY] = fnGetBack(nX, nY, nDirection, 1);
            switch(MathFunc::LDistance2(nBackX, nBackY, X(), Y())){
                case 0:
                    {
                        // already get there
                        // need to make a turn if needed

                        if(Direction() != nDirection){
                            m_Direction= nDirection;
                            DispatchAction(ActionStand(X(), Y(), Direction()));
                        }

                        fnOnOK();
                        return true;
                    }
                default:
                    {
                        return MoveOneStep(nBackX, nBackY, fnOnOK, fnOnError);
                    }
            }
        }

        // long distance
        // slave have to do space move

        auto [nBackX, nBackY] = fnGetBack(nX, nY, nDirection, 3);
        return RequestSpaceMove(nMapID, nBackX, nBackY, false, fnOnOK, fnOnError);
    });
}

void Monster::TrackAttackUID(uint64_t nTargetUID, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!nTargetUID){
        throw std::invalid_argument(str_fflprintf(": Invalid zero UID"));
    }

    // TODO choose proper DC
    // for different monster it may use different DC
    
    int nProperDC = m_MonsterRecord.DCList()[0];
    int nMinCDistance = 1;

    TrackUID(nTargetUID, nMinCDistance, [nTargetUID, nProperDC, fnOnOK, fnOnError, this]()
    {
        AttackUID(nTargetUID, nProperDC, fnOnOK, fnOnError);
    }, fnOnError);
}

uint64_t Monster::Activate()
{
    if(auto nUID = CharObject::Activate()){
        CreateBvTree();
        return nUID;
    }
    return 0;
}

void Monster::UpdateCoroFunc()
{
    while(HP() > 0){

        if(const uint64_t targetUID = CoroNode_GetProperTarget()){
            CoroNode_TrackAttackUID(targetUID);
        }

        else if(MasterUID()){
            if(m_ActorPod->CheckInvalid(MasterUID())){
                GoDie();
            }
            CoroNode_FollowMaster();
        }

        else{
            CoroNode_RandomMove();
        }
    }

    GoDie();
}

bool Monster::Update()
{
    if(HP() < 0){
        return GoDie();
    }

    if(MasterUID() && m_ActorPod->CheckInvalid(MasterUID())){
        return GoDie();
    }

    if(!m_UpdateCoro.exited()){
        m_UpdateCoro.coro_resume();
    }
    return true;

    switch(auto nState = m_BvTree->update()){
        case BV_PENDING:
            {
                return true;
            }
        case BV_ABORT:
        case BV_SUCCESS:
        case BV_FAILURE:
            {
                m_BvTree->reset();
                return true;
            }
        default:
            {
                throw std::runtime_error(str_fflprintf(": Invalid node status: %d", nState));
            }
    }
}

void Monster::OperateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_CHECKMASTER:
            {
                On_MPK_CHECKMASTER(rstMPK);
                break;
            }
        case MPK_QUERYFINALMASTER:
            {
                On_MPK_QUERYFINALMASTER(rstMPK);
                break;
            }
        case MPK_QUERYFRIENDTYPE:
            {
                On_MPK_QUERYFRIENDTYPE(rstMPK);
                break;
            }
        case MPK_QUERYNAMECOLOR:
            {
                On_MPK_QUERYNAMECOLOR(rstMPK);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                On_MPK_NOTIFYNEWCO(rstMPK);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                On_MPK_DEADFADEOUT(rstMPK);
                break;
            }
        case MPK_NOTIFYDEAD:
            {
                On_MPK_NOTIFYDEAD(rstMPK);
                break;
            }
        case MPK_UPDATEHP:
            {
                On_MPK_UPDATEHP(rstMPK);
                break;
            }
        case MPK_EXP:
            {
                On_MPK_EXP(rstMPK);
                break;
            }
        case MPK_MISS:
            {
                On_MPK_MISS(rstMPK);
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK);
                break;
            }
        case MPK_ATTACK:
            {
                On_MPK_ATTACK(rstMPK);
                break;
            }
        case MPK_MAPSWITCH:
            {
                On_MPK_MAPSWITCH(rstMPK);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                On_MPK_QUERYLOCATION(rstMPK);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                On_MPK_QUERYCORECORD(rstMPK);
                break;
            }
        case MPK_BADACTORPOD:
            {
                On_MPK_BADACTORPOD(rstMPK);
                break;
            }
        case MPK_OFFLINE:
            {
                On_MPK_OFFLINE(rstMPK);
                break;
            }
        default:
            {
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

void Monster::SearchViewRange()
{
}

void Monster::ReportCORecord(uint64_t nUID)
{
    if(!nUID || nUID == UID()){
        return;
    }

    AMCORecord stAMCOR;
    std::memset(&stAMCOR, 0, sizeof(stAMCOR));

    // TODO: don't use OBJECT_MONSTER, we need translation
    //       rule of communication, the sender is responsible to translate

    // 1. set type
    stAMCOR.COType = CREATURE_MONSTER;

    // 2. set current action
    stAMCOR.Action.UID   = UID();
    stAMCOR.Action.MapID = MapID();

    stAMCOR.Action.Action    = ACTION_STAND;
    stAMCOR.Action.Speed     = SYS_DEFSPEED;
    stAMCOR.Action.Direction = Direction();

    stAMCOR.Action.X    = X();
    stAMCOR.Action.Y    = Y();
    stAMCOR.Action.AimX = X();
    stAMCOR.Action.AimY = Y();

    stAMCOR.Action.AimUID      = 0;
    stAMCOR.Action.ActionParam = 0;

    // 3. set specified co information
    stAMCOR.Monster.MonsterID = MonsterID();

    // don't reply to server map
    // even get co information pull request from map
    m_ActorPod->Forward(nUID, {MPK_CORECORD, stAMCOR});
}

bool Monster::InRange(int nRangeType, int nX, int nY)
{
    if(true
            && m_Map
            && m_Map->ValidC(nX, nY)){
        switch(nRangeType){
            case RANGE_VISIBLE:
                {
                    return MathFunc::LDistance2(X(), Y(), nX, nY) < 20 * 20;
                }
            case RANGE_ATTACK:
                {
                    // inside this range
                    // monster will decide to make an attack
                    return MathFunc::LDistance2(X(), Y(), nX, nY) < 10 * 10;
                }
        }
    }
    return false;
}

DamageNode Monster::GetAttackDamage(int nDC)
{
    switch(nDC){
        case DC_PHY_PLAIN:
            {
                return {UID(), nDC, m_MonsterRecord.DC + std::rand() % (1 + (std::max<int>)(m_MonsterRecord.DCMax - m_MonsterRecord.DC, 0)), EC_NONE};
            }
        case DC_MAG_FIRE:
            {
                return {UID(), nDC, m_MonsterRecord.MDC + std::rand() % (1 + (std::max<int>)(m_MonsterRecord.MDCMax - m_MonsterRecord.MDC, 0)), EC_FIRE};
            }
        default:
            {
                return {};
            }
    }
}

bool Monster::CanMove()
{
    if(CharObject::CanMove() && CanAct()){
        return g_MonoServer->GetTimeTick() >= m_LastMoveTime + m_MonsterRecord.WalkWait;
    }
    return false;
}

// should have a better way for GCD (global cooldown)
// because for actions with gfx frames, client takes time to present
// we should give client a chance to show all its animation before dispatch action result

// i.e.:
// monster moves to (x, y) and attacks player
// if there is no GCD, it jumps to (x, y) and immediately attack player and dispatch the attack result
// from client we see monster is just start moving, but player already get the ACTION_HITTED

bool Monster::CanAttack()
{
    if(!CanAct()){
        return false;
    }

    if(!CharObject::CanAttack()){
        return false;
    }

    auto nCurrTick = g_MonoServer->GetTimeTick();

    if(nCurrTick < m_LastMoveTime + 800){
        return false;
    }

    return nCurrTick >= m_LastAttackTime + m_MonsterRecord.AttackWait;
}

bool Monster::DCValid(int nDC, bool bCheck)
{
    if(std::find(m_MonsterRecord.DCList().begin(), m_MonsterRecord.DCList().end(), nDC) != m_MonsterRecord.DCList().end()){
        if(bCheck){
            if(auto &rstDCR = DB_DCRECORD(nDC)){
                if(m_HP < rstDCR.HP){ return false; }
                if(m_MP < rstDCR.MP){ return false; }
            }
        }
        return true;
    }
    return false;
}

void Monster::RemoveTarget(uint64_t nUID)
{
    if(m_Target.UID == nUID){
        m_Target = {};
    }
}

void Monster::SetTarget(uint64_t nUID)
{
    m_Target.UID = nUID;
    m_Target.ActiveTime = g_MonoServer->GetTimeTick();
}

bool Monster::GoDie()
{
    switch(GetState(STATE_NEVERDIE)){
        case 0:
            {
                switch(GetState(STATE_DEAD)){
                    case 0:
                        {
                            RandomDrop();
                            DispatchOffenderExp();

                            // dispatch die acton without auto-fade-out
                            // server send the fade-out request in GoGhost()

                            // auto-fade-out is for zombie handling
                            // when client confirms a zombie, client use auto-fade-out die action

                            DispatchAction(ActionDie(X(), Y(), Direction(), false));

                            // let's dispatch ActionDie before mark it dead
                            // theoratically dead actor shouldn't dispatch anything

                            SetState(STATE_DEAD, 1);

                            Delay(2 * 1000, [this](){ GoGhost(); });
                            return true;
                        }
                    default:
                        {
                            return true;
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::GoGhost()
{
    switch(GetState(STATE_NEVERDIE)){
        case 0:
            {
                switch(GetState(STATE_DEAD)){
                    case 0:
                        {
                            return false;
                        }
                    default:
                        {
                            // 1. setup state and inform all others
                            SetState(STATE_GHOST, 1);

                            AMDeadFadeOut stAMDFO;
                            std::memset(&stAMDFO, 0, sizeof(stAMDFO));

                            stAMDFO.UID   = UID();
                            stAMDFO.MapID = MapID();
                            stAMDFO.X     = X();
                            stAMDFO.Y     = Y();

                            if(true
                                    && ActorPodValid()
                                    && m_Map
                                    && m_Map->ActorPodValid()){
                                m_ActorPod->Forward(m_Map->UID(), {MPK_DEADFADEOUT, stAMDFO});
                            }

                            // 2. deactivate the actor here
                            //    disable the actorpod then no source can drive it
                            //    then current *this* can't be refered by any actor threads after this invocation
                            //    then MonoServer::EraseUID() is safe to delete *this*
                            //
                            //    don't do delete m_ActorPod to disable the actor
                            //    since currently we are in the actor thread which accquired by m_ActorPod
                            Deactivate();
                            return true;
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::StruckDamage(const DamageNode &rstDamage)
{
    switch(UIDFunc::GetMonsterID(UID())){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                if(MasterUID()){
                    return true;
                }
                break;
            }
        default:
            {
                break;
            }
    }

    if(rstDamage){
        m_HP = (std::max<int>)(0, HP() - rstDamage.Damage);
        DispatchHealth();

        if(HP() <= 0){
            GoDie();
        }
        return true;
    }
    return false;
}

bool Monster::MoveOneStep(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!CanMove()){
        fnOnError();
        return false;
    }

    switch(EstimateHop(nX, nY)){
        case 0:
            {
                fnOnError();
                return false;
            }
        case 1:
            {
                if(OneStepCost(nullptr, 1, X(), Y(), nX, nY) >= 0.00){
                    return RequestMove(nX, nY, MoveSpeed(), false, fnOnOK, fnOnError);
                }
                break;
            }
        case 2:
            {
                break;
            }
        default:
            {
                fnOnError();
                return false;
            }
    }

    int nXm = -1;
    int nYm = -1;

    if(m_AStarCache.Retrieve(&nXm, &nYm, X(), Y(), nX, nY, MapID())){
        if(OneStepCost(nullptr, 1, X(), Y(), nXm, nYm) >= 0.00){
            return RequestMove(nXm, nYm, MoveSpeed(), false, fnOnOK, fnOnError);
        }
    }

    switch(FindPathMethod()){
        case FPMETHOD_ASTAR:
            {
                return MoveOneStepAStar(nX, nY, fnOnOK, fnOnError);
            }
        case FPMETHOD_GREEDY:
            {
                return MoveOneStepGreedy(nX, nY, fnOnOK, fnOnError);
            }
        case FPMETHOD_COMBINE:
            {
                return MoveOneStepCombine(nX, nY, fnOnOK, fnOnError);
            }
        case FPMETHOD_NEIGHBOR:
            {
                return MoveOneStepNeighbor(nX, nY, fnOnOK, fnOnError);
            }
        default:
            {
                fnOnError();
                return false;
            }
    }
}

bool Monster::MoveOneStepNeighbor(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!CanMove()){
        fnOnError();
        return false;
    }

    CharObject::COPathFinder stFinder(this, 1);
    if(!stFinder.Search(X(), Y(), nX, nY)){
        fnOnError();
        return false;
    }

    auto [stPathNode, nNodeNum] = stFinder.GetFirstNPathNode<5>();
    if(nNodeNum < 2){
        throw std::runtime_error(str_fflprintf(": Incorrect pathnode number: %zu", nNodeNum));
    }

    m_AStarCache.Cache({stPathNode.begin(), stPathNode.begin() + nNodeNum}, MapID());
    return RequestMove(stPathNode[1].X, stPathNode[1].Y, MoveSpeed(), false, fnOnOK, fnOnError);
}

bool Monster::MoveOneStepGreedy(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!CanMove()){
        fnOnError();
        return false;
    }

    bool bLongJump   = (MaxStep() > 1) && (MathFunc::CDistance(X(), Y(), nX, nY) >= MaxStep());
    auto stvPathNode = GetChaseGrid(nX, nY, bLongJump ? MaxStep() : 1);

    return RequestMove(stvPathNode[0].X, stvPathNode[0].Y, MoveSpeed(), false, fnOnOK, [this, bLongJump, nX, nY, stvPathNode, fnOnOK, fnOnError]()
    {
        RequestMove(stvPathNode[1].X, stvPathNode[1].Y, MoveSpeed(), false, fnOnOK, [this, bLongJump, nX, nY, stvPathNode, fnOnOK, fnOnError]()
        {
            RequestMove(stvPathNode[2].X, stvPathNode[2].Y, MoveSpeed(), false, fnOnOK, [this, bLongJump, nX, nY,fnOnOK, fnOnError]()
            {
                if(!bLongJump){
                    fnOnError();
                    return;
                }

                auto stvMinPathNode = GetChaseGrid(nX, nY, 1);
                RequestMove(stvMinPathNode[0].X, stvMinPathNode[0].Y, MoveSpeed(), false, fnOnOK, [this, stvMinPathNode, fnOnOK, fnOnError]()
                {
                    RequestMove(stvMinPathNode[1].X, stvMinPathNode[1].Y, MoveSpeed(), false, fnOnOK, [this, stvMinPathNode, fnOnOK, fnOnError]()
                    {
                        RequestMove(stvMinPathNode[2].X, stvMinPathNode[2].Y, MoveSpeed(), false, fnOnOK, fnOnError);
                    });
                });
            });
        });
    });
}

bool Monster::MoveOneStepCombine(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!CanMove()){
        fnOnError();
        return false;
    }

    return MoveOneStepGreedy(nX, nY, fnOnOK, [this, nX, nY, fnOnOK, fnOnError]()
    {
        MoveOneStepNeighbor(nX, nY, fnOnOK, fnOnError);
    });
}

bool Monster::MoveOneStepAStar(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!CanMove()){
        fnOnError();
        return false;
    }

    AMPathFind stAMPF;
    std::memset(&stAMPF, 0, sizeof(stAMPF));

    stAMPF.UID     = UID();
    stAMPF.MapID   = MapID();
    stAMPF.CheckCO = 1;
    stAMPF.MaxStep = MaxStep();
    stAMPF.X       = X();
    stAMPF.Y       = Y();
    stAMPF.EndX    = nX;
    stAMPF.EndY    = nY;

    return m_ActorPod->Forward(MapUID(), {MPK_PATHFIND, stAMPF}, [this, nX, nY, fnOnOK, fnOnError](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.Type()){
            case MPK_PATHFINDOK:
                {
                    AMPathFindOK stAMPFOK;
                    std::memcpy(&stAMPFOK, rstRMPK.Data(), sizeof(stAMPFOK));

                    constexpr auto nNodeCount = std::extent<decltype(stAMPFOK.Point)>::value;
                    static_assert(nNodeCount >= 2);

                    auto pBegin = stAMPFOK.Point;
                    auto pEnd   = stAMPFOK.Point + nNodeCount; 

                    std::vector<PathFind::PathNode> stvPathNode;
                    for(auto pCurr = pBegin; pCurr != pEnd; ++pCurr){
                        if(m_Map->GroundValid(pCurr->X, pCurr->Y)){
                            stvPathNode.emplace_back(pCurr->X, pCurr->Y);
                        }else{
                            break;
                        }
                    }

                    if(!stvPathNode.back().Eq(nX, nY)){
                        stvPathNode.emplace_back(nX, nY);
                    }
                    m_AStarCache.Cache(stvPathNode, MapID());

                    RequestMove(stAMPFOK.Point[1].X, stAMPFOK.Point[1].Y, MoveSpeed(), false, fnOnOK, fnOnError);
                    break;
                }
            default:
                {
                    fnOnError();
                    break;
                }
        }
    });
}

int Monster::FindPathMethod()
{
    return FPMETHOD_COMBINE;
}

void Monster::RandomDrop()
{
    for(auto &rstGroupRecord: DB_MONSTERDROPITEM(MonsterID())){
        for(auto &rstItemRecord: rstGroupRecord.second){
            if(std::rand() % rstItemRecord.ProbRecip == 0){
                AMNewDropItem stAMNDI;
                stAMNDI.UID   = UID();
                stAMNDI.X     = X();
                stAMNDI.Y     = Y();
                stAMNDI.ID    = rstItemRecord.ID;
                stAMNDI.Value = rstItemRecord.Value;

                // suggest server map to add a new drop item, but server map
                // may reject this suggestion silently.
                //
                // and if we are not in group-0
                // break if we select the first one item
                m_ActorPod->Forward(m_Map->UID(), {MPK_NEWDROPITEM, stAMNDI});
                if(rstGroupRecord.first != 0){
                    break;
                }
            }
        }
    }
}

void Monster::RecursiveCheckInViewTarget(size_t nIndex, std::function<void(uint64_t)> fnTarget)
{
    if(nIndex >= m_InViewCOList.size()){
        fnTarget(0);
        return;
    }

    auto nUID = m_InViewCOList[nIndex].UID;
    CheckFriendType(nUID, [this, nIndex, nUID, fnTarget](int nFriendType)
    {
        // when reach here
        // m_InViewCOList[nIndex] may not be nUID anymore

        // if changed
        // we'd better redo the search

        if(nIndex >= m_InViewCOList.size() || m_InViewCOList[nIndex].UID != nUID){
            RecursiveCheckInViewTarget(0, fnTarget);
            return;
        }

        if(nFriendType == FT_ENEMY){
            fnTarget(nUID);
            return;
        }
        RecursiveCheckInViewTarget(nIndex + 1, fnTarget);
    });
}

void Monster::SearchNearestTarget(std::function<void(uint64_t)> fnTarget)
{
    if(m_InViewCOList.empty()){
        fnTarget(0);
        return;
    }
    RecursiveCheckInViewTarget(0, fnTarget);
}

void Monster::GetProperTarget(std::function<void(uint64_t)> fnTarget)
{
    if(m_Target.UID){
        if(g_MonoServer->GetTimeTick() < m_Target.ActiveTime + 60 * 1000){
            CheckFriendType(m_Target.UID, [nTargetUID = m_Target.UID, fnTarget, this](int nFriendType)
            {
                if(nFriendType == FT_ENEMY){
                    fnTarget(nTargetUID);
                    return;
                }

                // may changed monster
                // last target is not a target anymore

                RemoveTarget(nTargetUID);
                SearchNearestTarget(fnTarget);
            });
        }
        return;
    }

    RemoveTarget(m_Target.UID);
    SearchNearestTarget(fnTarget);
}

void Monster::CheckFriend(uint64_t nCheckUID, const std::function<void(int)> &fnOnFriend)
{
    enum UIDFromType: int
    {
        UIDFROM_NONE    = 0,
        UIDFROM_PLAYER  = 1,
        UIDFROM_SUMMON  = 2,
        UIDFROM_SLAVE   = 3,
        UIDFROM_MONSTER = 4,
    };

    auto fnUIDFrom = [](uint64_t nUID) -> int
    {
        // define return code
        // <= 0: error
        //    1: player
        //    2: 变异骷髅 or 神兽
        //    3: monster, but tameble
        //    4: monster

        if(UIDFunc::GetUIDType(nUID) == UID_PLY){
            return UIDFROM_PLAYER;
        }else if(UIDFunc::GetUIDType(nUID) == UID_MON){
            switch(UIDFunc::GetMonsterID(nUID)){
                case DBCOM_MONSTERID(u8"神兽"):
                case DBCOM_MONSTERID(u8"变异骷髅"):
                    {
                        return UIDFROM_SUMMON;
                    }
                default:
                    {
                        return UIDFROM_MONSTER;
                    }
            }
        }
        return UIDFROM_NONE;
    };

    if(MasterUID()){
        switch(fnUIDFrom(MasterUID())){
            case UIDFROM_PLAYER:
                {
                    switch(fnUIDFrom(nCheckUID)){
                        case UIDFROM_PLAYER:
                        case UIDFROM_SUMMON:
                            {
                                fnOnFriend(FT_FRIEND);
                                return;
                            }
                        case UIDFROM_MONSTER:
                            {
                                fnOnFriend(FT_ENEMY);
                                return;
                            }
                        default:
                            {
                                return;
                            }
                    }
                }
            case UIDFROM_MONSTER:
                {
                    switch(fnUIDFrom(nCheckUID)){
                        case UIDFROM_PLAYER:
                        case UIDFROM_SUMMON:
                            {
                                fnOnFriend(FT_ENEMY);
                                return;
                            }
                        case UIDFROM_MONSTER:
                            {
                                fnOnFriend(FT_FRIEND);
                                return;
                            }
                        default:
                            {
                                return;
                            }
                    }
                }
            default:
                {
                    return;
                }
        }
    }else{

        // a monster without master
        // do everything decided by itself

        switch(fnUIDFrom(nCheckUID)){
            case UIDFROM_PLAYER:
            case UIDFROM_SUMMON:
                {
                    fnOnFriend(FT_ENEMY);
                    return;
                }
            case UIDFROM_MONSTER:
                {
                    fnOnFriend(FT_FRIEND);
                    return;
                }
            default:
                {
                    return;
                }
        }
    }
}

void Monster::CreateBvTree()
{
    bvarg_ref nTargetUID;

    m_BvTree = bvtree::if_branch
    (
        BvNode_GetProperTarget(nTargetUID),
        BvNode_TrackAttackUID(nTargetUID),

        bvtree::if_branch
        (
            BvNode_HasMaster(),
            BvNode_FollowMaster(),
            BvNode_RandomMove()
        )
    );
    m_BvTree->reset();
}

void Monster::QueryMaster(uint64_t nUID, std::function<void(uint64_t)> fnOp)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": Invalid zero UID"));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_fflprintf(": Query self for MasterUID()"));
    }

    m_ActorPod->Forward(nUID, MPK_QUERYMASTER, [this, nUID, fnOp](const MessagePack &rstRMPK)
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
                    if(nUID == MasterUID()){
                        GoDie();
                    }
                    return;
                }
        }
    });
}

void Monster::CheckFriendType_AsGuard(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!IsGuard(UID())){
        throw std::runtime_error(str_fflprintf(": Invalid call to CheckFriendType_AsGuard"));
    }

    switch(UIDFunc::GetUIDType(nUID)){
        case UID_MON:
            {
                fnOp(IsGuard(nUID) ? FT_NEUTRAL : FT_ENEMY);
                return;
            }
        case UID_PLY:
            {
                m_ActorPod->Forward(nUID, MPK_QUERYNAMECOLOR, [fnOp](const MessagePack &rstMPK)
                {
                    switch(rstMPK.Type()){
                        case MPK_NAMECOLOR:
                            {
                                AMNameColor stAMNC;
                                std::memcpy(&stAMNC, rstMPK.Data(), sizeof(stAMNC));

                                switch(stAMNC.Color){
                                    case 'R':
                                        {
                                            fnOp(FT_ENEMY);
                                            return;
                                        }
                                    default:
                                        {
                                            fnOp(FT_NEUTRAL);
                                            return;
                                        }
                                }
                            }
                        default:
                            {
                                fnOp(FT_ERROR);
                                return;
                            }
                    }
                });
                return;
            }
        default:
            {
                throw std::invalid_argument(str_fflprintf(": Invalid UID type: %s", UIDFunc::GetUIDTypeString(nUID)));
            }
    }
}

void Monster::CheckFriendType_CtrlByMonster(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(MasterUID() && UIDFunc::GetUIDType(MasterUID()) != UID_MON){
        throw std::runtime_error(str_fflprintf(": Invalid call to CheckFriendType_CtrlByMonster"));
    }

    switch(UIDFunc::GetUIDType(nUID)){
        case UID_PLY:
            {
                fnOp(FT_ENEMY);
                return;
            }
        case UID_MON:
            {
                if(!DBCOM_MONSTERRECORD(UIDFunc::GetMonsterID(nUID)).Tamable){
                    fnOp(FT_NEUTRAL);
                    return;
                }

                QueryFinalMaster(nUID, [nUID, fnOp](uint64_t nFMasterUID)
                {
                    if(!nFMasterUID){
                        fnOp(FT_ERROR);
                        return;
                    }

                    switch(UIDFunc::GetUIDType(nFMasterUID)){
                        case UID_MON:
                            {
                                fnOp(FT_NEUTRAL);
                                return;
                            }
                        case UID_PLY:
                            {
                                fnOp(FT_ENEMY);
                                return;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid final master type: %s", UIDFunc::GetUIDTypeString(nFMasterUID)));
                            }
                    }
                });
                return;
            }
        default:
            {
                throw std::invalid_argument(str_fflprintf(": Invalid UID type: %s", UIDFunc::GetUIDTypeString(nUID)));
            }
    }
}

void Monster::CheckFriendType_CtrlByPlayer(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(UIDFunc::GetUIDType(MasterUID()) != UID_PLY){
        throw std::runtime_error(str_fflprintf(": Invalid call to CheckFriendType_CtrlByPlayer"));
    }

    switch(UIDFunc::GetUIDType(nUID)){
        case UID_MON:
            {
                if(!(IsPet(nUID) || DBCOM_MONSTERRECORD(UIDFunc::GetMonsterID(nUID)).Tamable)){
                    fnOp(FT_ENEMY);
                    return;
                }

                QueryFinalMaster(nUID, [this, nUID, fnOp](uint64_t nFMasterUID)
                {
                    if(!nFMasterUID){
                        fnOp(FT_ERROR);
                        return;
                    }

                    switch(UIDFunc::GetUIDType(nFMasterUID)){
                        case UID_MON:
                            {
                                fnOp(FT_ENEMY);
                                return;
                            }
                        case UID_PLY:
                            {
                                if(nFMasterUID == MasterUID()){
                                    fnOp(FT_NEUTRAL);
                                    return;
                                }

                                QueryFriendType(MasterUID(), nUID, [fnOp](int nFriendType)
                                {
                                    fnOp(nFriendType);
                                });
                                return;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid final master type: %s", UIDFunc::GetUIDTypeString(nFMasterUID)));
                            }
                    }
                });
                return;
            }
        case UID_PLY:
            {
                QueryFriendType(MasterUID(), nUID, [fnOp](int nFriendType)
                {
                    fnOp(nFriendType);
                });
                return;
            }
        default:
            {
                throw std::runtime_error(str_fflprintf(": Invalid call to CheckFriendType_CtrlByPlayer"));
            }
    }
}

void Monster::CheckFriendType(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": Invalid zero UID"));
    }

    if(nUID == UID()){
        throw std::invalid_argument(str_fflprintf(": Check friend type to self"));
    }

    // 1. 大刀卫士 or 弓箭卫士
    // 2. no master or master is still monster
    // 3. as pet, master is UID_PLY

    if(IsGuard(UID())){
        CheckFriendType_AsGuard(nUID, fnOp);
        return;
    }

    if(!MasterUID()){
        CheckFriendType_CtrlByMonster(nUID, fnOp);
        return;
    }

    // has a master
    // can be its master

    if(nUID == MasterUID()){
        fnOp(FT_FRIEND);
        return;
    }

    // has a master
    // check the final master

    QueryFinalMaster(UID(), [this, nUID, fnOp](uint64_t nFMasterUID)
    {
        if(!nFMasterUID){
            // TODO monster can swith master
            // then here we may incorrectly kill the monster
            fnOp(FT_ERROR);
            GoDie();
            return;
        }

        switch(UIDFunc::GetUIDType(nFMasterUID)){
            case UID_PLY:
                {
                    CheckFriendType_CtrlByPlayer(nUID, fnOp);
                    return;
                }
            case UID_MON:
                {
                    CheckFriendType_CtrlByMonster(nUID, fnOp);
                    return;
                }
            default:
                {
                    throw std::runtime_error(str_fflprintf(": Monster can't be controlled by type: %s", UIDFunc::GetUIDTypeString(nFMasterUID)));
                }
        }
    });
}

void Monster::QueryFriendType(uint64_t nUID, uint64_t nTargetUID, std::function<void(int)> fnOp)
{
    if(!(nUID && nTargetUID)){
        throw std::invalid_argument(str_fflprintf(": Invalid UID: %" PRIu64 ", %" PRIu64, nUID, nTargetUID));
    }

    AMQueryFriendType stAMQFT;
    std::memset(&stAMQFT, 0, sizeof(stAMQFT));

    stAMQFT.UID = nTargetUID;

    m_ActorPod->Forward(nUID, {MPK_QUERYFRIENDTYPE, stAMQFT}, [fnOp](const MessagePack &rstMPK)
    {
        switch(rstMPK.Type()){
            case MPK_FRIENDTYPE:
                {
                    AMFriendType stAMFT;
                    std::memcpy(&stAMFT, rstMPK.Data(), sizeof(stAMFT));

                    switch(stAMFT.Type){
                        case FT_ERROR:
                        case FT_ENEMY:
                        case FT_FRIEND:
                        case FT_NEUTRAL:
                            {
                                fnOp(stAMFT.Type);
                                return;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Invalid friend type: %d", stAMFT.Type));
                            }
                    }
                }
            default:
                {
                    fnOp(FT_NEUTRAL);
                    return;
                }
        }
    });
}

bool Monster::IsGuard(uint64_t nUID)
{
    if(UIDFunc::GetUIDType(nUID) != UID_MON){
        return false;
    }

    switch(UIDFunc::GetMonsterID(nUID)){
        case DBCOM_MONSTERID(u8"大刀卫士"):
        case DBCOM_MONSTERID(u8"弓箭卫士"):
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::IsPet(uint64_t nUID)
{
    if(UIDFunc::GetUIDType(nUID) != UID_MON){
        return false;
    }

    switch(UIDFunc::GetMonsterID(nUID)){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}
