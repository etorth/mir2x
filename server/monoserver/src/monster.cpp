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
#include "uidf.hpp"
#include "strf.hpp"
#include "dbconst.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"
#include "netdriver.hpp"
#include "actorpod.hpp"
#include "mathf.hpp"
#include "memorypn.hpp"
#include "sysconst.hpp"
#include "friendtype.hpp"
#include "randompick.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "messagepack.hpp"
#include "protocoldef.hpp"
#include "serverargparser.hpp"

extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

Monster::AStarCache::AStarCache()
    : Time(0)
    , MapID(0)
    , Path()
{}

bool Monster::AStarCache::Retrieve(int *pX, int *pY, int nX0, int nY0, int nX1, int nY1, uint32_t nMapID)
{
    if(g_monoServer->getCurrTick() >= (Time + Refresh)){
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

    Time = g_monoServer->getCurrTick();
}

Monster::Monster(uint32_t   nMonsterID,
        ServiceCore        *pServiceCore,
        ServerMap          *pServerMap,
        int                 nMapX,
        int                 nMapY,
        int                 nDirection,
        uint64_t            nMasterUID)
    : CharObject(pServiceCore, pServerMap, uidf::buildMonsterUID(nMonsterID), nMapX, nMapY, nDirection)
    , m_monsterID(nMonsterID)
    , m_masterUID(nMasterUID)
    , m_monsterRecord(DBCOM_MONSTERRECORD(nMonsterID))
    , m_AStarCache()
    , m_bvTree()
    , m_updateCoro([this](){ UpdateCoroFunc(); })
{
    if(!m_monsterRecord){
        g_monoServer->addLog(LOGTYPE_WARNING, "Invalid monster record: MonsterID = %d", (int)(MonsterID()));
        g_monoServer->Restart();
    }

    SetState(STATE_DEAD    , 0);
    SetState(STATE_NEVERDIE, 0);

    // set attack mode
    // SetState(STATE_ATTACKMODE, STATE_ATTACKMODE_NORMAL);
    SetState(STATE_ATTACKMODE, STATE_ATTACKMODE_ATTACKALL);

    m_HP    = m_monsterRecord.HP;
    m_HPMax = m_monsterRecord.HP;
    m_MP    = m_monsterRecord.MP;
    m_MPMax = m_monsterRecord.MP;
}

bool Monster::RandomMove()
{
    if(canMove()){
        auto fnMoveOneStep = [this]() -> bool
        {
            int nX = -1;
            int nY = -1;
            if(OneStepReach(Direction(), 1, &nX, &nY) == 1){
                return requestMove(nX, nY, MoveSpeed(), false, false);
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
                        m_direction = nDirection;
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
                m_direction = dir;
                DispatchAction(ActionStand(X(), Y(), Direction()));
                return true;
            }
        }
    }
    return false;
}

void Monster::AttackUID(uint64_t nUID, int nDC, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!canAttack()){
        fnOnError();
        return;
    }

    if(!DCValid(nDC, true)){
        fnOnError();
        return;
    }

    // retrieving could schedule location query
    // before response received we can't allow any attack request

    m_attackLock = true;
    retrieveLocation(nUID, [this, nDC, nUID, fnOnOK, fnOnError](const COLocation &stCOLocation)
    {
        if(!m_attackLock){
            throw fflerror("attackLock released before location query done");
        }
        m_attackLock = false;

        auto nX = stCOLocation.X;
        auto nY = stCOLocation.Y;

        switch(nDC){
            case DC_PHY_PLAIN:
                {
                    switch(mathf::LDistance2(X(), Y(), nX, nY)){
                        case 1:
                        case 2:
                            {
                                m_direction = PathFind::GetDirection(X(), Y(), nX, nY);
                                if(!canAttack()){
                                    fnOnError();
                                    return;
                                }

                                SetTarget(nUID);
                                m_lastAttackTime = g_monoServer->getCurrTick();
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
                                    // but don't check canAttack() since that's for attack lock
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
        m_attackLock = false;

        RemoveTarget(nUID);
        RemoveInViewCO(nUID);
    });
}

void Monster::TrackUID(uint64_t nUID, int nMinCDistance, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(nMinCDistance < 1){
        throw fflerror("invalid distance: %d", nMinCDistance);
    }

    retrieveLocation(nUID, [this, nMinCDistance, fnOnOK, fnOnError](const COLocation &rstCOLocation) -> bool
    {
        auto nX     = rstCOLocation.X;
        auto nY     = rstCOLocation.Y;
        auto nMapID = rstCOLocation.MapID;

        if(!m_map->In(nMapID, nX, nY)){
            fnOnError();
            return false;
        }

        if(mathf::CDistance(X(), Y(), nX, nY) <= nMinCDistance){
            fnOnOK();
            return true;
        }

        MoveOneStep(nX, nY, fnOnOK, fnOnError);
        return true;
    }, fnOnError);
}

void Monster::FollowMaster(std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!(masterUID() && canMove())){
        fnOnError();
        return;
    }

    // followMaster works almost like TrackUID(), but
    // 1. follower always try to stand at the back of the master
    // 2. when distance is too far or master is on different map, follower takes space move

    retrieveLocation(masterUID(), [this, fnOnOK, fnOnError](const COLocation &rstCOLocation) -> bool
    {
        // check if it's still my master?
        // possible during the location query master changed

        if(rstCOLocation.UID != masterUID()){
            fnOnError();
            return false;
        }

        if(!canMove()){
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

        if((nMapID == MapID()) && (mathf::LDistance<double>(nX, nY, X(), Y()) < 10.0)){

            // not that long
            // slave should move step by step

            auto [nBackX, nBackY] = fnGetBack(nX, nY, nDirection, 1);
            switch(mathf::LDistance2(nBackX, nBackY, X(), Y())){
                case 0:
                    {
                        // already get there
                        // need to make a turn if needed

                        if(Direction() != nDirection){
                            m_direction= nDirection;
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
        return requestSpaceMove(nBackX, nBackY, false, fnOnOK, fnOnError);
    });
}

void Monster::TrackAttackUID(uint64_t nTargetUID, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!nTargetUID){
        throw fflerror("invalid zero UID");
    }

    // TODO choose proper DC
    // for different monster it may use different DC

    int nProperDC = m_monsterRecord.DCList()[0];
    int nMinCDistance = 1;

    TrackUID(nTargetUID, nMinCDistance, [nTargetUID, nProperDC, fnOnOK, fnOnError, this]()
    {
        AttackUID(nTargetUID, nProperDC, fnOnOK, fnOnError);
    }, fnOnError);
}

uint64_t Monster::Activate()
{
    if(auto nUID = CharObject::Activate()){
        if(masterUID()){
            m_actorPod->forward(masterUID(), {MPK_CHECKMASTER}, [this](const MessagePack &rstRMPK)
            {
                if(rstRMPK.Type() != MPK_OK){
                    GoDie();
                }
            });
        }
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

        else if(masterUID()){
            if(m_actorPod->CheckInvalid(masterUID())){
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

bool Monster::update()
{
    if(HP() < 0){
        return GoDie();
    }

    if(masterUID() && m_actorPod->CheckInvalid(masterUID())){
        return GoDie();
    }

    if(g_serverArgParser->useBvTree){
        switch(auto nState = m_bvTree->update()){
            case BV_PENDING:
                {
                    return true;
                }
            case BV_ABORT:
            case BV_SUCCESS:
            case BV_FAILURE:
                {
                    m_bvTree->reset();
                    return true;
                }
            default:
                {
                    throw fflerror(": Invalid node status: %d", nState);
                }
        }
    }

    if(!m_updateCoro.is_done() && m_updateCoro.in_main()){
        m_updateCoro.coro_resume();
    }
    return true;
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
        case MPK_MASTERKILL:
            {
                On_MPK_MASTERKILL(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
                g_monoServer->Restart();
                break;
            }
    }
}

void Monster::SearchViewRange()
{
}

void Monster::ReportCORecord(uint64_t toUID)
{
    if(!toUID || toUID == UID()){
        return;
    }

    AMCORecord stAMCOR;
    std::memset(&stAMCOR, 0, sizeof(stAMCOR));

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

    stAMCOR.Monster.MonsterID = MonsterID();
    m_actorPod->forward(toUID, {MPK_CORECORD, stAMCOR});
}

bool Monster::InRange(int nRangeType, int nX, int nY)
{
    if(true
            && m_map
            && m_map->ValidC(nX, nY)){
        switch(nRangeType){
            case RANGE_VISIBLE:
                {
                    return mathf::LDistance2(X(), Y(), nX, nY) < 20 * 20;
                }
            case RANGE_ATTACK:
                {
                    // inside this range
                    // monster will decide to make an attack
                    return mathf::LDistance2(X(), Y(), nX, nY) < 10 * 10;
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
                return {UID(), nDC, m_monsterRecord.DC + std::rand() % (1 + (std::max<int>)(m_monsterRecord.DCMax - m_monsterRecord.DC, 0)), EC_NONE};
            }
        case DC_MAG_FIRE:
            {
                return {UID(), nDC, m_monsterRecord.MDC + std::rand() % (1 + (std::max<int>)(m_monsterRecord.MDCMax - m_monsterRecord.MDC, 0)), EC_FIRE};
            }
        default:
            {
                return {};
            }
    }
}

bool Monster::canMove()
{
    if(CharObject::canMove() && CanAct()){
        return g_monoServer->getCurrTick() >= m_lastMoveTime + m_monsterRecord.WalkWait;
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

bool Monster::canAttack()
{
    if(!CanAct()){
        return false;
    }

    if(!CharObject::canAttack()){
        return false;
    }

    auto nCurrTick = g_monoServer->getCurrTick();

    if(nCurrTick < m_lastMoveTime + 800){
        return false;
    }

    return nCurrTick >= m_lastAttackTime + m_monsterRecord.AttackWait;
}

bool Monster::DCValid(int nDC, bool bCheck)
{
    if(std::find(m_monsterRecord.DCList().begin(), m_monsterRecord.DCList().end(), nDC) != m_monsterRecord.DCList().end()){
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
    if(m_target.UID == nUID){
        m_target = {};
    }
}

void Monster::SetTarget(uint64_t nUID)
{
    m_target.UID = nUID;
    m_target.ActiveTime = g_monoServer->getCurrTick();
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
                                    && m_map
                                    && m_map->ActorPodValid()){
                                m_actorPod->forward(m_map->UID(), {MPK_DEADFADEOUT, stAMDFO});
                            }

                            // 2. deactivate the actor here
                            //    disable the actorpod then no source can drive it
                            //    then current *this* can't be refered by any actor threads after this invocation
                            //    then MonoServer::EraseUID() is safe to delete *this*
                            //
                            //    don't do delete m_actorPod to disable the actor
                            //    since currently we are in the actor thread which accquired by m_actorPod
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
    switch(uidf::getMonsterID(UID())){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                if(masterUID()){
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
    if(!canMove()){
        fnOnError();
        return false;
    }

    switch(estimateHop(nX, nY)){
        case 0:
            {
                fnOnError();
                return false;
            }
        case 1:
            {
                if(OneStepCost(nullptr, 1, X(), Y(), nX, nY) >= 0.00){
                    return requestMove(nX, nY, MoveSpeed(), false, false, fnOnOK, fnOnError);
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
            return requestMove(nXm, nYm, MoveSpeed(), false, false, fnOnOK, fnOnError);
        }
    }

    switch(FindPathMethod()){
        case FPMETHOD_ASTAR:
            {
                return MoveOneStepAStar(nX, nY, fnOnOK, fnOnError);
            }
        case FPMETHOD_DSTAR:
            {
                return MoveOneStepDStar(nX, nY, fnOnOK, fnOnError);
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
    if(!canMove()){
        fnOnError();
        return false;
    }

    CharObject::COPathFinder stFinder(this, 1);
    if(!stFinder.Search(X(), Y(), nX, nY)){
        fnOnError();
        return false;
    }

    const auto [stPathNode, nNodeNum] = stFinder.GetFirstNPathNode<5>();
    if(nNodeNum < 2){
        throw fflerror("incorrect pathnode number: %zu", nNodeNum);
    }

    m_AStarCache.Cache({stPathNode.begin(), stPathNode.begin() + nNodeNum}, MapID());
    return requestMove(stPathNode[1].X, stPathNode[1].Y, MoveSpeed(), false, false, fnOnOK, fnOnError);
}

bool Monster::MoveOneStepGreedy(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!canMove()){
        fnOnError();
        return false;
    }

    auto pathNodePtr = std::make_shared<svobuf<PathFind::PathNode, 3>>();
    const bool longJump = (MaxStep() > 1) && (mathf::CDistance(X(), Y(), nX, nY) >= MaxStep());
    GetValidChaseGrid(nX, nY, longJump ? MaxStep() : 1, *(pathNodePtr.get()));

    if(pathNodePtr->c.size() > 3){
        throw fflerror("invalid chase grid result: size = %zu", pathNodePtr->c.size());
    }

    if(pathNodePtr->c.empty()){
        fnOnError();
        return false;
    }

    const auto fnOnNodeListError = [nX, nY, longJump, fnOnOK, fnOnError, this]()
    {
        if(!longJump){
            fnOnError();
            return;
        }

        auto minPathNodePtr = std::make_shared<svobuf<PathFind::PathNode, 3>>();
        GetValidChaseGrid(nX, nY, 1, *(minPathNodePtr.get()));

        if(minPathNodePtr->c.size() > 3){
            throw fflerror("invalid chase grid result: size = %zu", minPathNodePtr->c.size());
        }

        if(minPathNodePtr->c.empty()){
            fnOnError();
            return;
        }

        requestMove(minPathNodePtr->c[0].X, minPathNodePtr->c[0].Y, MoveSpeed(), false, false, fnOnOK, [this, minPathNodePtr, fnOnOK, fnOnError]()
        {
            if(minPathNodePtr->c.size() == 1){
                fnOnError();
                return;
            }

            requestMove(minPathNodePtr->c[1].X, minPathNodePtr->c[1].Y, MoveSpeed(), false, false, fnOnOK, [this, minPathNodePtr, fnOnOK, fnOnError]()
            {
                if(minPathNodePtr->c.size() == 2){
                    fnOnError();
                    return;
                }

                requestMove(minPathNodePtr->c[2].X, minPathNodePtr->c[2].Y, MoveSpeed(), false, false, fnOnOK, fnOnError);
            });
        });
    };

    return requestMove(pathNodePtr->c[0].X, pathNodePtr->c[0].Y, MoveSpeed(), false, false, fnOnOK, [this, longJump, nX, nY, pathNodePtr, fnOnOK, fnOnNodeListError]()
    {
        if(pathNodePtr->c.size() == 1){
            fnOnNodeListError();
            return;
        }

        requestMove(pathNodePtr->c[1].X, pathNodePtr->c[1].Y, MoveSpeed(), false, false, fnOnOK, [this, longJump, nX, nY, pathNodePtr, fnOnOK, fnOnNodeListError]()
        {
            if(pathNodePtr->c.size() == 2){
                fnOnNodeListError();
                return;
            }

            requestMove(pathNodePtr->c[2].X, pathNodePtr->c[2].Y, MoveSpeed(), false, false, fnOnOK, [this, longJump, nX, nY,fnOnOK, fnOnNodeListError]()
            {
                fnOnNodeListError();
            });
        });
    });
}

bool Monster::MoveOneStepCombine(int nX, int nY, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!canMove()){
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
    if(!canMove()){
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

    return m_actorPod->forward(MapUID(), {MPK_PATHFIND, stAMPF}, [this, nX, nY, fnOnOK, fnOnError](const MessagePack &rstRMPK)
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
                        if(m_map->groundValid(pCurr->X, pCurr->Y)){
                            stvPathNode.emplace_back(pCurr->X, pCurr->Y);
                        }else{
                            break;
                        }
                    }

                    if(!stvPathNode.back().Eq(nX, nY)){
                        stvPathNode.emplace_back(nX, nY);
                    }
                    m_AStarCache.Cache(stvPathNode, MapID());

                    requestMove(stAMPFOK.Point[1].X, stAMPFOK.Point[1].Y, MoveSpeed(), false, false, fnOnOK, fnOnError);
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

bool Monster::MoveOneStepDStar(int, int, std::function<void()>, std::function<void()>)
{
    throw fflerror("Not supported now");
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
                m_actorPod->forward(m_map->UID(), {MPK_NEWDROPITEM, stAMNDI});
                if(rstGroupRecord.first != 0){
                    break;
                }
            }
        }
    }
}

void Monster::RecursiveCheckInViewTarget(size_t nIndex, std::function<void(uint64_t)> fnTarget)
{
    if(nIndex >= m_inViewCOList.size()){
        fnTarget(0);
        return;
    }

    auto nUID = m_inViewCOList[nIndex].UID;
    checkFriend(nUID, [this, nIndex, nUID, fnTarget](int nFriendType)
    {
        // when reach here
        // m_inViewCOList[nIndex] may not be nUID anymore

        // if changed
        // we'd better redo the search

        if(nIndex >= m_inViewCOList.size() || m_inViewCOList[nIndex].UID != nUID){
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
    if(m_inViewCOList.empty()){
        fnTarget(0);
        return;
    }
    RecursiveCheckInViewTarget(0, fnTarget);
}

void Monster::GetProperTarget(std::function<void(uint64_t)> fnTarget)
{
    if(m_target.UID){
        if(g_monoServer->getCurrTick() < m_target.ActiveTime + 60 * 1000){
            checkFriend(m_target.UID, [nTargetUID = m_target.UID, fnTarget, this](int nFriendType)
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

    RemoveTarget(m_target.UID);
    SearchNearestTarget(fnTarget);
}

void Monster::CreateBvTree()
{
    bvarg_ref nTargetUID;

    m_bvTree = bvtree::if_branch
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
    m_bvTree->reset();
}

void Monster::QueryMaster(uint64_t nUID, std::function<void(uint64_t)> fnOp)
{
    if(!nUID){
        throw fflerror("invalid zero UID");
    }

    if(nUID == UID()){
        throw fflerror("query self for masterUID()");
    }

    m_actorPod->forward(nUID, MPK_QUERYMASTER, [this, nUID, fnOp](const MessagePack &rstRMPK)
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
                    if(nUID == masterUID()){
                        GoDie();
                    }
                    return;
                }
        }
    });
}

void Monster::checkFriend_AsGuard(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!IsGuard(UID())){
        throw fflerror("invalid call to checkFriend_AsGuard");
    }

    switch(uidf::getUIDType(nUID)){
        case UID_MON:
            {
                fnOp(IsGuard(nUID) ? FT_NEUTRAL : FT_ENEMY);
                return;
            }
        case UID_PLY:
            {
                m_actorPod->forward(nUID, MPK_QUERYNAMECOLOR, [fnOp](const MessagePack &rstMPK)
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
                throw fflerror("invalid UID type: %s", uidf::getUIDTypeString(nUID));
            }
    }
}

void Monster::checkFriend_CtrlByMonster(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(masterUID() && uidf::getUIDType(masterUID()) != UID_MON){
        throw fflerror("invalid call to checkFriend_CtrlByMonster");
    }

    switch(uidf::getUIDType(nUID)){
        case UID_PLY:
            {
                fnOp(FT_ENEMY);
                return;
            }
        case UID_MON:
            {
                if(!DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).Tamable){
                    fnOp(FT_NEUTRAL);
                    return;
                }

                QueryFinalMaster(nUID, [nUID, fnOp](uint64_t nFMasterUID)
                {
                    if(!nFMasterUID){
                        fnOp(FT_ERROR);
                        return;
                    }

                    switch(uidf::getUIDType(nFMasterUID)){
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
                                throw fflerror("invalid final master type: %s", uidf::getUIDTypeString(nFMasterUID));
                            }
                    }
                });
                return;
            }
        case UID_NPC:
            {
                return;
            }
        default:
            {
                throw fflerror("invalid UID type: %s", uidf::getUIDTypeString(nUID));
            }
    }
}

void Monster::checkFriend_CtrlByPlayer(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(uidf::getUIDType(masterUID()) != UID_PLY){
        throw fflerror("invalid call to checkFriend_CtrlByPlayer");
    }

    switch(uidf::getUIDType(nUID)){
        case UID_MON:
            {
                if(!(IsPet(nUID) || DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).Tamable)){
                    fnOp(FT_ENEMY);
                    return;
                }

                QueryFinalMaster(nUID, [this, nUID, fnOp](uint64_t nFMasterUID)
                {
                    if(!nFMasterUID){
                        fnOp(FT_ERROR);
                        return;
                    }

                    switch(uidf::getUIDType(nFMasterUID)){
                        case UID_MON:
                            {
                                fnOp(FT_ENEMY);
                                return;
                            }
                        case UID_PLY:
                            {
                                if(nFMasterUID == masterUID()){
                                    fnOp(FT_NEUTRAL);
                                    return;
                                }

                                QueryFriendType(masterUID(), nUID, [fnOp](int nFriendType)
                                {
                                    fnOp(nFriendType);
                                });
                                return;
                            }
                        default:
                            {
                                throw fflerror("invalid final master type: %s", uidf::getUIDTypeString(nFMasterUID));
                            }
                    }
                });
                return;
            }
        case UID_PLY:
            {
                QueryFriendType(masterUID(), nUID, [fnOp](int nFriendType)
                {
                    fnOp(nFriendType);
                });
                return;
            }
        default:
            {
                throw fflerror("invalid call to checkFriend_CtrlByPlayer");
            }
    }
}

void Monster::checkFriend(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!nUID){
        throw fflerror("invalid zero UID");
    }

    if(nUID == UID()){
        throw fflerror("check friend type to self");
    }

    if(uidf::getUIDType(nUID) == UID_NPC){
        fnOp(FT_NEUTRAL);
        return;
    }

    // 1. 大刀卫士 or 弓箭卫士
    // 2. no master or master is still monster
    // 3. as pet, master is UID_PLY

    if(IsGuard(UID())){
        checkFriend_AsGuard(nUID, fnOp);
        return;
    }

    if(!masterUID()){
        checkFriend_CtrlByMonster(nUID, fnOp);
        return;
    }

    // has a master
    // can be its master

    if(nUID == masterUID()){
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

        switch(uidf::getUIDType(nFMasterUID)){
            case UID_PLY:
                {
                    checkFriend_CtrlByPlayer(nUID, fnOp);
                    return;
                }
            case UID_MON:
                {
                    checkFriend_CtrlByMonster(nUID, fnOp);
                    return;
                }
            default:
                {
                    throw fflerror("Monster can't be controlled by type: %s", uidf::getUIDTypeString(nFMasterUID));
                }
        }
    });
}

void Monster::QueryFriendType(uint64_t nUID, uint64_t nTargetUID, std::function<void(int)> fnOp)
{
    if(!(nUID && nTargetUID)){
        throw fflerror("invalid UID: %" PRIu64 ", %" PRIu64, nUID, nTargetUID);
    }

    AMQueryFriendType stAMQFT;
    std::memset(&stAMQFT, 0, sizeof(stAMQFT));

    stAMQFT.UID = nTargetUID;

    m_actorPod->forward(nUID, {MPK_QUERYFRIENDTYPE, stAMQFT}, [fnOp](const MessagePack &rstMPK)
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
                                throw fflerror("invalid friend type: %d", stAMFT.Type);
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
    if(uidf::getUIDType(nUID) != UID_MON){
        return false;
    }

    switch(uidf::getMonsterID(nUID)){
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
    if(uidf::getUIDType(nUID) != UID_MON){
        return false;
    }

    switch(uidf::getMonsterID(nUID)){
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
