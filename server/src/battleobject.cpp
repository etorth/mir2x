#include <memory>
#include <cinttypes>
#include "pathf.hpp"
#include "uidf.hpp"
#include "sgf.hpp"
#include "uidsf.hpp"
#include "totype.hpp"
#include "player.hpp"
#include "dbcomid.hpp"
#include "mapbindb.hpp"
#include "monster.hpp"
#include "mathf.hpp"
#include "servertaodog.hpp"
#include "fflerror.hpp"
#include "actorpod.hpp"
#include "server.hpp"
#include "charobject.hpp"
#include "actormsgpack.hpp"
#include "protocoldef.hpp"
#include "buff.hpp"
#include "bufflist.hpp"

extern MapBinDB *g_mapBinDB;
extern Server *g_server;

BattleObject::LuaThreadRunner::LuaThreadRunner(BattleObject *battleObjectPtr)
    : CharObject::LuaThreadRunner(battleObjectPtr)
{
    bindFunction("getHealth", [this]()
    {
        std::unordered_map<std::string, int> result;
        result["hp"] = getBO()->m_sdHealth.hp;
        result["mp"] = getBO()->m_sdHealth.mp;

        result["maxHP"] = getBO()->m_sdHealth.getMaxHP();
        result["maxMP"] = getBO()->m_sdHealth.getMaxMP();

        return sol::as_table(result);
    });
}

BattleObject::BOPathFinder::BOPathFinder(const BattleObject *boPtr, int checkCO)
    : pathf::AStarPathFinder(0, boPtr->maxStep(), [this](int srcX, int srcY, int srcDir, int dstX, int dstY) -> std::optional<double>
      {
          fflassert(pathf::hopValid(maxStep(), srcX, srcY, dstX, dstY), maxStep(), srcX, srcY, dstX, dstY);
          return m_BO->oneStepCost(this, m_checkCO, srcX, srcY, srcDir, dstX, dstY);
      })

    , m_BO(boPtr)
    , m_checkCO(checkCO)
{
    fflassert(m_BO);

    fflassert(m_checkCO >= 0, m_checkCO);
    fflassert(m_checkCO <= 2, m_checkCO);

    fflassert(m_BO->maxStep() >= 1, m_BO->maxStep());
    fflassert(m_BO->maxStep() <= 3, m_BO->maxStep());
}

int BattleObject::BOPathFinder::getGrid(int nX, int nY) const
{
    if(!m_BO->mapBin()->validC(nX, nY)){
        return PF_NONE;
    }

    const uint32_t nKey = (to_u32(nX) << 16) | to_u32(nY);
    if(auto p = m_cache.find(nKey); p != m_cache.end()){
        return p->second;
    }
    return (m_cache[nKey] = m_BO->checkPathGrid(nX, nY));
}

BattleObject::BattleObject(
        uint64_t uid,
        uint64_t argMapUID,
        int      mapX,
        int      mapY,
        int      direction)
    : CharObject(uid, argMapUID, mapX, mapY, direction)
    , m_lastAction(ACTION_NONE)
{
    m_lastActionTime.fill(0);
    defer([ptimer = std::make_shared<hres_timer>(), this]() mutable -> bool
    {
        if(hasActorPod() && (ptimer->diff_secf() >= 1.0)){
            if(m_sdHealth.dead()){
                return true;
            }

            updateHealth(m_sdHealth.getHPRecover(), m_sdHealth.getMPRecover());
            ptimer->reset();
        }
        return false;
    });
}

void BattleObject::beforeActivate()
{
    CharObject::beforeActivate();
    m_actorPod->registerOp(AM_QUERYDEAD, [this](const ActorMsgPack &mpk) -> corof::awaitable<>
    {
        m_actorPod->post(mpk.fromAddr(), m_sdHealth.dead() ? AM_TRUE : AM_FALSE);
        return {};
    });
}

corof::awaitable<bool> BattleObject::requestJump(int nX, int nY, int nDirection)
{
    if(!mapBin()->groundValid(nX, nY)){
        throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_lld(mapID()), nX, nY);
    }

    if(X() == nX && Y() == nY){
        co_return false;
    }

    if(!canMove(true)){
        co_return false;
    }

    m_moveLock = true;
    const auto moveLockSg = sgf::guard([this]() noexcept { m_moveLock = false; });

    AMTryJump amTJ;
    std::memset(&amTJ, 0, sizeof(amTJ));

    amTJ.X    = X();
    amTJ.Y    = Y();
    amTJ.EndX = nX;
    amTJ.EndY = nY;

    switch(const auto rmpk = co_await m_actorPod->send(mapUID(), {AM_TRYJUMP, amTJ}); rmpk.type()){
        case AM_ALLOWJUMP:
            {
                if(!canMove(false)){
                    m_actorPod->post(rmpk.fromAddr(), AM_JUMPERROR);
                    co_return false;
                }

                const auto oldX = m_X;
                const auto oldY = m_Y;
                const auto amAJ = rmpk.conv<AMAllowJump>();

                m_X = amAJ.EndX;
                m_Y = amAJ.EndY;

                if(pathf::dirValid(nDirection)){
                    m_direction = nDirection;
                }
                else{
                    m_direction = pathf::getOffDir(oldX, oldY, X(), Y());
                }

                AMJumpOK amJOK;
                std::memset(&amJOK, 0, sizeof(amJOK));

                amJOK.uid = UID();
                amJOK.mapUID = mapUID();
                amJOK.action = ActionJump
                {
                    .direction = Direction(),
                    .x = X(),
                    .y = Y(),
                };

                m_actorPod->post(rmpk.fromAddr(), {AM_JUMPOK, amJOK});
                trimInViewCO();

                if(isPlayer()){
                    dynamic_cast<Player *>(this)->afterChangeGLoc();
                }

                co_await m_buffList.runOnTrigger(BATGR_MOVE);
                co_await m_buffList.runOnBOMove();
                co_await m_buffList.dispatchAura();

                co_return true;
            }
        default:
            {
                co_return false;
            }
    }
}

corof::awaitable<bool> BattleObject::requestMove(int dstX, int dstY, int speed, bool allowHalfMove, bool removeMonster)
{
    if(!mapBin()->groundValid(dstX, dstY)){
        throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_lld(mapID()), dstX, dstY);
    }

    if(!canMove(true)){
        co_return false;
    }

    if(estimateHop(dstX, dstY) != 1){
        co_return false;
    }

    if(removeMonster){
        throw fflerror("RemoveMonster in requestMove() not implemented yet");
    }

    switch(mathf::LDistance2(X(), Y(), dstX, dstY)){
        case 1:
        case 2:
            {
                switch(checkPathGrid(dstX, dstY)){
                    case PF_FREE:
                        {
                            break;
                        }
                    case PF_OCCUPIED:
                    default:
                        {
                            co_return false;
                        }
                }
                break;
            }
        default:
            {
                // one-hop distance but has internal steps
                // need to check if there is co blocking the path
                const auto [nXm, nYm] = pathf::getFrontGLoc(X(), Y(), pathf::getOffDir(X(), Y(), dstX, dstY), 1);

                // for strict co check
                // need to skip the current (X(), Y())

                if(!oneStepCost(nullptr, 2, nXm, nYm, Direction(), dstX, dstY).has_value()){
                    if(!allowHalfMove){
                        co_return false;
                    }
                }
                break;
            }
    }

    m_moveLock = true;
    const auto moveLockSg = sgf::guard([this]() noexcept { m_moveLock = false; });

    AMTryMove amTM;
    std::memset(&amTM, 0, sizeof(amTM));

    amTM.UID           = UID();
    amTM.mapUID        = mapUID();
    amTM.X             = X();
    amTM.Y             = Y();
    amTM.EndX          = dstX;
    amTM.EndY          = dstY;
    amTM.AllowHalfMove = allowHalfMove;
    amTM.RemoveMonster = removeMonster;

    switch(const auto rmpk = co_await m_actorPod->send(mapUID(), {AM_TRYMOVE, amTM}); rmpk.type()){
        case AM_ALLOWMOVE:
            {
                // since we may allow half move
                // servermap permitted dst may not be (dstX, dstY)

                const auto amAM = rmpk.conv<AMAllowMove>();
                fflassert(mapBin()->validC(amAM.EndX, amAM.EndY));

                if(!canMove(false)){
                    m_actorPod->post(rmpk.fromAddr(), AM_MOVEERROR);
                    co_return false;
                }

                const auto oldX = m_X;
                const auto oldY = m_Y;

                m_X = amAM.EndX;
                m_Y = amAM.EndY;

                m_direction = pathf::getOffDir(oldX, oldY, X(), Y());

                AMMoveOK amMOK;
                std::memset(&amMOK, 0, sizeof(amMOK));
                amMOK.uid = UID();
                amMOK.mapUID = mapUID();
                amMOK.action = ActionMove
                {
                    .speed = speed,
                    .x = oldX,
                    .y = oldY,
                    .aimX = X(),
                    .aimY = Y(),
                };

                m_actorPod->post(rmpk.fromAddr(), {AM_MOVEOK, amMOK});
                trimInViewCO();

                co_await m_buffList.runOnTrigger(BATGR_MOVE);
                co_await m_buffList.runOnBOMove();
                co_await m_buffList.dispatchAura();

                // here firstly we make map to boardcast the ActionMove
                // then if BO is a player, notify all its slaves with ActionStand

                // however the order of these two actions reaches neighbor can switch
                // from neighbor's view it firstly get an ActionStand but location changed, then get an ActionMove but destination is current location

                if(isPlayer()){
                    dynamic_cast<Player *>(this)->afterChangeGLoc();
                }
                co_return true;
            }
        default:
            {
                co_return false;
            }
    }
}

corof::awaitable<bool> BattleObject::requestSpaceMove(int locX, int locY, bool strictMove)
{
    if(strictMove){
        if(!mapBin()->groundValid(locX, locY)){
            throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_llu(mapID()), locX, locY);
        }
    }
    else{
        if(!mapBin()->validC(locX, locY)){
            throw fflerror("invalid destination: (mapID = %lld, x = %d, y = %d)", to_llu(mapID()), locX, locY);
        }
    }

    if(!canMove(true)){
        co_return false;
    }

    m_moveLock = true;
    const auto moveLockSg = sgf::guard([this]() noexcept { m_moveLock = false; });

    AMTrySpaceMove amTSM;
    std::memset(&amTSM, 0, sizeof(amTSM));

    amTSM.X = X();
    amTSM.Y = Y();
    amTSM.EndX = locX;
    amTSM.EndY = locY;
    amTSM.StrictMove = strictMove;

    switch(const auto rmpk = co_await m_actorPod->send(mapUID(), {AM_TRYSPACEMOVE, amTSM}); rmpk.type()){
        case AM_ALLOWSPACEMOVE:
            {
                if(!canMove(false)){
                    m_actorPod->post(rmpk.fromAddr(), AM_SPACEMOVEERROR);
                    co_return false;
                }

                // setup new map
                // don't use the requested location
                const auto amASM = rmpk.conv<AMAllowSpaceMove>();

                const auto oldX = m_X;
                const auto oldY = m_Y;

                m_X = amASM.EndX;
                m_Y = amASM.EndY;

                AMSpaceMoveOK amSMOK;
                std::memset(&amSMOK, 0, sizeof(amSMOK));

                amSMOK.uid = UID();
                amSMOK.mapUID = mapUID();
                amSMOK.action = ActionSpaceMove
                {
                    .direction = Direction(),
                    .x = oldX,
                    .y = oldY,
                    .aimX = X(),
                    .aimY = Y(),
                };

                m_actorPod->post(rmpk.fromAddr(), {AM_SPACEMOVEOK, amSMOK});
                trimInViewCO();

                if(isPlayer()){
                    dynamic_cast<Player *>(this)->reportAction(UID(), mapUID(), amSMOK.action);
                    dynamic_cast<Player *>(this)->afterChangeGLoc();
                }

                co_await m_buffList.runOnTrigger(BATGR_MOVE);
                co_await m_buffList.runOnBOMove();
                co_await m_buffList.dispatchAura();

                co_return true;
            }
        default:
            {
                co_return false;
            }
    }
}

corof::awaitable<bool> BattleObject::requestMapSwitch(uint64_t argMapUID, int locX, int locY, bool strictMove)
{
    if(argMapUID == mapUID()){
        throw fflerror("request to switch on same map: mapUID %llu", to_llu(argMapUID));
    }

    if(locX < 0 || locY < 0){
        throw fflerror("invalid argument: mapUID %llu, locX %d, locY %d", to_llu(argMapUID), locX, locY);
    }

    if(!canMove(true)){
        co_return false;
    }

    m_moveLock = true;
    const auto moveLockSg = sgf::guard([this]() noexcept { m_moveLock = false; });

    AMLoadMap amLM;
    std::memset(&amLM, 0, sizeof(amLM));
    amLM.mapUID = argMapUID;

    const auto mpk = co_await m_actorPod->send(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM});
    if(mpk.type() != AM_LOADMAPOK){
        co_return false;
    }

    AMTryMapSwitch amTMS;
    std::memset(&amTMS, 0, sizeof(amTMS));
    amTMS.X = locX;
    amTMS.Y = locY;
    amTMS.strictMove = strictMove;

    const auto rmpk = co_await m_actorPod->send(argMapUID, {AM_TRYMAPSWITCH, amTMS});
    if(rmpk.type() != AM_ALLOWMAPSWITCH){
        co_return false;
    }

    auto mapSwitchErrorSg = sgf::guard([rmpk, this]()
    {
        m_actorPod->post(rmpk.fromAddr(), AM_MAPSWITCHERROR);
    });

    if(!canMove(false)){
        co_return false;
    }

    AMTryLeave amTL;
    std::memset(&amTL, 0, sizeof(amTL));
    amTL.X = X();
    amTL.Y = Y();

    const auto leavermpk = co_await m_actorPod->send(mapUID(), {AM_TRYLEAVE, amTL});
    if(leavermpk.type() != AM_ALLOWLEAVE){
        co_return false;
    }

    auto leaveErrorSg = sgf::guard([leavermpk, this]()
    {
        m_actorPod->post(leavermpk.fromAddr(), AM_LEAVEERROR);
    });

    if(!canMove(false)){
        co_return false;
    }

    const auto amAMS = rmpk.conv<AMAllowMapSwitch>();

    m_mapUID = argMapUID;
    m_mapBinPtr = g_mapBinDB->retrieve(uidf::getMapID(argMapUID));

    m_X = amAMS.X;
    m_Y = amAMS.Y;

    AMLeaveOK amLOK;
    std::memset(&amLOK, 0, sizeof(amLOK));
    amLOK.mapUID = mapUID(); // send new map UID to old map intentionally
    amLOK.action = makeActionStand();

    leaveErrorSg.dismiss();
    co_await m_actorPod->send(leavermpk.fromAddr(), {AM_LEAVEOK, amLOK}); // wait till the old map finishes leave

    AMMapSwitchOK amMSOK;
    std::memset(&amMSOK, 0, sizeof(amMSOK));
    amMSOK.action = makeActionStand();

    mapSwitchErrorSg.dismiss();
    m_actorPod->post({mapUID(), rmpk.seqID()}, {AM_MAPSWITCHOK, amMSOK});

    trimInViewCO();
    if(isPlayer()){
        dynamic_cast<Player *>(this)->reportStand();
        dynamic_cast<Player *>(this)->afterChangeGLoc();
    }

    co_await m_buffList.runOnTrigger(BATGR_MOVE);
    co_await m_buffList.runOnBOMove();
    co_await m_buffList.dispatchAura();

    co_return true;
}

bool BattleObject::canAct() const
{
    if(m_sdHealth.dead()){
        return false;
    }

    switch(m_lastAction){
        case ACTION_SPAWN:
            {
                switch(uidf::getUIDType(UID())){
                    case UID_MON:
                        {
                            switch(uidf::getMonsterID(UID())){
                                case DBCOM_MONSTERID(u8"变异骷髅"):
                                    {
                                        return g_server->getCurrTick() > m_lastActionTime.at(ACTION_SPAWN) + 600;
                                    }
                                case DBCOM_MONSTERID(u8"神兽"):
                                    {
                                        return g_server->getCurrTick() > m_lastActionTime.at(ACTION_SPAWN) + 400;
                                    }
                                case DBCOM_MONSTERID(u8"食人花"):
                                    {
                                        return g_server->getCurrTick() > m_lastActionTime.at(ACTION_SPAWN) + 400;
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
        default:
            {
                break;
            }
    }
    return true;
}

bool BattleObject::canMove(bool checkMoveLock) const
{
    if(!canAct()){
        return false;
    }

    if(!checkMoveLock){
        return true;
    }

    return !m_moveLock;
}

bool BattleObject::canAttack(bool checkAttackLock) const
{
    if(!canAct()){
        return false;
    }

    if(!checkAttackLock){
        return true;
    }

    return !m_moveLock;
}

std::optional<std::tuple<int, int, int>> BattleObject::oneStepReach(int dir, int maxDistance) const
{
    fflassert(pathf::dirValid(dir));
    fflassert(maxDistance >= 1);

    std::optional<std::tuple<int, int, int>> result;
    for(int d = 1; d <= maxDistance; ++d){
        const auto [dstX, dstY] = pathf::getFrontGLoc(X(), Y(), dir, d);
        if(!mapBin()->groundValid(dstX, dstY)){
            break;
        }
        result = std::make_tuple(dstX, dstY, d);
    }
    return result;
}

bool BattleObject::goDie()
{
    if(m_sdHealth.dead()){
        return false;
    }

    setHealth(0);
    return true;
}

void BattleObject::goGhost()
{
    fflassert(m_sdHealth.dead());

    AMDeadFadeOut amDFO;
    std::memset(&amDFO, 0, sizeof(amDFO));

    amDFO.UID    = UID();
    amDFO.mapUID = mapUID();
    amDFO.X      = X();
    amDFO.Y      = Y();

    m_actorPod->post(mapUID(), {AM_DEADFADEOUT, amDFO});
    deactivate(); // calls dtor
}

void BattleObject::dispatchHealth()
{
    dispatchInViewCONetPackage(SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void BattleObject::dispatchHealth(uint64_t uid)
{
    fflassert(uidf::isPlayer(uid));
    forwardNetPackage(uid, SM_HEALTH, cerealf::serialize(m_sdHealth));
}

void BattleObject::dispatchAttackDamage(uint64_t nUID, int nDC, int modifierID)
{
    if(nUID && dcValid(nDC, true)){
        AMAttack amA;
        std::memset(&amA, 0, sizeof(amA));

        amA.UID   = UID();
        amA.mapUID = mapUID();

        amA.X = X();
        amA.Y = Y();

        amA.damage = getAttackDamage(nDC, modifierID);
        m_actorPod->post(nUID, {AM_ATTACK, amA});
    }
}

int BattleObject::Speed(int nSpeedType) const
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

corof::awaitable<uint64_t> BattleObject::addMonster(uint32_t monsterID, int x, int y, bool strictLoc)
{
    const SDInitCharObject sdICO = SDInitMonster
    {
        .monsterID = monsterID,
        .mapUID = mapUID(),
        .x = x,
        .y = y,
        .strictLoc = strictLoc,
        .direction = DIR_BEGIN, // monster may ignore
        .masterUID = UID(),
    };

    switch(const auto rmpk = co_await m_actorPod->send(uidf::getPeerCoreUID(uidf::peerIndex(mapUID())), {AM_ADDCO, cerealf::serialize(sdICO)}); rmpk.type()){
        case AM_UID:
            {
                co_return rmpk.conv<AMUID>().uid;
            }
        default:
            {
                co_return 0;
            }
    }
}

int BattleObject::estimateHop(int nX, int nY)
{
    if(!mapBin()->validC(nX, nY)){
        return -1;
    }

    switch(const auto distance2 = mathf::LDistance2<int>(nX, nY, X(), Y())){
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
                fflassert(maxStep() >= 1, maxStep());
                fflassert(maxStep() <= 3, maxStep());

                if(false
                        || distance2 == 1 * maxStep() * maxStep()
                        || distance2 == 2 * maxStep() * maxStep()){

                    const auto nDir = pathf::getOffDir(X(), Y(), nX, nY);
                    const auto nMaxReach = oneStepReach(nDir, maxStep());

                    if(nMaxReach.has_value() && std::get<2>(nMaxReach.value()) == maxStep()){
                        return 1;
                    }
                }
                return 2;
            }
    }
}

int BattleObject::checkPathGrid(int argX, int argY) const
{
    if(!mapBin()->validC(argX, argY)){
        return PF_NONE;
    }

    if(!mapBin()->cell(argX, argY).land.canThrough()){
        return PF_OBSTACLE;
    }

    if(X() == argX && Y() == argY){
        return PF_OCCUPIED;
    }

    for(const auto &[uid, coLoc]: m_inViewCOList){
        if(coLoc.x == argX && coLoc.y == argY){
            return PF_OCCUPIED;
        }
    }
    return PF_FREE;
}

std::array<pathf::PathNode, 3> BattleObject::getChaseGrid(int nX, int nY, int nDLen) const
{
    // always get the next step to chase
    // this function won't check if (nX, nY) is valid

    const int nX0 = X();
    const int nY0 = Y();

    std::array<pathf::PathNode, 3> pathNodeList
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

std::vector<pathf::PathNode> BattleObject::getValidChaseGrid(int nX, int nY, int nDLen) const
{
    std::vector<pathf::PathNode> result;
    for(const auto &node: getChaseGrid(nX, nY, nDLen)){
        if(mapBin()->groundValid(node.X, node.Y)){
            result.push_back(node);
        }
    }
    return result;
}

void BattleObject::getValidChaseGrid(int nX, int nY, int nDLen, scoped_alloc::svobuf_wrapper<pathf::PathNode, 3> &buf) const
{
    buf.c.clear();
    for(const auto &node: getChaseGrid(nX, nY, nDLen)){
        if(mapBin()->groundValid(node.X, node.Y)){
            buf.c.push_back(node);
        }
    }

    if(buf.c.size() > buf.svocap()){
        throw fflerror("more than 3 valid chase grid found");
    }
}

std::optional<double> BattleObject::oneStepCost(const BattleObject::BOPathFinder *finder, int checkCO, int srcX, int srcY, int srcDir, int dstX, int dstY) const
{
    fflassert(checkCO >= 0, checkCO);
    fflassert(checkCO <= 2, checkCO);

    int hopSize = -1;
    switch(mathf::LDistance2(srcX, srcY, dstX, dstY)){
        case  1:
        case  2: hopSize = 1; break;
        case  4:
        case  8: hopSize = 2; break;
        case  9:
        case 18: hopSize = 3; break;
        case  0: return .0;
        default: return {};
    }

    double gridExtraPen = 0.00;
    const auto hopDir = pathf::getOffDir(srcX, srcY, dstX, dstY);

    for(int stepSize = 1; stepSize <= hopSize; ++stepSize){
        const auto [currX, currY] = pathf::getFrontGLoc(srcX, srcY, hopDir, stepSize);
        switch(const auto pfGrid = finder ? finder->getGrid(currX, currY) : checkPathGrid(currX, currY)){
            case PF_FREE:
                {
                    break;
                }
            case PF_OCCUPIED:
                {
                    switch(checkCO){
                        case 0:
                            {
                                break;
                            }
                        case 1:
                            {
                                gridExtraPen += 100.00;
                                break;
                            }
                        case 2:
                            {
                                return {};
                            }
                        default:
                            {
                                throw fflvalue(checkCO);
                            }
                    }
                    break;
                }
            case PF_NONE:
            case PF_OBSTACLE:
                {
                    return {};
                }
            default:
                {
                    throw fflvalue(currX, currY, pfGrid);
                }
        }
    }

    return 1.00 + hopSize * 0.10 + gridExtraPen + pathf::getDirAbsDiff(srcDir, hopDir) * 0.01;
}

void BattleObject::setLastAction(int type)
{
    m_lastAction = type;
    m_lastActionTime.at(type) = g_server->getCurrTick();
}

bool BattleObject::isOffender(uint64_t nUID)
{
    for(auto &offender: m_offenderList){
        if(offender.uid == nUID){
            return true;
        }
    }
    return false;
}

corof::awaitable<std::optional<SDHealth>> BattleObject::queryHealth(uint64_t uid)
{
    switch(const auto rmpk = co_await m_actorPod->send(uid, AM_QUERYHEALTH); rmpk.type()){
        case AM_HEALTH:
            {
                co_return rmpk.deserialize<SDHealth>();
            }
        default:
            {
                co_return std::nullopt;
            }
    }
}

corof::awaitable<uint64_t> BattleObject::queryFinalMaster(uint64_t targetUID)
{
    const auto fnQuery = [thisptr = this](this auto, uint64_t targetUID) -> corof::awaitable<uint64_t>
    {
        switch(const auto rmpk = co_await thisptr->m_actorPod->send(targetUID, AM_QUERYFINALMASTER); rmpk.type()){
            case AM_UID:
                {
                    co_return rmpk. template conv<AMUID>().uid;
                }
            default:
                {
                    if(thisptr->isMonster() && (targetUID == dynamic_cast<Monster *>(thisptr)->masterUID())){
                        thisptr->goDie();
                    }
                    co_return 0;
                }
        }
    };

    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
            {
                // here don't use mr.tameable
                // monsters not tameable means wizard can't tame it
                // but can be created by GM command, or buy from monster merchant

                if(targetUID == UID()){
                    if(const auto masterUID = dynamic_cast<Monster *>(this)->masterUID()){
                        switch(uidf::getUIDType(masterUID)){
                            case UID_PLY:
                                {
                                    co_return masterUID;
                                }
                            case UID_MON:
                                {
                                    co_return co_await fnQuery(masterUID);
                                }
                            default:
                                {
                                    throw fflvalue(uidf::getUIDString(masterUID));
                                }
                        }
                    }
                    else{
                        co_return UID();
                    }
                }
                else{
                    co_return co_await fnQuery(targetUID);
                }
            }
        case UID_PLY:
        case UID_NPC:
            {
                co_return targetUID;
            }
        default:
            {
                throw fflvalue(uidf::getUIDString(targetUID));
            }
    }
}

void BattleObject::dispatchBuffIDList()
{
    const auto sdBuf = cerealf::serialize(SDBuffIDList
    {
        .uid = UID(),
        .idList = m_buffList.getIDList(),
    });

    dispatchInViewCONetPackage(SM_BUFFIDLIST, sdBuf);
    if(isPlayer()){
        dynamic_cast<Player *>(this)->postNetMessage(SM_BUFFIDLIST, sdBuf);
    }
}

void BattleObject::removeBuff(uint64_t buffSeq, bool dispatch)
{
    if(auto buffp = m_buffList.hasBuff(buffSeq)){
        const auto fromUID = buffp->fromUID();
        m_buffList.removeBuff(buffSeq);

        AMRemoveBuff amRB;
        std::memset(&amRB, 0, sizeof(amRB));
        amRB.fromUID = UID();
        amRB.fromBuffSeq = buffSeq;

        for(const auto uid: getInViewUIDList()){
            switch(uidf::getUIDType(uid)){
                case UID_PLY:
                case UID_MON:
                    {
                        m_actorPod->post(uid, {AM_REMOVEBUFF, amRB});
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }

        for(auto fromBuff: m_buffList.hasFromBuff(fromUID, buffSeq)){
            removeBuff(fromBuff->buffSeq(), false);
        }

        if(dispatch){
            dispatchBuffIDList();
        }
    }
}

void BattleObject::removeFromBuff(uint64_t fromUID, uint64_t fromBuffSeq, bool dispatch)
{
    for(auto pbuff: m_buffList.hasFromBuff(fromUID, fromBuffSeq)){
        removeBuff(pbuff->buffSeq(), false);
    }

    if(dispatch){
        dispatchBuffIDList();
    }
}

BaseBuff *BattleObject::addBuff(uint64_t fromUID, uint64_t fromBuffSeq, uint32_t buffID)
{
    const auto fnAddBuff = [fromUID, fromBuffSeq, buffID, this]() -> BaseBuff *
    {
        auto buffSeqID = m_buffList.rollSeqID(buffID);
        auto buffPtr   = std::make_unique<BaseBuff>(this, fromUID, fromBuffSeq, buffID, buffSeqID);

        if(auto buff = m_buffList.addBuff(std::move(buffPtr))){
            for(const auto paura: buff->getAuraList()){
                if(paura->getBAR().aura.self){
                    addBuff(buff->fromUID(), buff->buffSeq(), DBCOM_BUFFID(paura->getBAR().aura.buff));
                }
                [paura]() -> corof::awaitable<> { co_await paura->dispatch(); }().resume();
            }

            if(buff->getBR().icon.show){
                dispatchBuffIDList();
            }
            return buff;
        }
        return nullptr;
    };

    // NOTE currently only *replace* buff added by self
    // can not remove/override buffs added by other person to avoid cheating

    const auto &br = DBCOM_BUFFRECORD(buffID);
    if(!br){
        return nullptr;
    }

    const auto pbuffList = m_buffList.hasBuff(br.name);
    if(to_d(pbuffList.size()) < br.stackCount + 1){
        return fnAddBuff();
    }

    // total number has reach max stack count
    // check if there are buffs from fromUID and can replace, remove the oldest and add new buff
    const auto pbuffListFromUID = m_buffList.hasFromBuff(fromUID, fromBuffSeq, buffID);
    fflassert(pbuffList.size() >= pbuffListFromUID.size());

    if(!pbuffListFromUID.empty() && br.stackReplace){
        m_buffList.removeBuff(pbuffListFromUID.front()->buffSeq());
        return fnAddBuff();
    }

    return nullptr;
}

bool BattleObject::updateHealth(int addHP, int addMP, int addMaxHP, int addMaxMP)
{
    if(m_sdHealth.updateHealth(addHP, addMP, addMaxHP, addMaxMP)){
        dispatchInViewCONetPackage(SM_HEALTH, cerealf::serialize(m_sdHealth));
        if(m_sdHealth.dead()){
            onDie();
        }
        return true;
    }
    return false;
}

bool BattleObject::setHealth(std::optional<int> hp, std::optional<int> mp, std::optional<int> maxHP, std::optional<int> maxMP)
{
    if(m_sdHealth.setHealth(hp, mp, maxHP, maxMP)){
        dispatchInViewCONetPackage(SM_HEALTH, cerealf::serialize(m_sdHealth));
        if(m_sdHealth.dead()){
            onDie();
        }
        return true;
    }
    return false;
}

std::pair<int, SDTaggedValMap &> BattleObject::updateBuffedAbility(uint32_t buffActID, int percentage, int value)
{
    fflassert(std::abs(percentage) >= 0);
    fflassert(std::abs(percentage) <= 100);

    const auto fnAddValue = [percentage, value](int currValue) -> int
    {
        return std::lround(currValue * percentage / 100.0) + value;
    };

    switch(buffActID){
        case DBCOM_BUFFACTID(u8"DC下限"):
            {
                return {m_sdBuffedAbility.dc[0].add(fnAddValue(m_sdBuffedAbility.dc[0].sum())), m_sdBuffedAbility.dc[0]};
            }
        case DBCOM_BUFFACTID(u8"DC上限"):
            {
                return {m_sdBuffedAbility.dc[1].add(fnAddValue(m_sdBuffedAbility.dc[1].sum())), m_sdBuffedAbility.dc[1]};
            }
        case DBCOM_BUFFACTID(u8"AC下限"):
            {
                return {m_sdBuffedAbility.ac[0].add(fnAddValue(m_sdBuffedAbility.ac[0].sum())), m_sdBuffedAbility.ac[0]};
            }
        case DBCOM_BUFFACTID(u8"AC上限"):
            {
                return {m_sdBuffedAbility.ac[1].add(fnAddValue(m_sdBuffedAbility.ac[1].sum())), m_sdBuffedAbility.ac[1]};
            }
        case DBCOM_BUFFACTID(u8"MAC下限"):
            {
                return {m_sdBuffedAbility.mac[0].add(fnAddValue(m_sdBuffedAbility.mac[0].sum())), m_sdBuffedAbility.mac[0]};
            }
        case DBCOM_BUFFACTID(u8"MAC上限"):
            {
                return {m_sdBuffedAbility.mac[1].add(fnAddValue(m_sdBuffedAbility.mac[1].sum())), m_sdBuffedAbility.mac[1]};
            }
        default:
            {
                throw fflvalue(buffActID, percentage, value);
            }
    }
}

void BattleObject::sendBuff(uint64_t uid, uint64_t fromBuffSeq, uint32_t buffID)
{
    AMAddBuff amAB;
    std::memset(&amAB, 0, sizeof(amAB));

    amAB.id = buffID;
    amAB.fromUID = UID();
    amAB.fromBuffSeq = fromBuffSeq;
    m_actorPod->post(uid, {AM_ADDBUFF, amAB});
}

void BattleObject::notifyDead(uint64_t uid)
{
    fflassert(uid);
    fflassert(m_sdHealth.dead());

    AMNotifyDead amND;
    std::memset(&amND, 0, sizeof(amND));

    amND.UID = UID();
    m_actorPod->post(uid, {AM_NOTIFYDEAD, amND});
}
