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
#include "netdriver.hpp"
#include "actorpod.hpp"
#include "actormsg.hpp"
#include "sysconst.hpp"
#include "friendtype.hpp"
#include "monoserver.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"
#include "dropitemconfig.hpp"
#include "serverargparser.hpp"

extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

std::optional<pathf::PathNode> Monster::AStarCache::retrieve(uint32_t mapID, int srcX, int srcY, int dstX, int dstY)
{
    if(g_monoServer->getCurrTick() >= (m_time + m_refresh)){
        m_path.clear();
        return {};
    }

    // if cache doesn't match we won't clean it
    // only cleared by timeout

    if((m_mapID == mapID) && (m_path.size() >= 3)){
        const auto fnFindIndex = [this](int argX, int argY) -> int
        {
            for(int i = 0; i < to_d(m_path.size()); ++i){
                if(true
                        && m_path[i].X == argX
                        && m_path[i].Y == argY){
                    return i;
                }
            }
            return -1;
        };

        const auto srcIndex = fnFindIndex(srcX, srcY);
        const auto dstIndex = fnFindIndex(dstX, dstY);

        if(true
                && srcIndex >= 0
                && dstIndex >= srcIndex + 2){
            return pathf::PathNode
            {
                m_path[srcIndex + 1].X,
                m_path[srcIndex + 1].Y,
            };
        }
    }
    return {};
}

void Monster::AStarCache::cache(uint32_t mapID, std::vector<pathf::PathNode> nodeList)
{
    fflassert(!nodeList.empty());
    m_time = g_monoServer->getCurrTick();
    m_mapID = mapID;
    m_path.swap(nodeList);
}

Monster::Monster(uint32_t monID,
        const ServerMap  *mapCPtr,
        int               mapX,
        int               mapY,
        int               direction,
        uint64_t          masterUID)
    : BattleObject(mapCPtr, uidf::buildMonsterUID(monID), mapX, mapY, direction)
    , m_masterUID(masterUID)
{
    fflassert(getMR());
    m_sdHealth.uid   = UID();
    m_sdHealth.hp    = getMR().hp;
    m_sdHealth.mp    = getMR().mp;
    m_sdHealth.maxHP = getMR().hp;
    m_sdHealth.maxMP = getMR().mp;
}

bool Monster::randomMove()
{
    if(canMove()){
        auto fnMoveOneStep = [this]() -> bool
        {
            const auto reachRes = oneStepReach(Direction(), 1);
            if(!reachRes.has_value()){
                return false;
            }

            const auto [dstX, dstY, dstDist] = reachRes.value();
            if(dstDist != 1){
                return false;
            }
            return requestMove(dstX, dstY, moveSpeed(), false, false);
        };

        const auto fnMakeOneTurn = [this]() -> bool
        {
            for(int i = 0, startDir = pathf::getRandDir(); i < 8; ++i){
                if(const auto currDir = pathf::getNextDir(startDir, i); Direction() != currDir){
                    if(oneStepReach(currDir, 1).has_value()){
                        m_direction = currDir;
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
    for(int i = 0, startDir = pathf::getRandDir(); i < 8; ++i){
        if(const auto currDir = pathf::getNextDir(startDir, i); Direction() != currDir){
            if(oneStepReach(currDir, 1).has_value()){
                m_direction = currDir;
                dispatchAction(makeActionStand());
                return true;
            }
        }
    }
    return false;
}

void Monster::attackUID(uint64_t uid, int magicID, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canAttack()){
        if(onError){
            onError();
        }
        return;
    }

    if(!dcValid(magicID, true)){
        if(onError){
            onError();
        }
        return;
    }

    // retrieving could schedule location query
    // before response received we can't allow any attack request

    m_attackLock = true;
    getCOLocation(uid, [this, magicID, uid, onOK, onError](const COLocation &coLoc)
    {
        fflassert(m_attackLock);
        m_attackLock = false;

        const auto &mr = DBCOM_MAGICRECORD(magicID);
        if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
            if(onError){
                onError();
            }
            return;
        }

        if(const auto newDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(newDir)){
            m_direction = newDir;
        }

        if(!canAttack()){
            if(onError){
                onError();
            }
            return;
        }

        const auto [buffID, modifierID] = m_buffList.rollAttackModifier();
        dispatchAction(ActionAttack
        {
            .speed = attackSpeed(),
            .x = X(),
            .y = Y(),
            .aimUID = uid,
            .extParam
            {
                .magicID = to_u32(magicID),
                .modifierID = to_u32(modifierID),
            },
        });

        if(buffID){
            sendBuff(uid, 0, buffID);
        }

        // send attack message to target
        // target can ignore this message directly
        //
        // For mir2 code, at the time when monster attacks
        //   1. immediately change target CO's HP and MP, but don't report
        //   2. delay 550ms, then report RM_ATTACK with CO's new HP and MP
        //   3. target CO reports to client for motion change (_MOTION_HITTED) and new HP/MP

        addDelay(550, [this, uid, magicID, modifierID]()
        {
            dispatchAttackDamage(uid, magicID, modifierID);
        });

        if(onOK){
            onOK();
        }
    },

    [this, uid, onError]()
    {
        m_attackLock = false;
        m_inViewCOList.erase(uid);

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
        const auto nDir   = pathf::dirValid(coLoc.direction) ? coLoc.direction : DIR_UP;
        const auto nMapID = coLoc.mapID;

        if(!m_map->in(nMapID, nX, nY)){
            if(onError){
                onError();
            }
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

        const auto nextDir = pathf::getNextDir(nDir, 1);
        const auto [nFrontX, nFrontY] = pathf::getFrontGLoc(nX, nY, nextDir, 1);
        if(!m_map->groundValid(nFrontX, nFrontY)){
            if(onError){
                onError();
            }
            return;
        }

        if(nFrontX == X() && nFrontY == Y()){
            if(onOK){
                onOK();
            }
        }
        else{
            requestJump(nFrontX, nFrontY, pathf::getBackDir(nextDir), onOK, onError);
        }
    }, onError);
}

void Monster::trackUID(uint64_t nUID, DCCastRange r, std::function<void()> onOK, std::function<void()> onError)
{
    getCOLocation(nUID, [this, r, onOK, onError](const COLocation &coLoc)
    {
        if(!m_map->in(coLoc.mapID, coLoc.x, coLoc.y)){
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
                moveOneStep(coLoc.x, coLoc.y, onOK, onError);
            }
        }
        else{
            if((X() == coLoc.x) && (Y() == coLoc.y)){
                if(onOK){
                    onOK();
                }
            }
            else{
                moveOneStep(coLoc.x, coLoc.y, onOK, onError);
            }
        }
    }, onError);
}

void Monster::followMaster(std::function<void()> onOK, std::function<void()> onError)
{
    if(!(masterUID() && canMove())){
        if(onError){
            onError();
        }
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

        const auto nX         = coLoc.x;
        const auto nY         = coLoc.y;
        const auto nMapID     = coLoc.mapID;
        const auto nDirection = pathf::dirValid(coLoc.direction) ? coLoc.direction : [this]() -> int
        {
            switch(uidf::getUIDType(masterUID())){
                case UID_PLY: return pathf::getRandDir();
                default     : return DIR_BEGIN;
            }
        }();

        if((nMapID == mapID()) && (mathf::LDistance<double>(nX, nY, X(), Y()) < 10.0)){

            // not that long
            // slave should move step by step

            const auto [nBackX, nBackY] = pathf::getBackGLoc(nX, nY, nDirection, 1);
            switch(mathf::LDistance2(nBackX, nBackY, X(), Y())){
                case 0:
                    {
                        // already get there
                        // need to make a turn if needed

                        if(Direction() != nDirection){
                            m_direction = nDirection;
                            dispatchAction(makeActionStand());
                        }

                        if(onOK){
                            onOK();
                        }
                        return true;
                    }
                default:
                    {
                        return moveOneStep(nBackX, nBackY, onOK, onError);
                    }
            }
        }
        else{
            // long distance
            // need to do spacemove or even mapswitch

            const auto [nBackX, nBackY] = pathf::getBackGLoc(nX, nY, nDirection, 3);
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
    fflassert(targetUID);
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

    while(m_sdHealth.hp > 0){
        if(targetUID && targetActiveTimer.diff_sec() > SYS_TARGETSEC){
            targetUID = 0;
        }

        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
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
    if(m_sdHealth.hp < 0){
        return goDie();
    }

    if(masterUID() && !m_actorPod->checkUIDValid(masterUID())){
        return goDie();
    }

    if(m_buffList.update()){
        dispatchBuffIDList();
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
        case AM_ADDBUFF:
            {
                on_AM_ADDBUFF(rstMPK);
                break;
            }
        case AM_REMOVEBUFF:
            {
                on_AM_REMOVEBUFF(rstMPK);
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
        case AM_QUERYUIDBUFF:
            {
                on_AM_QUERYUIDBUFF(rstMPK);
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

DamageNode Monster::getAttackDamage(int dc, int modifierID) const
{
    switch(dc){
        case DBCOM_MAGICID(u8"物理攻击"):
            {
                return PlainPhyDamage
                {
                    .damage = mathf::rand<int>(getMR().dc[0], getMR().dc[1]),
                    .dcHit = getMR().dcHit,
                    .modifierID = modifierID,
                };
            }
        default:
            {
                return MagicDamage
                {
                    .magicID = dc,
                    .damage = mathf::rand<int>(getMR().mc[0], getMR().mc[1]),
                    .mcHit = getMR().mcHit,
                    .modifierID = modifierID,
                };
            }
    }
}

bool Monster::canMove() const
{
    if(!BattleObject::canMove()){
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
    if(!BattleObject::canAttack()){
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

    if(hasActorPod() && m_map && m_map->hasActorPod()){
        m_actorPod->forward(m_map->UID(), {AM_DEADFADEOUT, amDFO});
    }

    deactivate();
    return true;
}

bool Monster::struckDamage(uint64_t fromUID, const DamageNode &node)
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
            m_sdHealth.hp = std::max<int>(0, m_sdHealth.hp - damage);
            dispatchHealth();

            switch(node.modifierID){
                case DBCOM_ATTACKMODIFIERID(u8"吸血"):
                    {
                        AMHeal amH;
                        std::memset(&amH, 0, sizeof(amH));

                        amH.mapID = mapID();
                        amH.addHP = std::min<int>(damage, 20);

                        m_actorPod->forward(fromUID, {AM_HEAL, amH});
                        break;
                    }
                default:
                    {
                        break;
                    }
            }

            if(m_sdHealth.hp <= 0){
                goDie();
            }
        }
        return true;
    }
    return false;
}

bool Monster::moveOneStep(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    switch(estimateHop(nX, nY)){
        case 0:
            {
                if(onError){
                    onError();
                }
                return false;
            }
        case 1:
            {
                if(oneStepCost(nullptr, 1, X(), Y(), Direction(), nX, nY).has_value()){
                    return requestMove(nX, nY, moveSpeed(), false, false, onOK, onError);
                }
                break;
            }
        case 2:
            {
                break;
            }
        default:
            {
                if(onError){
                    onError();
                }
                return false;
            }
    }

    if(const auto nodeOpt = m_astarCache.retrieve(mapID(), X(), Y(), nX, nY); nodeOpt.has_value()){
        if(oneStepCost(nullptr, 1, X(), Y(), Direction(), nodeOpt.value().X, nodeOpt.value().Y).has_value()){
            return requestMove(nodeOpt.value().X, nodeOpt.value().Y, moveSpeed(), false, false, onOK, onError);
        }
    }

    switch(FindPathMethod()){
        case FPMETHOD_ASTAR:
            {
                return moveOneStepAStar(nX, nY, onOK, onError);
            }
        case FPMETHOD_GREEDY:
            {
                return moveOneStepGreedy(nX, nY, onOK, onError);
            }
        case FPMETHOD_COMBINE:
            {
                return moveOneStepCombine(nX, nY, onOK, onError);
            }
        case FPMETHOD_NEIGHBOR:
            {
                return moveOneStepNeighbor(nX, nY, onOK, onError);
            }
        default:
            {
                if(onError){
                    onError();
                }
                return false;
            }
    }
}

bool Monster::moveOneStepNeighbor(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    BattleObject::BOPathFinder stFinder(this, 1);
    if(!stFinder.search(X(), Y(), Direction(), nX, nY).hasPath()){
        if(onError){
            onError();
        }
        return false;
    }

    auto pathList = stFinder.getPathNode();
    const auto stPathNode = pathList.at(1);

    if(pathList.size() < 2){
        throw fflerror("incorrect pathnode number: %zu", pathList.size());
    }

    m_astarCache.cache(mapID(), std::move(pathList));
    return requestMove(stPathNode.X, stPathNode.Y, moveSpeed(), false, false, onOK, onError);
}

bool Monster::moveOneStepGreedy(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    auto pathNodePtr = std::make_shared<scoped_alloc::svobuf_wrapper<pathf::PathNode, 3>>();
    const bool longJump = (maxStep() > 1) && (mathf::CDistance(X(), Y(), nX, nY) >= maxStep());
    getValidChaseGrid(nX, nY, longJump ? maxStep() : 1, *(pathNodePtr.get()));

    if(pathNodePtr->c.size() > 3){
        throw fflerror("invalid chase grid result: size = %zu", pathNodePtr->c.size());
    }

    if(pathNodePtr->c.empty()){
        if(onError){
            onError();
        }
        return false;
    }

    const auto fnOnNodeListError = [nX, nY, longJump, onOK, onError, this]()
    {
        if(!longJump){
            if(onError){
                onError();
            }
            return;
        }

        auto minPathNodePtr = std::make_shared<scoped_alloc::svobuf_wrapper<pathf::PathNode, 3>>();
        getValidChaseGrid(nX, nY, 1, *(minPathNodePtr.get()));

        if(minPathNodePtr->c.size() > 3){
            throw fflerror("invalid chase grid result: size = %zu", minPathNodePtr->c.size());
        }

        if(minPathNodePtr->c.empty()){
            if(onError){
                onError();
            }
            return;
        }

        requestMove(minPathNodePtr->c[0].X, minPathNodePtr->c[0].Y, moveSpeed(), false, false, onOK, [this, minPathNodePtr, onOK, onError]()
        {
            if(minPathNodePtr->c.size() == 1){
                if(onError){
                    onError();
                }
                return;
            }

            requestMove(minPathNodePtr->c[1].X, minPathNodePtr->c[1].Y, moveSpeed(), false, false, onOK, [this, minPathNodePtr, onOK, onError]()
            {
                if(minPathNodePtr->c.size() == 2){
                    if(onError){
                        onError();
                    }
                    return;
                }

                requestMove(minPathNodePtr->c[2].X, minPathNodePtr->c[2].Y, moveSpeed(), false, false, onOK, onError);
            });
        });
    };

    return requestMove(pathNodePtr->c[0].X, pathNodePtr->c[0].Y, moveSpeed(), false, false, onOK, [this, longJump, nX, nY, pathNodePtr, onOK, fnOnNodeListError]()
    {
        if(pathNodePtr->c.size() == 1){
            fnOnNodeListError();
            return;
        }

        requestMove(pathNodePtr->c[1].X, pathNodePtr->c[1].Y, moveSpeed(), false, false, onOK, [this, longJump, nX, nY, pathNodePtr, onOK, fnOnNodeListError]()
        {
            if(pathNodePtr->c.size() == 2){
                fnOnNodeListError();
                return;
            }

            requestMove(pathNodePtr->c[2].X, pathNodePtr->c[2].Y, moveSpeed(), false, false, onOK, [this, longJump, nX, nY,onOK, fnOnNodeListError]()
            {
                fnOnNodeListError();
            });
        });
    });
}

bool Monster::moveOneStepCombine(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    return moveOneStepGreedy(nX, nY, onOK, [this, nX, nY, onOK, onError]()
    {
        moveOneStepNeighbor(nX, nY, onOK, onError);
    });
}

bool Monster::moveOneStepAStar(int nX, int nY, std::function<void()> onOK, std::function<void()> onError)
{
    if(!canMove()){
        if(onError){
            onError();
        }
        return false;
    }

    AMPathFind amPF;
    std::memset(&amPF, 0, sizeof(amPF));

    amPF.UID       = UID();
    amPF.mapID     = mapID();
    amPF.CheckCO   = 1;
    amPF.MaxStep   = maxStep();
    amPF.X         = X();
    amPF.Y         = Y();
    amPF.direction = Direction();
    amPF.EndX      = nX;
    amPF.EndY      = nY;

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

                    std::vector<pathf::PathNode> stvPathNode;
                    for(auto pCurr = pBegin; pCurr != pEnd; ++pCurr){
                        if(m_map->groundValid(pCurr->X, pCurr->Y)){
                            stvPathNode.emplace_back(pCurr->X, pCurr->Y);
                        }
                        else{
                            break;
                        }
                    }

                    if(!stvPathNode.back().eq(nX, nY)){
                        stvPathNode.emplace_back(nX, nY);
                    }

                    m_astarCache.cache(mapID(), std::move(stvPathNode));
                    requestMove(amPFOK.Point[1].X, amPFOK.Point[1].Y, moveSpeed(), false, false, onOK, onError);
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

int Monster::FindPathMethod()
{
    return FPMETHOD_COMBINE;
}

void Monster::searchNearestTargetHelper(std::unordered_set<uint64_t> seen, std::function<void(uint64_t)> fnTarget)
{
    const auto viewDistance = getMR().view;
    if(viewDistance <= 0){
        if(fnTarget){
            fnTarget(0);
        }
        return;
    }

    int minDistance = INT_MAX;
    uint64_t minDistanceUID = 0;

    if(seen.empty()){
        seen.reserve(m_inViewCOList.size() + 8);
    }

    // for monster like ServerCannibalPlant with view distance 1
    // need to use Chebyshev's distance, otherwise dirs for DIR_UPLEFT, DIR_UPRIGHT, DIR_DOWNLEFT, DIR_DOWNRIGHT are not reachable

    for(const auto &[uid, coLoc]: m_inViewCOList){
        if(!seen.count(uid)){
            const auto distCb = mathf::CDistance <int>(X(), Y(), coLoc.x, coLoc.y);
            const auto distL2 = mathf::LDistance2<int>(X(), Y(), coLoc.x, coLoc.y);

            if(false
                    || ((viewDistance <= 1) && (distCb <= viewDistance))
                    || ((viewDistance >  1) && (distL2 <= viewDistance * viewDistance))){

                if(distL2 < minDistance){
                    minDistance    = distL2;
                    minDistanceUID = uid;
                }
            }
        }
    }

    if(minDistanceUID){
        checkFriend(minDistanceUID, [this, minDistanceUID, seen = std::move(seen), fnTarget = std::move(fnTarget)](int friendType) mutable
        {
            if(friendType == FT_ENEMY){
                if(fnTarget){
                    fnTarget(minDistanceUID);
                }
                return;
            }

            seen.insert(minDistanceUID);
            searchNearestTargetHelper(std::move(seen), std::move(fnTarget));
        });
    }
    else{
        if(fnTarget){
            fnTarget(0);
        }
    }
}

void Monster::searchNearestTarget(std::function<void(uint64_t)> fnTarget)
{
    if(m_inViewCOList.empty()){
        if(fnTarget){
            fnTarget(0);
        }
        return;
    }

    // TODO check if offender is still on same map
    //      and if too far monster should pick up a target near it
    for(auto p = m_offenderList.rbegin(); p != m_offenderList.rend(); ++p){
        if(m_actorPod->checkUIDValid(p->uid)){
            if(fnTarget){
                fnTarget(p->uid);
            }
            return;
        }
    }

    if(uidf::isNeutralMode(UID())){
        if(fnTarget){
            fnTarget(0);
        }
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
    return DBCOM_MAGICID(str_haschar(getMR().dcName) ? getMR().dcName : u8"物理攻击");
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
                                        if(uidf::isGuardMode(finalMasterUID)){
                                            fnOp(FT_ENEMY);
                                        }
                                        else{
                                            fnOp(FT_NEUTRAL);
                                        }
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
                                    throw fflvalue(uidf::getUIDString(finalMasterUID));
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
                throw fflvalue(uidf::getUIDString(targetUID));
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
                                    throw fflvalue(finalMasterUID);
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
                throw fflvalue(targetUID);
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
                                throw fflreach();
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
                        .direction = Direction(),
                        .x = X(),
                        .y = Y(),
                        .fromUID = amA.UID,
                    });

                    struckDamage(amA.UID, amA.damage);
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
