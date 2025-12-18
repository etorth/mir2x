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
#include "actorpod.hpp"
#include "actormsg.hpp"
#include "sysconst.hpp"
#include "friendtype.hpp"
#include "server.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"
#include "dropitemconfig.hpp"
#include "serverargparser.hpp"

extern Server *g_server;
extern ServerArgParser *g_serverArgParser;

std::optional<pathf::PathNode> Monster::AStarCache::retrieve(uint32_t mapID, int srcX, int srcY, int dstX, int dstY)
{
    if(g_server->getCurrTick() >= (m_time + m_refresh)){
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
    m_time = g_server->getCurrTick();
    m_mapID = mapID;
    m_path.swap(nodeList);
}

Monster::Monster(
        uint32_t monID,
        uint64_t argMapUID,
        int      mapX,
        int      mapY,
        int      direction,
        uint64_t masterUID)
    : BattleObject(uidf::buildMonsterUID(monID, uidf::peerIndex(argMapUID)), argMapUID, mapX, mapY, direction)
    , m_masterUID(masterUID)
{
    fflassert(getMR());
    m_sdHealth.uid   = UID();
    m_sdHealth.hp    = getMR().hp;
    m_sdHealth.mp    = getMR().mp;
    m_sdHealth.maxHP = getMR().hp;
    m_sdHealth.maxMP = getMR().mp;
}

corof::awaitable<> Monster::onActivate()
{
    co_await BattleObject::onActivate();
    if(masterUID()){
        if(const auto mpk = co_await m_actorPod->send(masterUID(), AM_CHECKMASTER); mpk.type() != AM_CHECKMASTEROK){
            goDie();
            co_return;
        }
    }
    co_await runAICoro();
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

corof::awaitable<bool> Monster::randomMove()
{
    if(!canMove(true)){
        co_return false;
    }

    if(mathf::rand() % 100 < 20){
        if(const auto reachRes = oneStepReach(Direction(), 1); reachRes.has_value()){
            if(const auto [dstX, dstY, dstDist] = reachRes.value(); dstDist == 1){
                co_return co_await requestMove(dstX, dstY, moveSpeed(), false, false);
            }
        }
    }

    if(mathf::rand() % 100 < 20){
        co_return randomTurn();
    }

    co_return false; // does nothing
}

corof::awaitable<bool> Monster::attackUID(uint64_t uid, int magicID)
{
    if(!canAttack(true)){
        co_return false;
    }

    if(!dcValid(magicID, true)){
        co_return false;
    }

    // retrieving could schedule location query
    // before response received we can't allow any attack request

    m_attackLock = true;
    const auto attackLockSg = sgf::guard([this]() noexcept { m_attackLock = false; });

    const auto coLocOpt = co_await getCOLocation(uid);
    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
        co_return false;
    }

    if(const auto newDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(newDir)){
        m_direction = newDir;
    }

    if(!canAttack(false)){
        co_return false;
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

    addDelay(550, [this, uid, magicID, modifierID](bool)
    {
        dispatchAttackDamage(uid, magicID, modifierID);
    });

    co_return true;
}

corof::awaitable<bool> Monster::jumpUID(uint64_t targetUID)
{
    const auto coLocOpt = co_await getCOLocation(targetUID);
    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();

    if(mapUID() != coLoc.mapUID || !mapBin()->validC(coLoc.x, coLoc.y)){
        co_return false;
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

    const auto  dstDir = pathf::dirValid(coLoc.direction) ? coLoc.direction : DIR_UP;
    const auto nextDir = pathf::getNextDir(dstDir, 1);

    const auto [frontX, frontY] = pathf::getFrontGLoc(coLoc.x, coLoc.y, nextDir, 1);

    if(!mapBin()->groundValid(frontX, frontY)){
        co_return false;
    }

    if(frontX == X() && frontY == Y()){
        co_return true;
    }

    co_return co_await requestJump(frontX, frontY, pathf::getBackDir(nextDir));
}

corof::awaitable<bool> Monster::trackUID(uint64_t targetUID, DCCastRange r)
{
    const auto coLocOpt = co_await getCOLocation(targetUID);

    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();

    if(mapUID() != coLoc.mapUID || !mapBin()->validC(coLoc.x, coLoc.y)){
        co_return false;
    }

    // patch the track function, r can be provided as {}
    // if r provided as invalid, we accept it as to follow the uid generally

    if(r){
        if(pathf::inDCCastRange(r, X(), Y(), coLoc.x, coLoc.y)){
            co_return true;
        }
        else{
            co_return co_await moveOneStep(coLoc.x, coLoc.y);
        }
    }
    else{
        if((X() == coLoc.x) && (Y() == coLoc.y)){
            co_return true;
        }
        else{
            co_return co_await moveOneStep(coLoc.x, coLoc.y);
        }
    }
}

corof::awaitable<bool> Monster::followMaster()
{
    if(!masterUID()){
        co_return false;
    }

    if(!canMove(true)){
        co_return false;
    }

    // followMaster works almost like trackUID(), but
    // 1. follower always try to stand at the back of the master
    // 2. when distance is too far or master is on different map, follower takes space move

    const auto coLocOpt = co_await getCOLocation(masterUID());

    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();

    // check if it's still my master?
    // possible during the location query master changed

    if(coLoc.uid != masterUID()){
        co_return false;
    }

    if(!canMove(true)){
        co_return false;
    }

    const auto masterDir = pathf::dirValid(coLoc.direction) ? coLoc.direction : [this]() -> int
    {
        switch(uidf::getUIDType(masterUID())){
            case UID_PLY: return pathf::getRandDir();
            default     : return DIR_BEGIN;
        }
    }();

    if((coLoc.mapUID == mapUID()) && (mathf::LDistance<double>(coLoc.x, coLoc.y, X(), Y()) < 10.0)){

        // not that long
        // slave should move step by step
        const auto [backX, backY] = pathf::getBackGLoc(coLoc.x, coLoc.y, masterDir, 1);

        switch(mathf::LDistance2<int>(backX, backY, X(), Y())){
            case 0:
                {
                    // already get there
                    // need to make a turn if needed

                    if(Direction() != masterDir){
                        m_direction = masterDir;
                        dispatchAction(makeActionStand());
                    }

                    co_return true;
                }
            default:
                {
                    co_return co_await moveOneStep(backX, backY);
                }
        }
    }
    else{
        // long distance
        // need to do spacemove or even mapswitch
        const auto [backX, backY] = pathf::getBackGLoc(coLoc.x, coLoc.y, masterDir, 3);

        if(coLoc.mapUID == mapUID()){
            co_return co_await requestSpaceMove(backX, backY, false);
        }
        else{
            co_return co_await requestMapSwitch(coLoc.mapUID, backX, backY, false);
        }
    }
}

corof::awaitable<bool> Monster::jumpAttackUID(uint64_t targetUID)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    const auto magicID = getAttackMagic(targetUID);
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    if(!mr){
        co_return false;
    }

    if(co_await jumpUID(targetUID)){
        co_return co_await attackUID(targetUID, magicID);
    }

    co_return false;
}

corof::awaitable<bool> Monster::trackAttackUID(uint64_t targetUID)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    const auto magicID = getAttackMagic(targetUID);
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    if(!mr){
        co_return false;
    }

    if(co_await trackUID(targetUID, mr.castRange)){
        co_return co_await attackUID(targetUID, magicID);
    }

    co_return false;
}

corof::awaitable<> Monster::runAICoro()
{
    uint64_t targetUID = 0;
    hres_timer targetActiveTimer;

    while(!m_sdHealth.dead()){
        if(targetUID && targetActiveTimer.diff_sec() > SYS_TARGETSEC){
            targetUID = 0;
        }

        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
            if(targetUID){
                targetActiveTimer.reset();
            }
        }

        if(targetUID){
            if(co_await trackAttackUID(targetUID)){
                targetActiveTimer.reset();
            }
        }
        else if(masterUID()){
            if(m_actorPod->checkUIDValid(masterUID())){
                co_await followMaster();
            }
            else{
                break;
            }
        }
        else{
            co_await randomMove();
        }

        co_await asyncIdleWait(1000);
    }
}

corof::awaitable<> Monster::onActorMsg(const ActorMsgPack &rstMPK)
{
    switch(rstMPK.type()){
        case AM_CHECKMASTER:
            {
                return on_AM_CHECKMASTER(rstMPK);
            }
        case AM_ADDBUFF:
            {
                return on_AM_ADDBUFF(rstMPK);
            }
        case AM_REMOVEBUFF:
            {
                return on_AM_REMOVEBUFF(rstMPK);
            }
        case AM_QUERYFINALMASTER:
            {
                return on_AM_QUERYFINALMASTER(rstMPK);
            }
        case AM_QUERYFRIENDTYPE:
            {
                return on_AM_QUERYFRIENDTYPE(rstMPK);
            }
        case AM_QUERYNAMECOLOR:
            {
                return on_AM_QUERYNAMECOLOR(rstMPK);
            }
        case AM_QUERYHEALTH:
            {
                return on_AM_QUERYHEALTH(rstMPK);
            }
        case AM_NOTIFYNEWCO:
            {
                return on_AM_NOTIFYNEWCO(rstMPK);
            }
        case AM_DEADFADEOUT:
            {
                return on_AM_DEADFADEOUT(rstMPK);
            }
        case AM_NOTIFYDEAD:
            {
                return on_AM_NOTIFYDEAD(rstMPK);
            }
        case AM_UPDATEHP:
            {
                return on_AM_UPDATEHP(rstMPK);
            }
        case AM_EXP:
            {
                return on_AM_EXP(rstMPK);
            }
        case AM_MISS:
            {
                return on_AM_MISS(rstMPK);
            }
        case AM_HEAL:
            {
                return on_AM_HEAL(rstMPK);
            }
        case AM_ACTION:
            {
                return on_AM_ACTION(rstMPK);
            }
        case AM_ATTACK:
            {
                return on_AM_ATTACK(rstMPK);
            }
        case AM_MAPSWITCHTRIGGER:
            {
                return on_AM_MAPSWITCHTRIGGER(rstMPK);
            }
        case AM_QUERYLOCATION:
            {
                return on_AM_QUERYLOCATION(rstMPK);
            }
        case AM_QUERYUIDBUFF:
            {
                return on_AM_QUERYUIDBUFF(rstMPK);
            }
        case AM_QUERYCORECORD:
            {
                return on_AM_QUERYCORECORD(rstMPK);
            }
        case AM_BADACTORPOD:
            {
                return on_AM_BADACTORPOD(rstMPK);
            }
        case AM_OFFLINE:
            {
                return on_AM_OFFLINE(rstMPK);
            }
        case AM_MASTERKILL:
            {
                return on_AM_MASTERKILL(rstMPK);
            }
        case AM_MASTERHITTED:
            {
                return on_AM_MASTERHITTED(rstMPK);
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
    amCOR.mapUID = mapUID();
    amCOR.action = makeActionStand();
    amCOR.Monster.MonsterID = monsterID();
    m_actorPod->post(toUID, {AM_CORECORD, amCOR});
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

bool Monster::canMove(bool checkMoveLock) const
{
    if(!BattleObject::canMove(checkMoveLock)){
        return false;
    }

    return g_server->getCurrTick() >= std::max<uint32_t>(
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

bool Monster::canAttack(bool checkAttackLock) const
{
    if(!BattleObject::canAttack(checkAttackLock)){
        return false;
    }

    return g_server->getCurrTick() >= std::max<uint32_t>(
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

void Monster::onDie()
{
    dispatchOffenderExp();
    for(auto &item: getMonsterDropItemList(monsterID())){
        m_actorPod->post(mapUID(), {AM_DROPITEM, cerealf::serialize(SDDropItem
        {
            .x = X(),
            .y = Y(),
            .item = std::move(item),
        })});
    }

    dispatchAction(ActionDie
    {
        .x = X(),
        .y = Y(),
    });

    // don't deactivate() immdiately here
    // in future revive dead monsters maybe be revivied

    if(getMR().deadFadeOut){
        addDelay(1000, [this](bool) { goGhost(); });
    }
    else{
        goGhost();
    }
}

bool Monster::struckDamage(uint64_t fromUID, const DamageNode &node)
{
    if(!node){
        return false;
    }

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
        updateHealth(-damage);
        switch(node.modifierID){
            case DBCOM_ATTACKMODIFIERID(u8"吸血"):
                {
                    AMHeal amH;
                    std::memset(&amH, 0, sizeof(amH));

                    amH.mapUID = mapUID();
                    amH.addHP  = std::min<int>(damage, 20);

                    m_actorPod->post(fromUID, {AM_HEAL, amH});
                    break;
                }
            default:
                {
                    break;
                }
        }

        if(m_sdHealth.dead()){
            onDie();
        }
    }
    return true;
}

corof::awaitable<bool> Monster::moveOneStep(int argX, int argY)
{
    if(!canMove(true)){
        co_return false;
    }

    switch(estimateHop(argX, argY)){
        case 0:
            {
                co_return false;
            }
        case 1:
            {
                if(oneStepCost(nullptr, 1, X(), Y(), Direction(), argX, argY).has_value()){
                    co_return co_await requestMove(argX, argY, moveSpeed(), false, false);
                }
                break;
            }
        case 2:
            {
                break;
            }
        default:
            {
                co_return false;
            }
    }

    if(const auto nodeOpt = m_astarCache.retrieve(mapID(), X(), Y(), argX, argY); nodeOpt.has_value()){
        if(oneStepCost(nullptr, 1, X(), Y(), Direction(), nodeOpt.value().X, nodeOpt.value().Y).has_value()){
            co_return co_await requestMove(nodeOpt.value().X, nodeOpt.value().Y, moveSpeed(), false, false);
        }
    }

    switch(FindPathMethod()){
        case FPMETHOD_ASTAR:
            {
                co_return co_await moveOneStepAStar(argX, argY);
            }
        case FPMETHOD_GREEDY:
            {
                co_return co_await moveOneStepGreedy(argX, argY);
            }
        case FPMETHOD_COMBINE:
            {
                co_return co_await moveOneStepCombine(argX, argY);
            }
        case FPMETHOD_NEIGHBOR:
            {
                co_return co_await moveOneStepNeighbor(argX, argY);
            }
        default:
            {
                co_return false;
            }
    }
}

corof::awaitable<bool> Monster::moveOneStepNeighbor(int argX, int argY)
{
    if(!canMove(true)){
        co_return false;
    }

    BattleObject::BOPathFinder stFinder(this, 1);
    if(!stFinder.search(X(), Y(), Direction(), argX, argY).hasPath()){
        co_return false;
    }

    auto pathList = stFinder.getPathNode();
    const auto stPathNode = pathList.at(1);

    if(pathList.size() < 2){
        throw fflerror("incorrect pathnode number: %zu", pathList.size());
    }

    m_astarCache.cache(mapID(), std::move(pathList));
    co_return co_await requestMove(stPathNode.X, stPathNode.Y, moveSpeed(), false, false);
}

corof::awaitable<bool> Monster::moveOneStepGreedy(int argX, int argY)
{
    if(!canMove(true)){
        co_return false;
    }

    scoped_alloc::svobuf_wrapper<int, 2> distList;
    distList.c.push_back(1);

    if((maxStep() > 1) && (mathf::CDistance(X(), Y(), argX, argY) >= maxStep())){
        distList.c.push_back(maxStep());
    }

    scoped_alloc::svobuf_wrapper<pathf::PathNode, 3> pathNodeList;
    for(const auto stepSize: distList.c){

        pathNodeList.c.clear();
        getValidChaseGrid(argX, argY, stepSize, pathNodeList);

        if(pathNodeList.c.size() > 3){
            throw fflerror("invalid chase grid size: %zu", pathNodeList.c.size());
        }

        for(const auto &node: pathNodeList.c){
            if(co_await requestMove(node.X, node.Y, moveSpeed(), false, false)){
                co_return true;
            }
        }
    }
    co_return false;
}

corof::awaitable<bool> Monster::moveOneStepCombine(int argX, int argY)
{
    if(!canMove(true)){
        co_return false;
    }

    if(co_await moveOneStepGreedy(argX, argY)){
        co_return true;
    }
    if(co_await moveOneStepNeighbor(argX, argY)){
        co_return true;
    }

    co_return false;
}

corof::awaitable<bool> Monster::moveOneStepAStar(int argX, int argY)
{
    if(!canMove(true)){
        co_return false;
    }

    AMPathFind amPF;
    std::memset(&amPF, 0, sizeof(amPF));

    amPF.UID       = UID();
    amPF.mapUID    = mapUID();
    amPF.CheckCO   = 1;
    amPF.MaxStep   = maxStep();
    amPF.X         = X();
    amPF.Y         = Y();
    amPF.direction = Direction();
    amPF.EndX      = argX;
    amPF.EndY      = argY;

    switch(const auto rmpk = co_await m_actorPod->send(mapUID(), {AM_PATHFIND, amPF}); rmpk.type()){
        case AM_PATHFINDOK:
            {
                AMPathFindOK amPFOK;
                std::memcpy(&amPFOK, rmpk.data(), sizeof(amPFOK));

                constexpr auto nNodeCount = std::extent<decltype(amPFOK.Point)>::value;
                static_assert(nNodeCount >= 2);

                auto pBegin = amPFOK.Point;
                auto pEnd   = amPFOK.Point + nNodeCount;

                std::vector<pathf::PathNode> stvPathNode;
                for(auto pCurr = pBegin; pCurr != pEnd; ++pCurr){
                    if(mapBin()->groundValid(pCurr->X, pCurr->Y)){
                        stvPathNode.emplace_back(pCurr->X, pCurr->Y);
                    }
                    else{
                        break;
                    }
                }

                if(!stvPathNode.back().eq(argX, argY)){
                    stvPathNode.emplace_back(argX, argY);
                }

                m_astarCache.cache(mapID(), std::move(stvPathNode));
                co_return co_await requestMove(amPFOK.Point[1].X, amPFOK.Point[1].Y, moveSpeed(), false, false);
            }
        default:
            {
                co_return false;
            }
    }
}

int Monster::FindPathMethod()
{
    return FPMETHOD_COMBINE;
}

corof::awaitable<uint64_t> Monster::searchNearestTarget()
{
    if(m_inViewCOList.empty()){
        co_return 0;
    }

    for(auto p = m_offenderList.rbegin(); p != m_offenderList.rend(); ++p){
        if(m_actorPod->checkUIDValid(p->uid)){
            co_return p->uid;
        }
    }

    if(uidf::isNeutralMode(UID())){
        co_return 0;
    }

    const auto viewDistance = getMR().view;
    if(viewDistance <= 0){
        co_return 0;
    }

    // for monster like ServerCannibalPlant with view distance 1
    // need to use Chebyshev's distance, otherwise dirs for DIR_UPLEFT, DIR_UPRIGHT, DIR_DOWNLEFT, DIR_DOWNRIGHT are not reachable

    const auto fnSearchNearestUID = [viewDistance, this](std::unordered_set<uint64_t> &seen) -> uint64_t
    {
        int minDistance = INT_MAX;
        uint64_t minDistanceUID = 0;

        for(const auto &[uid, coLoc]: m_inViewCOList){
            if(!seen.contains(uid)){
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
            seen.insert(minDistanceUID);
        }
        return minDistanceUID;
    };

    std::unordered_set<uint64_t> seen;
    seen.reserve(m_inViewCOList.size() + 8);

    while(const auto targetUID = fnSearchNearestUID(seen)){
        switch(uidf::getUIDType(targetUID)){
            case UID_MON:
            case UID_PLY: break;
            default     : continue;
        }

        if(co_await queryDead(targetUID)){
            continue;
        }

        switch(co_await checkFriend(targetUID)){
            case FT_ENEMY:
                {
                    co_return targetUID;
                }
            default:
                {
                    break;
                }
        }
    }

    co_return 0;
}

corof::awaitable<uint64_t> Monster::pickTarget()
{
    // can have different strategy to pick target: nearest, latest, weakest
    //
    return searchNearestTarget();
}

int Monster::getAttackMagic(uint64_t) const
{
    return DBCOM_MAGICID(str_haschar(getMR().dcName) ? getMR().dcName : u8"物理攻击");
}

corof::awaitable<int> Monster::checkFriend_ctrlByMonster(uint64_t targetUID)
{
    fflassert(targetUID);
    if(masterUID()){
        fflassert(uidf::isMonster(masterUID()));
    }

    switch(uidf::getUIDType(targetUID)){
        case UID_PLY:
            {
                co_return FT_ENEMY;
            }
        case UID_MON:
            {
                if(const auto finalMasterUID = co_await queryFinalMaster(targetUID)){
                    switch(uidf::getUIDType(finalMasterUID)){
                        case UID_MON:
                            {
                                if(uidf::isGuardMode(finalMasterUID)){
                                    co_return FT_ENEMY;
                                }
                                else{
                                    co_return FT_NEUTRAL;
                                }
                            }
                        case UID_PLY:
                            {
                                co_return FT_ENEMY;
                            }
                        default:
                            {
                                throw fflvalue(uidf::getUIDString(finalMasterUID));
                            }
                    }
                }
                else{
                    co_return FT_ERROR;
                }
            }
        case UID_NPC:
            {
                co_return FT_NEUTRAL;
            }
        default:
            {
                throw fflvalue(uidf::getUIDString(targetUID));
            }
    }
}

corof::awaitable<int> Monster::checkFriend_ctrlByPlayer(uint64_t targetUID)
{
    fflassert(targetUID);
    fflassert(masterUID());
    fflassert(uidf::isPlayer(masterUID()));

    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
            {
                if(const auto finalMasterUID = co_await queryFinalMaster(targetUID)){
                    switch(uidf::getUIDType(finalMasterUID)){
                        case UID_MON:
                            {
                                co_return FT_ENEMY;
                            }
                        case UID_PLY:
                            {
                                if(finalMasterUID == masterUID()){
                                    co_return FT_NEUTRAL;
                                }
                                else{
                                    co_return co_await queryPlayerFriend(masterUID(), finalMasterUID);
                                }
                            }
                        default:
                            {
                                throw fflvalue(uidf::getUIDString(finalMasterUID));
                            }
                    }
                }
                else{
                    co_return FT_ERROR;
                }
            }
        case UID_PLY:
            {
                co_return co_await queryPlayerFriend(masterUID(), targetUID);
            }
        default:
            {
                throw fflvalue(uidf::getUIDString(targetUID));
            }
    }
}

corof::awaitable<int> Monster::checkFriend(uint64_t targetUID)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    if(uidf::getUIDType(targetUID) == UID_NPC){
        co_return FT_NEUTRAL;
    }

    if(!masterUID()){
        co_return co_await checkFriend_ctrlByMonster(targetUID);
    }

    // has a master
    // can be its master

    if(targetUID == masterUID()){
        co_return FT_FRIEND;
    }

    // has a master
    // check the final master

    if(const auto finalMasterUID = co_await queryFinalMaster(UID())){
        switch(uidf::getUIDType(finalMasterUID)){
            case UID_PLY:
                {
                    co_return co_await checkFriend_ctrlByPlayer(targetUID);
                }
            case UID_MON:
                {
                    co_return co_await checkFriend_ctrlByMonster(targetUID);
                }
            default:
                {
                    throw fflvalue(uidf::getUIDString(finalMasterUID));
                }
        }
    }

    goDie();
    co_return FT_ERROR;
}

corof::awaitable<int> Monster::queryPlayerFriend(uint64_t fromUID, uint64_t targetUID)
{
    // this function means:
    // ask fromUID: how do you feel about targetUID

    fflassert(uidf::isPlayer(fromUID));
    fflassert(uidf::isPlayer(targetUID));

    AMQueryFriendType amQFT;
    std::memset(&amQFT, 0, sizeof(amQFT));
    amQFT.UID = targetUID;

    switch(const auto rmpk = co_await m_actorPod->send(fromUID, {AM_QUERYFRIENDTYPE, amQFT}); rmpk.type()){
        case AM_FRIENDTYPE:
            {
                const auto amFT = rmpk.conv<AMFriendType>();
                switch(amFT.Type){
                    case FT_ERROR:
                    case FT_ENEMY:
                    case FT_FRIEND:
                    case FT_NEUTRAL:
                        {
                            co_return amFT.Type;
                        }
                    default:
                        {
                            throw fflvalue(amFT.Type);
                        }
                }
            }
        default:
            {
                co_return FT_ERROR;
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

corof::awaitable<bool> Monster::asyncIdleWait(uint64_t tick)
{
    fflassert(tick > 0);

    if(g_serverArgParser->sharedConfig().disableMonsterIdleWait){
        co_return co_await asyncWait(tick);
    }

    if(hasPlayerNeighbor()){
        co_return co_await asyncWait(tick);
    }

    m_idleWaitToken.emplace();
    const auto timeout = co_await asyncWait(0, std::addressof(m_idleWaitToken.value())); // infinite wait till cancel

    m_idleWaitToken.reset();
    co_return timeout;
}

corof::awaitable<> Monster::onAMAttack(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAttack>();
    if(amA.UID == UID()){
        co_return;
    }

    if(m_sdHealth.dead()){
        notifyDead(amA.UID);
        co_return;
    }

    switch(const auto friendType = co_await checkFriend(amA.UID); friendType){
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
                                m_actorPod->post(amA.UID, {AM_MISS, amM});
                                co_return;
                            }
                        default:
                            {
                                co_return;
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
                co_return;
            }
        default:
            {
                co_return;
            }
    }
}

corof::awaitable<> Monster::onAMMasterHitted(const ActorMsgPack &)
{
    // do nothing by default
    co_return;
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
                    m_actorPod->post(offender.uid, {AM_EXP, amE});
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

corof::awaitable<bool> Monster::moveForward()
{
    const auto reachRes = oneStepReach(Direction(), 1);
    if(!reachRes.has_value()){
        co_return false;
    }

    const auto [nextX, nextY, reachDist] = reachRes.value();
    if(reachDist != 1){
        co_return false;
    }

    co_return co_await requestMove(nextX, nextY, moveSpeed(), false, false);
}

corof::awaitable<bool> Monster::needHeal(uint64_t uid)
{
    switch(uidf::getUIDType(uid)){
        case UID_MON:
        case UID_PLY:
            {
                const auto health = co_await queryHealth(uid);
                if(health.has_value() && health.value().hp < health.value().maxHP){
                    co_return true;
                }
                break;
            }
        default:
            {
                break;
            }
    }
    co_return false;
}

corof::awaitable<uint64_t> Monster::pickHealTarget()
{
    if(masterUID() && m_inViewCOList.contains(masterUID()) && (co_await needHeal(masterUID()))){
        co_return masterUID();
    }

    for(const auto uid: getInViewUIDList()){
        if(uid == masterUID()){
            continue;
        }

        if((co_await checkFriend(uid)) != FT_FRIEND){
            continue;
        }

        if(co_await needHeal(uid)){
            co_return uid;
        }
    }
    co_return 0;
}

corof::awaitable<bool> Monster::inDCCastRange(uint64_t targetUID, DCCastRange r)
{
    fflassert(targetUID);
    fflassert(r);

    const auto coLocOpt = co_await getCOLocation(targetUID);

    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();
    co_return pathf::inDCCastRange(r, X(), Y(), coLoc.x, coLoc.y);
}

corof::awaitable<bool> Monster::validTarget(uint64_t targetUID)
{
    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
        case UID_PLY:
            {
                break;
            }
        default:
            {
                co_return false;
            }
    }

    if(!m_actorPod->checkUIDValid(targetUID)){
        co_return false;
    }

    if(co_await queryDead(targetUID)){
        co_return false;
    }

    const auto coLocOpt = co_await getCOLocation(targetUID);

    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto &coLoc = coLocOpt.value();

    if(!inView(coLoc.mapUID, coLoc.x, coLoc.y)){
        co_return false;
    }

    const auto viewDistance = getMR().view;
    if(viewDistance <= 0){
        co_return false;
    }

    if(mathf::LDistance2<int>(X(), Y(), coLoc.x, coLoc.y) > viewDistance * viewDistance){
        co_return false;
    }
    co_return true;
}
