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
{
    if(!m_monsterRecord){
        throw fflerror("invalid monster record: MonsterID = %llu", to_llu(monsterID()));
    }

    m_HP    = m_monsterRecord.HP;
    m_HPMax = m_monsterRecord.HP;
    m_MP    = m_monsterRecord.MP;
    m_MPMax = m_monsterRecord.MP;
}

bool Monster::randomMove()
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
                        dispatchAction(makeActionStand());

                        // we won't do reportStand() for monster
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

bool Monster::randomTurn()
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
                dispatchAction(makeActionStand());
                return true;
            }
        }
    }
    return false;
}

void Monster::attackUID(uint64_t nUID, int nDC, std::function<void()> fnOnOK, std::function<void()> fnOnError)
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

                                setTarget(nUID);
                                m_lastAttackTime = g_monoServer->getCurrTick();
                                dispatchAction(ActionAttack
                                {
                                    .speed = AttackSpeed(),
                                    .x = X(),
                                    .y = Y(),
                                    .aimUID = nUID,
                                    .damageID = DC_PHY_PLAIN,
                                });

                                // 2. send attack message to target
                                //    target can ignore this message directly
                                //
                                //    For mir2 code, at the time when monster attacks
                                //    1. immediately change target CO's HP and MP, but don't report
                                //    2. delay 550ms, then report RM_ATTACK with CO's new HP and MP
                                //    3. target CO reports to client for motion change (_MOTION_HITTED) and new HP/MP
                                addDelay(550, [this, nUID]()
                                {
                                    // monster may go dead after this delay
                                    // but don't check canAttack() since that's for attack lock
                                    if(true){
                                        dispatchAttack(nUID, DC_PHY_PLAIN);
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

        removeTarget(nUID);
        RemoveInViewCO(nUID);
    });
}

void Monster::trackUID(uint64_t nUID, int nMinCDistance, std::function<void()> fnOnOK, std::function<void()> fnOnError)
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

void Monster::followMaster(std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!(masterUID() && canMove())){
        fnOnError();
        return;
    }

    // followMaster works almost like trackUID(), but
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
                            dispatchAction(makeActionStand());
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
        else{
            // long distance
            // need to do spacemove or even mapswitch

            const auto [nBackX, nBackY] = fnGetBack(nX, nY, nDirection, 3);
            if(nMapID == MapID()){
                return requestSpaceMove(nBackX, nBackY, false, fnOnOK, fnOnError);
            }
            else{
                return requestMapSwitch(nMapID, nBackX, nBackY, false, fnOnOK, fnOnError);
            }
        }
    });
}

void Monster::trackAttackUID(uint64_t nTargetUID, std::function<void()> fnOnOK, std::function<void()> fnOnError)
{
    if(!nTargetUID){
        throw fflerror("invalid zero UID");
    }

    // TODO choose proper DC
    // for different monster it may use different DC

    int nProperDC = m_monsterRecord.DCList()[0];
    int nMinCDistance = 1;

    trackUID(nTargetUID, nMinCDistance, [nTargetUID, nProperDC, fnOnOK, fnOnError, this]()
    {
        attackUID(nTargetUID, nProperDC, fnOnOK, fnOnError);
    }, fnOnError);
}

corof::long_jmper Monster::updateCoroFunc()
{
    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_getProperTarget()){
            co_await coro_trackAttackUID(targetUID);
        }

        else if(masterUID()){
            if(m_actorPod->checkUIDValid(masterUID())){
                co_await coro_followMaster();
            }
            else{
                break;
            }
        }
        else{
            co_await coro_randomMove();
        }
    }

    goDie();
    co_return true;
}

bool Monster::update()
{
    if(HP() < 0){
        return goDie();
    }

    if(masterUID() && !m_actorPod->checkUIDValid(masterUID())){
        return goDie();
    }

    if(!m_updateCoro.valid() || m_updateCoro.poll_one()){
        m_updateCoro = updateCoroFunc();
    }
    return true;
}

void Monster::operateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                on_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_CHECKMASTER:
            {
                on_MPK_CHECKMASTER(rstMPK);
                break;
            }
        case MPK_QUERYFINALMASTER:
            {
                on_MPK_QUERYFINALMASTER(rstMPK);
                break;
            }
        case MPK_QUERYFRIENDTYPE:
            {
                on_MPK_QUERYFRIENDTYPE(rstMPK);
                break;
            }
        case MPK_QUERYNAMECOLOR:
            {
                on_MPK_QUERYNAMECOLOR(rstMPK);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                on_MPK_NOTIFYNEWCO(rstMPK);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                on_MPK_DEADFADEOUT(rstMPK);
                break;
            }
        case MPK_NOTIFYDEAD:
            {
                on_MPK_NOTIFYDEAD(rstMPK);
                break;
            }
        case MPK_UPDATEHP:
            {
                on_MPK_UPDATEHP(rstMPK);
                break;
            }
        case MPK_EXP:
            {
                on_MPK_EXP(rstMPK);
                break;
            }
        case MPK_MISS:
            {
                on_MPK_MISS(rstMPK);
                break;
            }
        case MPK_ACTION:
            {
                on_MPK_ACTION(rstMPK);
                break;
            }
        case MPK_ATTACK:
            {
                on_MPK_ATTACK(rstMPK);
                break;
            }
        case MPK_MAPSWITCH:
            {
                on_MPK_MAPSWITCH(rstMPK);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                on_MPK_QUERYLOCATION(rstMPK);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                on_MPK_QUERYCORECORD(rstMPK);
                break;
            }
        case MPK_BADACTORPOD:
            {
                on_MPK_BADACTORPOD(rstMPK);
                break;
            }
        case MPK_OFFLINE:
            {
                on_MPK_OFFLINE(rstMPK);
                break;
            }
        case MPK_MASTERKILL:
            {
                on_MPK_MASTERKILL(rstMPK);
                break;
            }
        case MPK_MASTERHITTED:
            {
                on_MPK_MASTERHITTED(rstMPK);
                break;
            }
        default:
            {
                throw fflerror("Unsupported message: %s", mpkName(rstMPK.Type()));
            }
    }
}

void Monster::SearchViewRange()
{
}

void Monster::reportCO(uint64_t toUID)
{
    if(!toUID || toUID == UID()){
        return;
    }

    AMCORecord amCOR;
    std::memset(&amCOR, 0, sizeof(amCOR));

    amCOR.UID = UID();
    amCOR.MapID = MapID();
    amCOR.action = ActionStand
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    };

    amCOR.Monster.MonsterID = monsterID();
    m_actorPod->forward(toUID, {MPK_CORECORD, amCOR});
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
        return g_monoServer->getCurrTick() >= m_lastMoveTime + m_monsterRecord.walkWait;
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

    if(nCurrTick < m_lastMoveTime + walkWait()){
        return false;
    }

    return nCurrTick >= m_lastAttackTime + m_monsterRecord.attackWait;
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

void Monster::removeTarget(uint64_t nUID)
{
    if(m_target.UID == nUID){
        m_target.UID = 0;
        m_target.activeTimer.reset();
    }
}

void Monster::setTarget(uint64_t nUID)
{
    m_target.UID = nUID;
    m_target.activeTimer.reset();
}

bool Monster::goDie()
{
    if(m_dead.get()){
        return true;
    }
    m_dead.set(true);

    randomDrop();
    dispatchOffenderExp();

    // dispatch die acton without auto-fade-out
    // server send the fade-out request in goGhost()

    // auto-fade-out is for zombie handling
    // when client confirms a zombie, client use auto-fade-out die action

    dispatchAction(ActionDie
    {
        .x = X(),
        .y = Y(),
        .fadeOut = false,
    });

    // let's dispatch ActionDie before mark it dead
    // theoratically dead actor shouldn't dispatch anything

    if(DBCOM_MONSTERRECORD(monsterID()).deadFadeOut){
        addDelay(2 * 1000, [this](){ goGhost(); });
        return true;
    }
    else{
        return goGhost();
    }
}

bool Monster::goGhost()
{
    if(!m_dead.get()){
        return false;
    }

    AMDeadFadeOut amDFO;
    std::memset(&amDFO, 0, sizeof(amDFO));

    amDFO.UID   = UID();
    amDFO.MapID = MapID();
    amDFO.X     = X();
    amDFO.Y     = Y();

    // send this to remove the map grid coverage
    // for monster don't need fadeout (like Taodog) we shouldn't send the FADEOUT to client

    if(checkActorPod() && m_map && m_map->checkActorPod()){
        m_actorPod->forward(m_map->UID(), {MPK_DEADFADEOUT, amDFO});
    }

    deactivate();
    return true;
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
        dispatchHealth();

        if(HP() <= 0){
            goDie();
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

    auto pathNodePtr = std::make_shared<scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3>>();
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

        auto minPathNodePtr = std::make_shared<scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3>>();
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

    AMPathFind amPF;
    std::memset(&amPF, 0, sizeof(amPF));

    amPF.UID     = UID();
    amPF.MapID   = MapID();
    amPF.CheckCO = 1;
    amPF.MaxStep = MaxStep();
    amPF.X       = X();
    amPF.Y       = Y();
    amPF.EndX    = nX;
    amPF.EndY    = nY;

    return m_actorPod->forward(MapUID(), {MPK_PATHFIND, amPF}, [this, nX, nY, fnOnOK, fnOnError](const MessagePack &rstRMPK)
    {
        switch(rstRMPK.Type()){
            case MPK_PATHFINDOK:
                {
                    AMPathFindOK amPFOK;
                    std::memcpy(&amPFOK, rstRMPK.Data(), sizeof(amPFOK));

                    constexpr auto nNodeCount = std::extent<decltype(amPFOK.Point)>::value;
                    static_assert(nNodeCount >= 2);

                    auto pBegin = amPFOK.Point;
                    auto pEnd   = amPFOK.Point + nNodeCount;

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

                    requestMove(amPFOK.Point[1].X, amPFOK.Point[1].Y, MoveSpeed(), false, false, fnOnOK, fnOnError);
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

void Monster::randomDrop()
{
    for(auto &rstGroupRecord: DB_MONSTERDROPITEM(monsterID())){
        for(auto &rstItemRecord: rstGroupRecord.second){
            if(std::rand() % rstItemRecord.ProbRecip == 0){
                AMNewDropItem amNDI;
                amNDI.UID   = UID();
                amNDI.X     = X();
                amNDI.Y     = Y();
                amNDI.ID    = rstItemRecord.ID;
                amNDI.Value = rstItemRecord.Value;

                // suggest server map to add a new drop item, but server map
                // may reject this suggestion silently.
                //
                // and if we are not in group-0
                // break if we select the first one item
                m_actorPod->forward(m_map->UID(), {MPK_NEWDROPITEM, amNDI});
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

void Monster::getProperTarget(std::function<void(uint64_t)> fnTarget)
{
    if(m_target.UID && m_target.activeTimer.diff_sec() < 60){
        checkFriend(m_target.UID, [targetUID = m_target.UID, fnTarget, this](int nFriendType)
        {
            if(nFriendType == FT_ENEMY){
                fnTarget(targetUID);
                return;
            }

            // may changed monster
            // last target is not a target anymore

            removeTarget(targetUID);
            SearchNearestTarget(fnTarget);
        });
    }
    else{
        removeTarget(m_target.UID);
        SearchNearestTarget(fnTarget);
    }
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
                    AMUID amUID;
                    std::memcpy(&amUID, rstRMPK.Data(), sizeof(amUID));

                    fnOp(amUID.UID);
                    return;
                }
            default:
                {
                    fnOp(0);
                    if(nUID == masterUID()){
                        goDie();
                    }
                    return;
                }
        }
    });
}

void Monster::checkFriend_AsGuard(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!isGuard(UID())){
        throw fflerror("invalid call to checkFriend_AsGuard");
    }

    switch(uidf::getUIDType(nUID)){
        case UID_MON:
            {
                fnOp(isGuard(nUID) ? FT_NEUTRAL : FT_ENEMY);
                return;
            }
        case UID_PLY:
            {
                m_actorPod->forward(nUID, MPK_QUERYNAMECOLOR, [fnOp](const MessagePack &rstMPK)
                {
                    switch(rstMPK.Type()){
                        case MPK_NAMECOLOR:
                            {
                                AMNameColor amNC;
                                std::memcpy(&amNC, rstMPK.Data(), sizeof(amNC));

                                switch(amNC.Color){
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
                if(!DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).tamable){
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
                if(!(isPet(nUID) || DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).tamable)){
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

    if(isGuard(UID())){
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
            goDie();
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

    AMQueryFriendType amQFT;
    std::memset(&amQFT, 0, sizeof(amQFT));

    amQFT.UID = nTargetUID;

    m_actorPod->forward(nUID, {MPK_QUERYFRIENDTYPE, amQFT}, [fnOp](const MessagePack &rstMPK)
    {
        switch(rstMPK.Type()){
            case MPK_FRIENDTYPE:
                {
                    AMFriendType amFT;
                    std::memcpy(&amFT, rstMPK.Data(), sizeof(amFT));

                    switch(amFT.Type){
                        case FT_ERROR:
                        case FT_ENEMY:
                        case FT_FRIEND:
                        case FT_NEUTRAL:
                            {
                                fnOp(amFT.Type);
                                return;
                            }
                        default:
                            {
                                throw fflerror("invalid friend type: %d", amFT.Type);
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

bool Monster::isGuard(uint64_t nUID)
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

bool Monster::isPet(uint64_t nUID)
{
    if(uidf::getUIDType(nUID) != UID_MON){
        return false;
    }

    switch(uidf::getMonsterID(nUID)){
        case DBCOM_MONSTERID(u8"变异骷髅"):
        case DBCOM_MONSTERID(u8"神兽"):
            {
                return true;
            }
        default:
            {
                return false;
            }
    }
}
