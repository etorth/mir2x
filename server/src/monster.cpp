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
#include "mathf.hpp"
#include "pathf.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"
#include "netdriver.hpp"
#include "actorpod.hpp"
#include "actormsg.hpp"
#include "sysconst.hpp"
#include "friendtype.hpp"
#include "randompick.hpp"
#include "monoserver.hpp"
#include "dbcomrecord.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"
#include "dropitemconfig.hpp"
#include "serverargparser.hpp"

extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

Monster::AStarCache::AStarCache()
    : Time(0)
    , mapID(0)
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

    if((nMapID == mapID) && (Path.size() >= 3)){
        auto fnFindIndex = [this](int nX, int nY) -> int
        {
            for(int nIndex = 0; nIndex < to_d(Path.size()); ++nIndex){
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
    mapID = nMapID;
    Path.swap(stvPathNode);

    Time = g_monoServer->getCurrTick();
}

Monster::Monster(uint32_t monID,
        const ServerMap  *mapCPtr,
        int               mapX,
        int               mapY,
        int               direction,
        uint64_t          masterUID)
    : CharObject(mapCPtr, uidf::buildMonsterUID(monID), mapX, mapY, direction)
    , m_masterUID(masterUID)
{
    if(!getMR()){
        throw fflerror("invalid monster record: MonsterID = %llu", to_llu(monsterID()));
    }

    m_sdHealth.uid   = UID();
    m_sdHealth.HP    = getMR().hp;
    m_sdHealth.MP    = getMR().mp;
    m_sdHealth.maxHP = getMR().hp;
    m_sdHealth.maxMP = getMR().mp;
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

            auto nDirCount = to_d(std::extent<decltype(nDirV)>::value);
            auto nDirStart = to_d(std::rand() % nDirCount);

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

    const auto dirCount = to_d(std::extent<decltype(dirs)>::value);
    const auto dirStart = to_d(std::rand() % dirCount);

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

void Monster::attackUID(uint64_t nUID, int nDC, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canAttack()){
        onError();
        return;
    }

    if(!dcValid(nDC, true)){
        onError();
        return;
    }

    // retrieving could schedule location query
    // before response received we can't allow any attack request

    m_attackLock = true;
    getCOLocation(nUID, [this, nDC, nUID, onOK, onError](const COLocation &coLoc)
    {
        fflassert(m_attackLock);
        m_attackLock = false;

        const auto &mr = DBCOM_MAGICRECORD(nDC);
        if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
            if(onError){
                onError();
            }
            return;
        }

        if(const auto newDir = PathFind::GetDirection(X(), Y(), coLoc.x, coLoc.y); directionValid(newDir)){
            m_direction = newDir;
        }

        if(!canAttack()){
            if(onError){
                onError();
            }
            return;
        }

        dispatchAction(ActionAttack
        {
            .speed = attackSpeed(),
            .x = X(),
            .y = Y(),
            .aimUID = nUID,
            .damageID = to_u32(nDC),
        });

        // send attack message to target
        // target can ignore this message directly
        //
        // For mir2 code, at the time when monster attacks
        //   1. immediately change target CO's HP and MP, but don't report
        //   2. delay 550ms, then report RM_ATTACK with CO's new HP and MP
        //   3. target CO reports to client for motion change (_MOTION_HITTED) and new HP/MP

        addDelay(550, [this, nUID, nDC]()
        {
            dispatchAttackDamage(nUID, nDC);
        });

        if(onOK){
            onOK();
        }
    },

    [this, nUID, onError]()
    {
        m_attackLock = false;
        m_inViewCOList.erase(nUID);

        if(onError){
            onError();
        }
    });
}

void Monster::jumpUID(uint64_t targetUID, std::function<void()> onOK, std::function<void()> onError)
{
    getCOLocation(targetUID, [this, onOK, onError](const COLocation &coLoc)
    {
        const auto nX     = coLoc.x;
        const auto nY     = coLoc.y;
        const auto nDir   = directionValid(coLoc.direction) ? coLoc.direction : DIR_UP;
        const auto nMapID = coLoc.mapID;

        if(!m_map->In(nMapID, nX, nY)){
            onError();
            return;
        }

        // default stand and direction when jump to an UID
        // this way keeps distance and has best opporitunity to attack if UID is moving forward
        //
        // +---+---+---+
        // |   |   |  /|
        // |   |   |L  |
        // +---+---+---+
        // |   | ^ |   |
        // |   | | |   |
        // +---+---+---+
        // |   |   |   |
        // |   |   |   |
        // +---+---+---+

        const auto nextDir = pathf::nextDirection(nDir, 1);
        const auto [nFrontX, nFrontY] = pathf::getFrontGLoc(nX, nY, nextDir, 1);
        if(!m_map->groundValid(nFrontX, nFrontY)){
            onError();
            return;
        }

        if(nFrontX == X() && nFrontY == Y()){
            onOK();
        }
        else{
            requestJump(nFrontX, nFrontY, PathFind::GetBack(nextDir), onOK, onError);
        }
    }, onError);
}

void Monster::trackUID(uint64_t nUID, DCCastRange r, std::function<void()> onOK, std::function<void()> onError)
{
    getCOLocation(nUID, [this, r, onOK, onError](const COLocation &coLoc)
    {
        if(!m_map->In(coLoc.mapID, coLoc.x, coLoc.y)){
            if(onError){
                onError();
            }
            return;
        }

        // patch the track function, r can be provided as {}
        // if r provided as invalid, we accept it as to follow the uid generally

        if(r){
            if(pathf::inDCCastRange(r, X(), Y(), coLoc.x, coLoc.y)){
                if(onOK){
                    onOK();
                }
            }
            else{
                MoveOneStep(coLoc.x, coLoc.y, onOK, onError);
            }
        }
        else{
            if((X() == coLoc.x) && (Y() == coLoc.y)){
                if(onOK){
                    onOK();
                }
            }
            else{
                MoveOneStep(coLoc.x, coLoc.y, onOK, onError);
            }
        }
    }, onError);
}

void Monster::followMaster(std::function<void()> onOK, std::function<void()> onError)
{
    if(!(masterUID() && canMove())){
        onError();
        return;
    }

    // followMaster works almost like trackUID(), but
    // 1. follower always try to stand at the back of the master
    // 2. when distance is too far or master is on different map, follower takes space move

    getCOLocation(masterUID(), [this, onOK, onError](const COLocation &coLoc) -> bool
    {
        // check if it's still my master?
        // possible during the location query master changed

        if(coLoc.uid != masterUID()){
            onError();
            return false;
        }

        if(!canMove()){
            onError();
            return false;
        }

        auto nX         = coLoc.x;
        auto nY         = coLoc.y;
        auto nMapID     = coLoc.mapID;
        auto nDirection = coLoc.direction;

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

        if((nMapID == mapID()) && (mathf::LDistance<double>(nX, nY, X(), Y()) < 10.0)){

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

                        onOK();
                        return true;
                    }
                default:
                    {
                        return MoveOneStep(nBackX, nBackY, onOK, onError);
                    }
            }
        }
        else{
            // long distance
            // need to do spacemove or even mapswitch

            const auto [nBackX, nBackY] = fnGetBack(nX, nY, nDirection, 3);
            if(nMapID == mapID()){
                return requestSpaceMove(nBackX, nBackY, false, onOK, onError);
            }
            else{
                return requestMapSwitch(nMapID, nBackX, nBackY, false, onOK, onError);
            }
        }
    });
}

void Monster::jumpAttackUID(uint64_t targetUID, std::function<void()> onOK, std::function<void()> onError)
{
    if(!targetUID){
        throw fflerror("invalid zero UID");
    }

    const auto magicID = getAttackMagic(targetUID);
    const auto &mr = DBCOM_MAGICRECORD(magicID);
    if(!mr){
        if(onError){
            onError();
        }
        return;
    }

    jumpUID(targetUID, [targetUID, magicID, onOK, onError, this]()
    {
        attackUID(targetUID, magicID, onOK, onError);
    }, onError);
}

void Monster::trackAttackUID(uint64_t targetUID, std::function<void()> onOK, std::function<void()> onError)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    const auto magicID = getAttackMagic(targetUID);
    const auto &mr = DBCOM_MAGICRECORD(magicID);
    if(!mr){
        if(onError){
            onError();
        }
        return;
    }

    trackUID(targetUID, mr.castRange, [targetUID, magicID, onOK, onError, this]()
    {
        attackUID(targetUID, magicID, onOK, onError);
    }, onError);
}

corof::eval_poller Monster::updateCoroFunc()
{
    uint64_t targetUID = 0;
    hres_timer targetActiveTimer;

    while(m_sdHealth.HP > 0){
        if(targetUID && targetActiveTimer.diff_sec() > SYS_TARGETSEC){
            targetUID = 0;
        }

        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            targetUID = 0;
            m_inViewCOList.erase(targetUID);
        }

        if(targetUID){
            const auto [argMapID, argX, argY] = co_await coro_getCOGLoc(targetUID);
            if(!inView(argMapID, argX, argY)){
                targetUID = 0;
                m_inViewCOList.erase(targetUID);
            }
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
            if(targetUID){
                targetActiveTimer.reset();
            }
        }

        if(targetUID){
            if(co_await coro_trackAttackUID(targetUID)){
                targetActiveTimer.reset();
            }
        }
        else if(masterUID()){
            if(m_actorPod->checkUIDValid(masterUID())){
                co_await coro_followMaster();
            }
            else{
                break;
            }
        }
        else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
            co_await coro_randomMove();
            m_actorPod->setMetronomeFreq(10);
        }
        else{
            m_actorPod->setMetronomeFreq(1); // don't set freq as 0, the UpdateCoro is driven by METRONOME
        }

        // always wait
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

bool Monster::update()
{
    if(m_sdHealth.HP < 0){
        return goDie();
    }

    if(masterUID() && !m_actorPod->checkUIDValid(masterUID())){
        return goDie();
    }

    if(!m_updateCoro.valid() || m_updateCoro.poll()){
        m_updateCoro = updateCoroFunc();
    }
    return true;
}

void Monster::operateAM(const ActorMsgPack &rstMPK)
{
    switch(rstMPK.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(rstMPK);
                break;
            }
        case AM_CHECKMASTER:
            {
                on_AM_CHECKMASTER(rstMPK);
                break;
            }
        case AM_QUERYFINALMASTER:
            {
                on_AM_QUERYFINALMASTER(rstMPK);
                break;
            }
        case AM_QUERYFRIENDTYPE:
            {
                on_AM_QUERYFRIENDTYPE(rstMPK);
                break;
            }
        case AM_QUERYNAMECOLOR:
            {
                on_AM_QUERYNAMECOLOR(rstMPK);
                break;
            }
        case AM_QUERYHEALTH:
            {
                on_AM_QUERYHEALTH(rstMPK);
                break;
            }
        case AM_NOTIFYNEWCO:
            {
                on_AM_NOTIFYNEWCO(rstMPK);
                break;
            }
        case AM_DEADFADEOUT:
            {
                on_AM_DEADFADEOUT(rstMPK);
                break;
            }
        case AM_NOTIFYDEAD:
            {
                on_AM_NOTIFYDEAD(rstMPK);
                break;
            }
        case AM_UPDATEHP:
            {
                on_AM_UPDATEHP(rstMPK);
                break;
            }
        case AM_EXP:
            {
                on_AM_EXP(rstMPK);
                break;
            }
        case AM_MISS:
            {
                on_AM_MISS(rstMPK);
                break;
            }
        case AM_HEAL:
            {
                on_AM_HEAL(rstMPK);
                break;
            }
        case AM_ACTION:
            {
                on_AM_ACTION(rstMPK);
                break;
            }
        case AM_ATTACK:
            {
                on_AM_ATTACK(rstMPK);
                break;
            }
        case AM_MAPSWITCHTRIGGER:
            {
                on_AM_MAPSWITCHTRIGGER(rstMPK);
                break;
            }
        case AM_QUERYLOCATION:
            {
                on_AM_QUERYLOCATION(rstMPK);
                break;
            }
        case AM_QUERYCORECORD:
            {
                on_AM_QUERYCORECORD(rstMPK);
                break;
            }
        case AM_BADACTORPOD:
            {
                on_AM_BADACTORPOD(rstMPK);
                break;
            }
        case AM_OFFLINE:
            {
                on_AM_OFFLINE(rstMPK);
                break;
            }
        case AM_MASTERKILL:
            {
                on_AM_MASTERKILL(rstMPK);
                break;
            }
        case AM_MASTERHITTED:
            {
                on_AM_MASTERHITTED(rstMPK);
                break;
            }
        default:
            {
                throw fflerror("Unsupported message: %s", mpkName(rstMPK.type()));
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
    amCOR.mapID = mapID();
    amCOR.action = makeActionStand();
    amCOR.Monster.MonsterID = monsterID();
    m_actorPod->forward(toUID, {AM_CORECORD, amCOR});
}

DamageNode Monster::getAttackDamage(int dc) const
{
    switch(dc){
        case DBCOM_MAGICID(u8"物理攻击"):
            {
                return PlainPhyDamage
                {
                    .damage = mathf::rand<int>(getMR().dc[0], getMR().dc[1]),
                    .dcHit = getMR().dcHit,
                };
            }
        default:
            {
                return MagicDamage
                {
                    .magicID = dc,
                    .damage = mathf::rand<int>(getMR().mc[0], getMR().mc[1]),
                    .mcHit = getMR().mcHit,
                };
            }
    }
}

bool Monster::canMove() const
{
    if(!CharObject::canMove()){
        return false;
    }

    return g_monoServer->getCurrTick() >= std::max<uint32_t>(
    {
        m_lastActionTime.at(m_lastAction    ) + 100,
        m_lastActionTime.at(ACTION_JUMP     ) + 100,
        m_lastActionTime.at(ACTION_SPACEMOVE) + 100,
        m_lastActionTime.at(ACTION_MOVE     ) + getMR().walkWait,
        m_lastActionTime.at(ACTION_ATTACK   ) + getMR().attackWait,
    });
}

// should have a better way for GCD (global cooldown)
// because for actions with gfx frames, client takes time to present
// we should give client a chance to show all its animation before dispatch action result

// i.e.:
// monster moves to (x, y) and attacks player
// if there is no GCD, it jumps to (x, y) and immediately attack player and dispatch the attack result
// from client we see monster is just start moving, but player already get the ACTION_HITTED

bool Monster::canAttack() const
{
    if(!CharObject::canAttack()){
        return false;
    }

    return g_monoServer->getCurrTick() >= std::max<uint32_t>(
    {
        m_lastActionTime.at(m_lastAction ) + 100,
        m_lastActionTime.at(ACTION_MOVE  ) + getMR().walkWait,
        m_lastActionTime.at(ACTION_ATTACK) + getMR().attackWait,
    });
}

bool Monster::dcValid(int, bool)
{
    return true;
}

bool Monster::goDie()
{
    if(m_dead.get()){
        return true;
    }

    m_dead.set(true);
    dispatchOffenderExp();

    for(auto &item: getMonsterDropItemList(monsterID())){
        m_actorPod->forward(m_map->UID(), {AM_DROPITEM, cerealf::serialize(SDDropItem
        {
            .x = X(),
            .y = Y(),
            .item = std::move(item),
        })});
    }

    // dispatch die acton without auto-fade-out
    // server send the fade-out request in goGhost()

    // auto-fade-out is for zombie handling
    // when client confirms a zombie, client use auto-fade-out die action

    dispatchAction(ActionDie
    {
        .x = X(),
        .y = Y(),
    });

    // let's dispatch ActionDie before mark it dead
    // theoratically dead actor shouldn't dispatch anything

    if(getMR().deadFadeOut){
        addDelay(1000, [this]() { goGhost(); });
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
    amDFO.mapID = mapID();
    amDFO.X     = X();
    amDFO.Y     = Y();

    // send this to remove the map grid coverage
    // for monster don't need fadeout (like Taodog) we shouldn't send the FADEOUT to client

    if(checkActorPod() && m_map && m_map->checkActorPod()){
        m_actorPod->forward(m_map->UID(), {AM_DEADFADEOUT, amDFO});
    }

    deactivate();
    return true;
}

bool Monster::struckDamage(const DamageNode &node)
{
    if(node){
        const bool phyDC = (to_u32(node.magicID) == DBCOM_MAGICID(u8"物理攻击"));
        const auto hitProb = [phyDC, &node, this]() -> double
        {
            if(phyDC){
                return mathf::sigmoid(to_df(node.dcHit - getMR().dcDodge) / 2.5) / 2.0 + 0.5;
            }
            else{
                return mathf::sigmoid(to_df(node.mcHit - getMR().mcDodge) / 2.5) / 2.0 + 0.5;
            }
        }();

        if(mathf::rand<double>(0.0, 1.0) > hitProb){
            return true;
        }

        const auto damage = [phyDC, &node, this]() -> int
        {
            if(phyDC){
                return std::max<int>(0, node.damage - mathf::rand<int>(getMR().ac[0], getMR().ac[1]));
            }

            const double elemRatio = std::max<double>(0.0, 1.0 + 0.1 * [&node, this]() -> int
            {
                const auto &mr = DBCOM_MAGICRECORD(node.magicID);
                fflassert(mr);

                switch(magicElemID(mr.elem)){
                    case MET_FIRE   : return getMR().acElem.fire;
                    case MET_ICE    : return getMR().acElem.ice;
                    case MET_LIGHT  : return getMR().acElem.light;
                    case MET_WIND   : return getMR().acElem.wind;
                    case MET_HOLY   : return getMR().acElem.holy;
                    case MET_DARK   : return getMR().acElem.dark;
                    case MET_PHANTOM: return getMR().acElem.phantom;
                    default         : return 0;
                }
            }());
            return std::max<int>(0, node.damage - std::lround(mathf::rand<int>(getMR().mac[0], getMR().mac[1]) * elemRatio));
        }();

        if(damage > 0){
            m_sdHealth.HP = std::max<int>(0, m_sdHealth.HP - damage);
            dispatchHealth();

            if(m_sdHealth.HP <= 0){
                goDie();
            }
        }
        return true;
    }
    return false;
}

bool Monster::MoveOneStep(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        onError();
        return false;
    }

    switch(estimateHop(nX, nY)){
        case 0:
            {
                onError();
                return false;
            }
        case 1:
            {
                if(OneStepCost(nullptr, 1, X(), Y(), nX, nY) >= 0.00){
                    return requestMove(nX, nY, MoveSpeed(), false, false, onOK, onError);
                }
                break;
            }
        case 2:
            {
                break;
            }
        default:
            {
                onError();
                return false;
            }
    }

    int nXm = -1;
    int nYm = -1;

    if(m_AStarCache.Retrieve(&nXm, &nYm, X(), Y(), nX, nY, mapID())){
        if(OneStepCost(nullptr, 1, X(), Y(), nXm, nYm) >= 0.00){
            return requestMove(nXm, nYm, MoveSpeed(), false, false, onOK, onError);
        }
    }

    switch(FindPathMethod()){
        case FPMETHOD_ASTAR:
            {
                return MoveOneStepAStar(nX, nY, onOK, onError);
            }
        case FPMETHOD_DSTAR:
            {
                return MoveOneStepDStar(nX, nY, onOK, onError);
            }
        case FPMETHOD_GREEDY:
            {
                return MoveOneStepGreedy(nX, nY, onOK, onError);
            }
        case FPMETHOD_COMBINE:
            {
                return MoveOneStepCombine(nX, nY, onOK, onError);
            }
        case FPMETHOD_NEIGHBOR:
            {
                return MoveOneStepNeighbor(nX, nY, onOK, onError);
            }
        default:
            {
                onError();
                return false;
            }
    }
}

bool Monster::MoveOneStepNeighbor(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        onError();
        return false;
    }

    CharObject::COPathFinder stFinder(this, 1);
    if(!stFinder.Search(X(), Y(), nX, nY)){
        onError();
        return false;
    }

    const auto [stPathNode, nNodeNum] = stFinder.GetFirstNPathNode<5>();
    if(nNodeNum < 2){
        throw fflerror("incorrect pathnode number: %zu", nNodeNum);
    }

    m_AStarCache.Cache({stPathNode.begin(), stPathNode.begin() + nNodeNum}, mapID());
    return requestMove(stPathNode[1].X, stPathNode[1].Y, MoveSpeed(), false, false, onOK, onError);
}

bool Monster::MoveOneStepGreedy(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        onError();
        return false;
    }

    auto pathNodePtr = std::make_shared<scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3>>();
    const bool longJump = (MaxStep() > 1) && (mathf::CDistance(X(), Y(), nX, nY) >= MaxStep());
    GetValidChaseGrid(nX, nY, longJump ? MaxStep() : 1, *(pathNodePtr.get()));

    if(pathNodePtr->c.size() > 3){
        throw fflerror("invalid chase grid result: size = %zu", pathNodePtr->c.size());
    }

    if(pathNodePtr->c.empty()){
        onError();
        return false;
    }

    const auto fnOnNodeListError = [nX, nY, longJump, onOK, onError, this]()
    {
        if(!longJump){
            onError();
            return;
        }

        auto minPathNodePtr = std::make_shared<scoped_alloc::svobuf_wrapper<PathFind::PathNode, 3>>();
        GetValidChaseGrid(nX, nY, 1, *(minPathNodePtr.get()));

        if(minPathNodePtr->c.size() > 3){
            throw fflerror("invalid chase grid result: size = %zu", minPathNodePtr->c.size());
        }

        if(minPathNodePtr->c.empty()){
            onError();
            return;
        }

        requestMove(minPathNodePtr->c[0].X, minPathNodePtr->c[0].Y, MoveSpeed(), false, false, onOK, [this, minPathNodePtr, onOK, onError]()
        {
            if(minPathNodePtr->c.size() == 1){
                onError();
                return;
            }

            requestMove(minPathNodePtr->c[1].X, minPathNodePtr->c[1].Y, MoveSpeed(), false, false, onOK, [this, minPathNodePtr, onOK, onError]()
            {
                if(minPathNodePtr->c.size() == 2){
                    onError();
                    return;
                }

                requestMove(minPathNodePtr->c[2].X, minPathNodePtr->c[2].Y, MoveSpeed(), false, false, onOK, onError);
            });
        });
    };

    return requestMove(pathNodePtr->c[0].X, pathNodePtr->c[0].Y, MoveSpeed(), false, false, onOK, [this, longJump, nX, nY, pathNodePtr, onOK, fnOnNodeListError]()
    {
        if(pathNodePtr->c.size() == 1){
            fnOnNodeListError();
            return;
        }

        requestMove(pathNodePtr->c[1].X, pathNodePtr->c[1].Y, MoveSpeed(), false, false, onOK, [this, longJump, nX, nY, pathNodePtr, onOK, fnOnNodeListError]()
        {
            if(pathNodePtr->c.size() == 2){
                fnOnNodeListError();
                return;
            }

            requestMove(pathNodePtr->c[2].X, pathNodePtr->c[2].Y, MoveSpeed(), false, false, onOK, [this, longJump, nX, nY,onOK, fnOnNodeListError]()
            {
                fnOnNodeListError();
            });
        });
    });
}

bool Monster::MoveOneStepCombine(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        onError();
        return false;
    }

    return MoveOneStepGreedy(nX, nY, onOK, [this, nX, nY, onOK, onError]()
    {
        MoveOneStepNeighbor(nX, nY, onOK, onError);
    });
}

bool Monster::MoveOneStepAStar(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        onError();
        return false;
    }

    AMPathFind amPF;
    std::memset(&amPF, 0, sizeof(amPF));

    amPF.UID     = UID();
    amPF.mapID   = mapID();
    amPF.CheckCO = 1;
    amPF.MaxStep = MaxStep();
    amPF.X       = X();
    amPF.Y       = Y();
    amPF.EndX    = nX;
    amPF.EndY    = nY;

    return m_actorPod->forward(mapUID(), {AM_PATHFIND, amPF}, [this, nX, nY, onOK, onError](const ActorMsgPack &rstRMPK)
    {
        switch(rstRMPK.type()){
            case AM_PATHFINDOK:
                {
                    AMPathFindOK amPFOK;
                    std::memcpy(&amPFOK, rstRMPK.data(), sizeof(amPFOK));

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
                    m_AStarCache.Cache(stvPathNode, mapID());

                    requestMove(amPFOK.Point[1].X, amPFOK.Point[1].Y, MoveSpeed(), false, false, onOK, onError);
                    break;
                }
            default:
                {
                    onError();
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

void Monster::searchNearestTargetHelper(std::unordered_set<uint64_t> seen, std::function<void(uint64_t)> fnTarget)
{
    const auto viewDistance = DBCOM_MONSTERRECORD(monsterID()).view;
    if(viewDistance <= 0){
        fnTarget(0);
        return;
    }

    int minDistance = INT_MAX;
    uint64_t minDistanceUID = 0;

    if(seen.empty()){
        seen.reserve(m_inViewCOList.size() + 8);
    }

    for(const auto &[uid, coLoc]: m_inViewCOList){
        if(!seen.count(uid)){
            if(const auto currDistance = mathf::LDistance2<int>(X(), Y(), coLoc.x, coLoc.y); (currDistance <= viewDistance * viewDistance) && (currDistance < minDistance)){
                minDistance    = currDistance;
                minDistanceUID = uid;
            }
        }
    }

    if(minDistanceUID){
        checkFriend(minDistanceUID, [this, minDistanceUID, seen = std::move(seen), fnTarget = std::move(fnTarget)](int friendType) mutable
        {
            if(friendType == FT_ENEMY){
                fnTarget(minDistanceUID);
                return;
            }

            seen.insert(minDistanceUID);
            searchNearestTargetHelper(std::move(seen), std::move(fnTarget));
        });
    }
    else{
        fnTarget(0);
    }
}

void Monster::searchNearestTarget(std::function<void(uint64_t)> fnTarget)
{
    if(m_inViewCOList.empty()){
        fnTarget(0);
        return;
    }

    // TODO check if offender is still on same map
    //      and if too far monster should pick up a target near it
    for(auto p = m_offenderList.rbegin(); p != m_offenderList.rend(); ++p){
        if(m_actorPod->checkUIDValid(p->uid)){
            fnTarget(p->uid);
            return;
        }
    }

    if(DBCOM_MONSTERRECORD(monsterID()).behaveMode == BM_NEUTRAL){
        fnTarget(0);
        return;
    }

    searchNearestTargetHelper({}, std::move(fnTarget));
}

void Monster::pickTarget(std::function<void(uint64_t)> fnTarget)
{
    // can have different strategy to pick target: nearest, latest, weakest
    //
    searchNearestTarget(std::move(fnTarget));
}

int Monster::getAttackMagic(uint64_t) const
{
    const auto &mr = DBCOM_MONSTERRECORD(monsterID());
    fflassert(mr);
    return DBCOM_MAGICID(str_haschar(mr.dcName) ? mr.dcName : u8"物理攻击");
}

void Monster::queryMaster(uint64_t targetUID, std::function<void(uint64_t)> fnOp)
{
    fflassert(targetUID);
    if(targetUID == UID()){
        if(fnOp){
            fnOp(masterUID() ? masterUID() : UID());
        }
    }
    else{
        m_actorPod->forward(targetUID, AM_QUERYMASTER, [this, targetUID, fnOp](const ActorMsgPack &rmpk)
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

                        if(targetUID == masterUID()){
                            goDie();
                        }
                        return;
                    }
            }
        });
    }
}

void Monster::checkFriend_ctrlByMonster(uint64_t targetUID, std::function<void(int)> fnOp)
{
    fflassert(targetUID);
    if(masterUID()){
        fflassert(uidf::isMonster(masterUID()));
    }

    switch(uidf::getUIDType(targetUID)){
        case UID_PLY:
            {
                if(fnOp){
                    fnOp(FT_ENEMY);
                }
                return;
            }
        case UID_MON:
            {
                queryFinalMaster(targetUID, [targetUID, fnOp](uint64_t finalMasterUID)
                {
                    if(finalMasterUID){
                        switch(uidf::getUIDType(finalMasterUID)){
                            case UID_MON:
                                {
                                    if(fnOp){
                                        fnOp(FT_NEUTRAL);
                                    }
                                    return;
                                }
                            case UID_PLY:
                                {
                                    if(fnOp){
                                        fnOp(FT_ENEMY);
                                    }
                                    return;
                                }
                            default:
                                {
                                    throw bad_value(finalMasterUID);
                                }
                        }
                    }
                    else{
                        if(fnOp){
                            fnOp(FT_ERROR);
                        }
                    }
                });
                return;
            }
        case UID_NPC:
            {
                if(fnOp){
                    fnOp(FT_NEUTRAL);
                }
                return;
            }
        default:
            {
                throw bad_value(targetUID);
            }
    }
}

void Monster::checkFriend_ctrlByPlayer(uint64_t targetUID, std::function<void(int)> fnOp)
{
    fflassert(targetUID);
    fflassert(masterUID());
    fflassert(uidf::isPlayer(masterUID()));

    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
            {
                queryFinalMaster(targetUID, [targetUID, fnOp, this](uint64_t finalMasterUID)
                {
                    if(finalMasterUID){
                        switch(uidf::getUIDType(finalMasterUID)){
                            case UID_MON:
                                {
                                    if(fnOp){
                                        fnOp(FT_ENEMY);
                                    }
                                    return;
                                }
                            case UID_PLY:
                                {
                                    if(finalMasterUID == masterUID()){
                                        if(fnOp){
                                            fnOp(FT_NEUTRAL);
                                        }
                                    }
                                    else{
                                        queryPlayerFriend(masterUID(), finalMasterUID, [fnOp](int friendType)
                                        {
                                            if(fnOp){
                                                fnOp(friendType);
                                            }
                                        });
                                    }
                                    return;
                                }
                            default:
                                {
                                    throw bad_value(finalMasterUID);
                                }
                        }
                    }
                    else{
                        if(fnOp){
                            fnOp(FT_ERROR);
                        }
                    }
                });
                return;
            }
        case UID_PLY:
            {
                queryPlayerFriend(masterUID(), targetUID, [fnOp](int friendType)
                {
                    if(fnOp){
                        fnOp(friendType);
                    }
                });
                return;
            }
        default:
            {
                throw bad_value(targetUID);
            }
    }
}

void Monster::checkFriend(uint64_t targetUID, std::function<void(int)> fnOp)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    if(uidf::getUIDType(targetUID) == UID_NPC){
        if(fnOp){
            fnOp(FT_NEUTRAL);
        }
        return;
    }

    if(!masterUID()){
        checkFriend_ctrlByMonster(targetUID, fnOp);
        return;
    }

    // has a master
    // can be its master

    if(targetUID == masterUID()){
        if(fnOp){
            fnOp(FT_FRIEND);
        }
        return;
    }

    // has a master
    // check the final master

    queryFinalMaster(UID(), [this, targetUID, fnOp](uint64_t finalMasterUID)
    {
        // TODO monster can swith master
        // then here we may incorrectly kill the monster

        if(!finalMasterUID){
            if(fnOp){
                fnOp(FT_ERROR);
            }

            goDie();
            return;
        }

        switch(uidf::getUIDType(finalMasterUID)){
            case UID_PLY:
                {
                    checkFriend_ctrlByPlayer(targetUID, fnOp);
                    return;
                }
            case UID_MON:
                {
                    checkFriend_ctrlByMonster(targetUID, fnOp);
                    return;
                }
            default:
                {
                    throw fflerror("invalid master uid: %s", to_cstr(uidf::getUIDString(finalMasterUID)));
                }
        }
    });
}

void Monster::queryPlayerFriend(uint64_t fromUID, uint64_t targetUID, std::function<void(int)> fnOp)
{
    // this function means:
    // ask fromUID: how do you feel about targetUID

    fflassert(uidf::isPlayer(fromUID));
    fflassert(uidf::isPlayer(targetUID));

    AMQueryFriendType amQFT;
    std::memset(&amQFT, 0, sizeof(amQFT));

    amQFT.UID = targetUID;
    m_actorPod->forward(fromUID, {AM_QUERYFRIENDTYPE, amQFT}, [fnOp](const ActorMsgPack &rmpk)
    {
        switch(rmpk.type()){
            case AM_FRIENDTYPE:
                {
                    const auto amFT = rmpk.conv<AMFriendType>();
                    switch(amFT.Type){
                        case FT_ERROR:
                        case FT_ENEMY:
                        case FT_FRIEND:
                        case FT_NEUTRAL:
                            {
                                if(fnOp){
                                    fnOp(amFT.Type);
                                }
                                return;
                            }
                        default:
                            {
                                throw bad_reach();
                            }
                    }
                }
            default:
                {
                    if(fnOp){
                        fnOp(FT_ERROR);
                    }
                    return;
                }
        }
    });
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

bool Monster::hasPlayerNeighbor() const
{
    for(const auto &[uid, coLoc]: m_inViewCOList){
        if(uidf::getUIDType(coLoc.uid) == UID_PLY){
            return true;
        }
    }
    return false;
}

void Monster::onAMAttack(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAttack>();
    if(amA.UID == UID()){
        return;
    }

    if(m_dead.get()){
        notifyDead(amA.UID);
        return;
    }

    checkFriend(amA.UID, [amA, this](int friendType)
    {
        switch(friendType){
            case FT_ENEMY:
                {
                    if(const auto &mr = DBCOM_MAGICRECORD(amA.damage.magicID); !pathf::inDCCastRange(mr.castRange, X(), Y(), amA.X, amA.Y)){
                        switch(uidf::getUIDType(amA.UID)){
                            case UID_MON:
                            case UID_PLY:
                                {
                                    AMMiss amM;
                                    std::memset(&amM, 0, sizeof(amM));

                                    amM.UID = amA.UID;
                                    m_actorPod->forward(amA.UID, {AM_MISS, amM});
                                    return;
                                }
                            default:
                                {
                                    return;
                                }
                        }
                    }

                    addOffenderDamage(amA.UID, amA.damage);
                    dispatchAction(ActionHitted
                    {
                        .x = X(),
                        .y = Y(),
                        .direction = Direction(),
                    });

                    struckDamage(amA.damage);
                    return;
                }
            default:
                {
                    return;
                }
        }
    });
}

void Monster::onAMMasterHitted(const ActorMsgPack &)
{
    // do nothing by default
}

void Monster::dispatchOffenderExp()
{
    for(auto p = m_offenderList.begin(); p != m_offenderList.end();){
        if(hres_tstamp().to_sec() >= p->activeTime + 120){
            p = m_offenderList.erase(p);
        }
        else{
            p++;
        }
    }

    if(m_offenderList.empty()){
        return;
    }

    const auto sumDamage = std::accumulate(m_offenderList.begin(), m_offenderList.end(), to_u64(0), [](uint64_t sum, const auto &offender) -> uint64_t
    {
        return sum + offender.damage;
    });

    for(const auto &offender: m_offenderList){
        switch(uidf::getUIDType(offender.uid)){
            case UID_MON:
            case UID_PLY:
                {
                    AMExp amE;
                    std::memset(&amE, 0, sizeof(amE));

                    amE.exp = std::max<int>(1, std::lround((to_df(offender.damage) / sumDamage) * getMR().exp));
                    m_actorPod->forward(offender.uid, {AM_EXP, amE});
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}

void Monster::addOffenderDamage(uint64_t nUID, int nDamage)
{
    fflassert(nUID);
    fflassert(nDamage >= 0);

    for(auto p = m_offenderList.begin(); p != m_offenderList.end(); ++p){
        if(p->uid == nUID){
            p->damage += to_u64(nDamage);
            p->activeTime = hres_tstamp().to_sec();
            return;
        }
    }

    m_offenderList.emplace_back(Offender
    {
        .uid = nUID,
        .damage = to_u64(nDamage),
        .activeTime = hres_tstamp().to_sec(),
    });
}
