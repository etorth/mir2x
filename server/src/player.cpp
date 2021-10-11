/*
 * =====================================================================================
 *
 *       Filename: player.cpp
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
#include "dbpod.hpp"
#include "player.hpp"
#include "uidf.hpp"
#include "pathf.hpp"
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "sysconst.hpp"
#include "netdriver.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "buildconfig.hpp"
#include "serverargparser.hpp"

extern DBPod *g_dbPod;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;
extern ServerArgParser *g_serverArgParser;

Player::Player(const SDInitPlayer &initParam, const ServerMap *mapPtr)
    : CharObject(mapPtr, uidf::buildPlayerUID(initParam.dbid, initParam.gender, initParam.jobList), initParam.x, initParam.y, DIR_DOWN)
    , m_exp(initParam.exp)
    , m_name(initParam.name)
    , m_nameColor(initParam.nameColor)
    , m_hair(initParam.hair)
    , m_hairColor(initParam.hairColor)
{
    m_sdHealth.uid   = UID();
    m_sdHealth.HP    = initParam.hp;
    m_sdHealth.MP    = initParam.mp;
    m_sdHealth.maxHP = initParam.hp;
    m_sdHealth.maxMP = initParam.mp;

    m_sdItemStorage.gold = initParam.gold;

    dbLoadWear();
    dbLoadBelt();
    dbLoadInventory();
    dbLoadLearnedMagic();
    dbLoadRuntimeConfig();

    m_stateTrigger.install([this, lastCheckTick = to_u32(0)]() mutable -> bool
    {
        if(const auto currTick = g_monoServer->getCurrTick(); currTick >= (lastCheckTick + 1000)){
            recoverHealth();
            lastCheckTick = currTick;
        }
        return false;
    });
}

void Player::operateAM(const ActorMsgPack &rstMPK)
{
    switch(rstMPK.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(rstMPK);
                break;
            }
        case AM_BADACTORPOD:
            {
                on_AM_BADACTORPOD(rstMPK);
                break;
            }
        case AM_NOTIFYNEWCO:
            {
                on_AM_NOTIFYNEWCO(rstMPK);
                break;
            }
        case AM_QUERYHEALTH:
            {
                on_AM_QUERYHEALTH(rstMPK);
                break;
            }
        case AM_CHECKMASTER:
            {
                on_AM_CHECKMASTER(rstMPK);
                break;
            }
        case AM_MAPSWITCH:
            {
                on_AM_MAPSWITCH(rstMPK);
                break;
            }
        case AM_NPCQUERY:
            {
                on_AM_NPCQUERY(rstMPK);
                break;
            }
        case AM_QUERYLOCATION:
            {
                on_AM_QUERYLOCATION(rstMPK);
                break;
            }
        case AM_QUERYFRIENDTYPE:
            {
                on_AM_QUERYFRIENDTYPE(rstMPK);
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
        case AM_DEADFADEOUT:
            {
                on_AM_DEADFADEOUT(rstMPK);
                break;
            }
        case AM_BINDCHANNEL:
            {
                on_AM_BINDCHANNEL(rstMPK);
                break;
            }
        case AM_SENDPACKAGE:
            {
                on_AM_SENDPACKAGE(rstMPK);
                break;
            }
        case AM_RECVPACKAGE:
            {
                on_AM_RECVPACKAGE(rstMPK);
                break;
            }
        case AM_QUERYCORECORD:
            {
                on_AM_QUERYCORECORD(rstMPK);
                break;
            }
        case AM_BADCHANNEL:
            {
                on_AM_BADCHANNEL(rstMPK);
                break;
            }
        case AM_OFFLINE:
            {
                on_AM_OFFLINE(rstMPK);
                break;
            }
        case AM_QUERYPLAYERWLDESP:
            {
                on_AM_QUERYPLAYERWLDESP(rstMPK);
                break;
            }
        case AM_REMOVEGROUNDITEM:
            {
                on_AM_REMOVEGROUNDITEM(rstMPK);
                break;
            }
        case AM_CORECORD:
            {
                on_AM_CORECORD(rstMPK);
                break;
            }
        case AM_NOTIFYDEAD:
            {
                on_AM_NOTIFYDEAD(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(rstMPK.type()));
                break;
            }
    }
}

void Player::operateNet(uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
#define _support_cm(cm) case cm: net_##cm(nType, pData, nDataLen); break
        _support_cm(CM_QUERYCORECORD             );
        _support_cm(CM_REQUESTADDEXP             );
        _support_cm(CM_REQUESTKILLPETS           );
        _support_cm(CM_REQUESTSPACEMOVE          );
        _support_cm(CM_REQUESTRETRIEVESECUREDITEM);
        _support_cm(CM_ACTION                    );
        _support_cm(CM_PICKUP                    );
        _support_cm(CM_PING                      );
        _support_cm(CM_CONSUMEITEM               );
        _support_cm(CM_BUY                       );
        _support_cm(CM_QUERYGOLD                 );
        _support_cm(CM_NPCEVENT                  );
        _support_cm(CM_QUERYSELLITEMLIST         );
        _support_cm(CM_QUERYPLAYERWLDESP         );
        _support_cm(CM_REQUESTEQUIPWEAR          );
        _support_cm(CM_REQUESTGRABWEAR           );
        _support_cm(CM_REQUESTEQUIPBELT          );
        _support_cm(CM_REQUESTGRABBELT           );
        _support_cm(CM_DROPITEM                  );
        _support_cm(CM_SETMAGICKEY               );
        default: break;
#undef _support_cm
    }
}

bool Player::update()
{
    return true;
}

void Player::reportCO(uint64_t toUID)
{
    if(!toUID){
        return;
    }

    AMCORecord amCOR;
    std::memset(&amCOR, 0, sizeof(amCOR));

    amCOR.UID = UID();
    amCOR.mapID = mapID();
    amCOR.action = makeActionStand();
    amCOR.Player.Level = level();
    m_actorPod->forward(toUID, {AM_CORECORD, amCOR});
}

void Player::reportStand()
{
    reportAction(UID(), makeActionStand());
}

void Player::reportAction(uint64_t nUID, const ActionNode &action)
{
    if(true
            && nUID
            && channID()){

        SMAction smA;
        std::memset(&smA, 0, sizeof(smA));

        smA.UID = nUID;
        smA.mapID = mapID();
        smA.action = action;

        g_netDriver->Post(channID(), SM_ACTION, smA);
    }
}

void Player::reportDeadUID(uint64_t nDeadUID)
{
    SMNotifyDead smND;
    std::memset(&smND, 0, sizeof(smND));

    smND.UID = nDeadUID;
    postNetMessage(SM_NOTIFYDEAD, smND);
}

void Player::reportHealth()
{
    dispatchNetPackage(true, SM_HEALTH, cerealf::serialize(m_sdHealth));
}

bool Player::goDie()
{
    if(m_dead.get()){
        return true;
    }
    m_dead.set(true);

    addDelay(2 * 1000, [this](){ goGhost(); });
    return true;
}

bool Player::goGhost()
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

    if(true
            && checkActorPod()
            && m_map
            && m_map->checkActorPod()){
        m_actorPod->forward(m_map->UID(), {AM_DEADFADEOUT, amDFO});
    }

    deactivate();
    return true;
}

bool Player::DCValid(int, bool)
{
    return true;
}

DamageNode Player::getAttackDamage(int nDC) const
{
    const auto node = getCombatNode(m_sdItemStorage.wear, {}, UID(), level());
    const double elemRatio = 1.0 + 0.1 * [nDC, &node]() -> int
    {
        const auto &mr = DBCOM_MAGICRECORD(nDC);
        fflassert(mr);

        switch(magicElemID(mr.elem)){
            case MET_FIRE   : return node.dcElem.fire;
            case MET_ICE    : return node.dcElem.ice;
            case MET_LIGHT  : return node.dcElem.light;
            case MET_WIND   : return node.dcElem.wind;
            case MET_HOLY   : return node.dcElem.holy;
            case MET_DARK   : return node.dcElem.dark;
            case MET_PHANTOM: return node.dcElem.phantom;
            default         : return 0;
        }
    }();

    switch(nDC){
        case DBCOM_MAGICID(u8"物理攻击"):
            {
                return PlainPhyDamage
                {
                    .damage = node.randPickDC(),
                    .dcHit = node.dcHit,
                };
            }
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                return MagicDamage
                {
                    .magicID = nDC,
                    .damage = to_d(std::lround(node.randPickSC() * elemRatio)),
                    .mcHit = node.mcHit,
                };
            }
        case DBCOM_MAGICID(u8"雷电术"):
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"疾光电影"):
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
            {
                return MagicDamage
                {
                    .magicID = nDC,
                    .damage = to_d(std::lround(node.randPickMC() * elemRatio)),
                    .mcHit = node.mcHit,
                };
            }
        default:
            {
                return {};
            }
    }
}

bool Player::struckDamage(const DamageNode &node)
{
    // hack for debug
    // make the player never die
    return true;

    if(node){
        const auto damage = [&node, this]() -> int
        {
            const auto combatNode = getCombatNode(m_sdItemStorage.wear, {}, UID(), level());
            if(DBCOM_MAGICID(u8"物理攻击") == to_u32(node.magicID)){
                return std::max<int>(0, node.damage - mathf::rand<int>(combatNode.ac[0], combatNode.ac[1]));
            }

            const double elemRatio = std::max<double>(0.0, 1.0 + 0.1 * [&node, &combatNode, this]() -> int
            {
                const auto &mr = DBCOM_MAGICRECORD(node.magicID);
                fflassert(mr);

                switch(magicElemID(mr.elem)){
                    case MET_FIRE   : return combatNode.acElem.fire;
                    case MET_ICE    : return combatNode.acElem.ice;
                    case MET_LIGHT  : return combatNode.acElem.light;
                    case MET_WIND   : return combatNode.acElem.wind;
                    case MET_HOLY   : return combatNode.acElem.holy;
                    case MET_DARK   : return combatNode.acElem.dark;
                    case MET_PHANTOM: return combatNode.acElem.phantom;
                    default         : return 0;
                }
            }());
            return std::max<int>(0, node.damage - std::lround(mathf::rand<int>(combatNode.mac[0], combatNode.mac[1]) * elemRatio));
        }();

        if(damage > 0){
            m_sdHealth.HP = std::max<int>(0, m_sdHealth.HP - damage);
            reportHealth();
            dispatchHealth();

            if(m_sdHealth.HP <= 0){
                goDie();
            }
        }
        return true;
    }
    return false;
}

bool Player::ActionValid(const ActionNode &)
{
    return true;
}

void Player::dispatchOffline()
{
    if(true
            && checkActorPod()
            && m_map
            && m_map->checkActorPod()){

        AMOffline amO;
        std::memset(&amO, 0, sizeof(amO));

        amO.UID   = UID();
        amO.mapID = mapID();
        amO.X     = X();
        amO.Y     = Y();

        m_actorPod->forward(m_map->UID(), {AM_OFFLINE, amO});
        return;
    }

    g_monoServer->addLog(LOGTYPE_WARNING, "Can't dispatch offline event");
}

void Player::reportOffline(uint64_t nUID, uint32_t nMapID)
{
    if(true
            && nUID
            && nMapID
            && channID()){

        SMOffline smO;
        smO.UID   = nUID;
        smO.mapID = nMapID;

        g_netDriver->Post(channID(), SM_OFFLINE, smO);
    }
}

bool Player::Offline()
{
    dispatchOffline();
    reportOffline(UID(), mapID());
    dbUpdateMapGLoc();

    deactivate();
    return true;
}

bool Player::postNetMessage(uint8_t nHC, const void *pData, size_t nDataLen)
{
    if(channID()){
        return g_netDriver->Post(channID(), nHC, (const uint8_t *)(pData), nDataLen);
    }
    return false;
}

void Player::onCMActionStand(CMAction stCMA)
{
    int nX = stCMA.action.x;
    int nY = stCMA.action.y;
    int nDirection = stCMA.action.direction;

    if(true
            && m_map
            && m_map->validC(nX, nY)){

        // server get report stand
        // means client is trying to re-sync
        // try client's current location and always response

        switch(estimateHop(nX, nY)){
            case 1:
                {
                    requestMove(nX, nY, SYS_MAXSPEED, false, false,
                    [this, stCMA]()
                    {
                        onCMActionStand(stCMA);
                    },
                    [this]()
                    {
                        reportStand();
                    });
                    return;
                }
            case 0:
            default:
                {
                    if(directionValid(nDirection)){
                        m_direction = nDirection;
                    }

                    reportStand();
                    return;
                }
        }
    }
}

void Player::onCMActionMove(CMAction stCMA)
{
    // server won't do any path finding
    // client should sent action with only one-hop movement

    int nX0 = stCMA.action.x;
    int nY0 = stCMA.action.y;
    int nX1 = stCMA.action.aimX;
    int nY1 = stCMA.action.aimY;

    switch(estimateHop(nX0, nY0)){
        case 0:
            {
                requestMove(nX1, nY1, MoveSpeed(), false, false, [this]()
                {
                    dbUpdateMapGLoc();
                },

                [this]()
                {
                    reportStand();
                });
                return;
            }
        case 1:
            {
                requestMove(nX0, nY0, SYS_MAXSPEED, false, false, [this, stCMA]()
                {
                    onCMActionMove(stCMA);
                },
                [this]()
                {
                    reportStand();
                });
                return;
            }
        default:
            {
                reportStand();
                return;
            }
    }
}

void Player::onCMActionAttack(CMAction stCMA)
{
    getCOLocation(stCMA.action.aimUID, [this, stCMA](const COLocation &rstLocation)
    {
        int nX0 = stCMA.action.x;
        int nY0 = stCMA.action.y;

        int nDCType = stCMA.action.extParam.attack.damageID;
        uint64_t nAimUID = stCMA.action.aimUID;

        if(rstLocation.mapID == mapID()){
            switch(nDCType){
                case DBCOM_MAGICID(u8"物理攻击"):
                    {
                        switch(estimateHop(nX0, nY0)){
                            case 0:
                                {
                                    switch(mathf::LDistance2(nX0, nY0, rstLocation.x, rstLocation.y)){
                                        case 1:
                                        case 2:
                                            {
                                                dispatchAttackDamage(nAimUID, nDCType);
                                                return;
                                            }
                                        default:
                                            {
                                                return;
                                            }
                                    }
                                    return;
                                }
                            case 1:
                                {
                                    requestMove(nX0, nY0, SYS_MAXSPEED, false, false,
                                    [this, stCMA]()
                                    {
                                        onCMActionAttack(stCMA);
                                    },
                                    [this]()
                                    {
                                        reportStand();
                                    });
                                    return;
                                }
                            default:
                                {
                                    return;
                                }
                        }
                        return;
                    }
                default:
                    {
                        return;
                    }
            }
        }
    });
}

void Player::onCMActionSpell(CMAction cmA)
{
    fflassert(cmA.action.type == ACTION_SPELL);
    const int magicID = cmA.action.extParam.spell.magicID;
    dispatchAction(cmA.action);

    const auto node = getCombatNode(m_sdItemStorage.wear, m_sdLearnedMagicList, UID(), level());
    switch(magicID){
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"灵魂火符"):
        case DBCOM_MAGICID(u8"冰月神掌"):
        case DBCOM_MAGICID(u8"冰月震天"):
            {
                // 灵魂火符 doesn't need to send back the CASTMAGIC message
                // the ACTION_SPELL creates the magic

                if(cmA.action.aimUID){
                    getCOLocation(cmA.action.aimUID, [this, cmA](const COLocation &coLoc)
                    {
                        const auto ld = mathf::LDistance<float>(coLoc.x, coLoc.y, cmA.action.x, cmA.action.y);
                        const auto delay = ld * 100;

                        addDelay(delay, [cmA, this]()
                        {
                            dispatchAttackDamage(cmA.action.aimUID, cmA.action.extParam.spell.magicID);
                        });
                    });
                }
                break;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.mapID  = mapID();
                smFM.Magic  = magicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = cmA.action.x;
                smFM.Y      = cmA.action.y;
                smFM.AimUID = cmA.action.aimUID;

                addDelay(1400, [this, smFM]()
                {
                    dispatchNetPackage(true, SM_CASTMAGIC, smFM);
                    addDelay(300, [smFM, this]()
                    {
                        dispatchAttackDamage(smFM.AimUID, DBCOM_MAGICID(u8"雷电术"));
                    });
                });
                break;
            }
        case DBCOM_MAGICID(u8"魔法盾"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();

                addDelay(800, [this, smFM]()
                {
                    dispatchNetPackage(true, SM_CASTMAGIC, smFM);
                    addDelay(10000, [this]()
                    {
                        SMBuff smB;
                        std::memset(&smB, 0, sizeof(smB));

                        smB.uid   = UID();
                        smB.type  = BFT_SHIELD;
                        smB.state = BFS_OFF;
                        dispatchNetPackage(true, SM_BUFF, smB);
                    });
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤骷髅"):
            {
                int nFrontX = -1;
                int nFrontY = -1;
                PathFind::GetFrontLocation(&nFrontX, &nFrontY, X(), Y(), Direction(), 2);

                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.mapID = mapID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                addDelay(600, [this, smFM]()
                {
                    addMonster(DBCOM_MONSTERID(u8"变异骷髅"), smFM.AimX, smFM.AimY, false);

                    // addMonster will send ACTION_SPAWN to client
                    // client then use it to play the magic for 召唤骷髅, we don't send magic message here
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤神兽"):
            {
                int nFrontX = -1;
                int nFrontY = -1;
                PathFind::GetFrontLocation(&nFrontX, &nFrontY, X(), Y(), Direction(), 2);

                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.mapID = mapID();
                smFM.Magic = magicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                addDelay(1000, [this, smFM]()
                {
                    addMonster(DBCOM_MONSTERID(u8"神兽"), smFM.AimX, smFM.AimY, false);
                });
                break;
            }
        case DBCOM_MAGICID(u8"火墙"):
            {
                addDelay(550, [this, cmA, node]()
                {
                    AMCastFireWall amCFW;
                    std::memset(&amCFW, 0, sizeof(amCFW));

                    amCFW.minDC = node.mc[0];
                    amCFW.maxDC = node.mc[1];
                    amCFW.mcHit = node.mcHit;

                    amCFW.duration = 20 * 1000;
                    amCFW.dps      = 2;

                    // not 3x3
                    // fire wall takes grids as a cross
                    //
                    // +---+---+---+
                    // |   | v |   |
                    // +---+---+---+
                    // | v | v | v |
                    // +---+---+---+
                    // |   | v |   |
                    // +---+---+---+

                    for(const int dir: {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT}){
                        if(dir == DIR_NONE){
                            amCFW.x = cmA.action.aimX;
                            amCFW.y = cmA.action.aimY;
                        }
                        else{
                            std::tie(amCFW.x, amCFW.y) = pathf::getFrontGLoc(cmA.action.aimX, cmA.action.aimY, dir, 1);
                        }

                        if(m_map->groundValid(amCFW.x, amCFW.y)){
                            m_actorPod->forward(m_map->UID(), {AM_CASTFIREWALL, amCFW});
                        }
                    }
                });
                break;
            }
        case DBCOM_MAGICID(u8"地狱火"):
        case DBCOM_MAGICID(u8"冰沙掌"):
        case DBCOM_MAGICID(u8"疾光电影"):
            {
                if(const auto dirIndex = pathf::getDir8(cmA.action.aimX - cmA.action.x, cmA.action.aimY - cmA.action.y); (dirIndex >= 0) && directionValid(dirIndex + DIR_BEGIN)){
                    m_direction = dirIndex + DIR_BEGIN;
                }

                std::set<std::tuple<int, int>> pathGridList;
                switch(Direction()){
                    case DIR_UP:
                    case DIR_DOWN:
                    case DIR_LEFT:
                    case DIR_RIGHT:
                        {
                            for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                pathGridList.insert({pathGX, pathGY});

                                if(distance > 3){
                                    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                    pathGridList.insert({pathGX + sgnDY, pathGY + sgnDX}); // switch sgnDX and sgnDY and plus/minus
                                    pathGridList.insert({pathGX - sgnDY, pathGY - sgnDX});
                                }
                            }
                            break;
                        }
                    case DIR_UPLEFT:
                    case DIR_UPRIGHT:
                    case DIR_DOWNLEFT:
                    case DIR_DOWNRIGHT:
                        {
                            for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                const auto [pathGX, pathGY] = pathf::getFrontGLoc(X(), Y(), Direction(), distance);
                                pathGridList.insert({pathGX, pathGY});

                                const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, Direction(), 1);
                                pathGridList.insert({pathGX + sgnDX, pathGY        });
                                pathGridList.insert({pathGX        , pathGY + sgnDY});
                            }
                            break;
                        }
                    default:
                        {
                            throw bad_reach();
                        }
                }

                AMStrikeFixedLocDamage amSFLD;
                std::memset(&amSFLD, 0, sizeof(amSFLD));

                for(const auto [pathGX, pathGY]: pathGridList){
                    if(m_map->groundValid(pathGX, pathGY)){
                        amSFLD.x = pathGX;
                        amSFLD.y = pathGY;
                        amSFLD.damage = getAttackDamage(magicID);
                        addDelay(550 + mathf::CDistance(X(), Y(), amSFLD.x, amSFLD.y) * 100, [amSFLD, castMapID = mapID(), this]()
                        {
                            if(castMapID == mapID()){
                                m_actorPod->forward(m_map->UID(), {AM_STRIKEFIXEDLOCDAMAGE, amSFLD});
                                if(g_serverArgParser->showStrikeGrid){
                                    SMStrikeGrid smSG;
                                    std::memset(&smSG, 0, sizeof(smSG));

                                    smSG.x = amSFLD.x;
                                    smSG.y = amSFLD.y;
                                    postNetMessage(SM_STRIKEGRID, smSG);
                                }
                            }
                        });
                    }
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

int Player::MaxStep() const
{
    if(Horse()){
        return 3;
    }else{
        return 2;
    }
}

void Player::recoverHealth()
{
    const auto lastHP = m_sdHealth.HP;
    const auto lastMP = m_sdHealth.MP;

    m_sdHealth.HP = std::min<int>(m_sdHealth.maxHP, m_sdHealth.HP + std::max<int>(m_sdHealth.maxHP / 60, 1));
    m_sdHealth.MP = std::min<int>(m_sdHealth.maxMP, m_sdHealth.MP + std::max<int>(m_sdHealth.maxMP / 60, 1));

    if((lastHP != m_sdHealth.HP) || (lastMP != m_sdHealth.MP)){
        reportHealth();
    }
}

void Player::gainExp(int addedExp)
{
    if(addedExp <= 0){
        return;
    }

    m_exp += addedExp;
    dbUpdateExp();
    postExp();
}

void Player::PullRectCO(int nW, int nH)
{
    if(true
            && nW > 0
            && nH > 0
            && checkActorPod()
            && m_map->checkActorPod()){

        AMPullCOInfo amPCOI;
        std::memset(&amPCOI, 0, sizeof(amPCOI));

        amPCOI.X     = X();
        amPCOI.Y     = Y();
        amPCOI.W     = nW;
        amPCOI.H     = nH;
        amPCOI.UID   = UID();
        amPCOI.mapID = m_map->ID();
        m_actorPod->forward(m_map->UID(), {AM_PULLCOINFO, amPCOI});
    }
}

bool Player::CanPickUp(uint32_t, uint32_t)
{
    return true;
}

void Player::reportGold()
{
    SMGold smG;
    std::memset(&smG, 0, sizeof(smG));
    smG.gold = gold();
    postNetMessage(SM_GOLD, smG);
}

void Player::reportRemoveItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    SMRemoveItem smRI;
    std::memset(&smRI, 0, sizeof(smRI));

    smRI.itemID = itemID;
    smRI. seqID =  seqID;
    smRI. count =  count;
    postNetMessage(SM_REMOVEITEM, smRI);
}

void Player::reportSecuredItemList()
{
    postNetMessage(SM_SHOWSECUREDITEMLIST, cerealf::serialize(SDShowSecuredItemList
    {
        .itemList = dbLoadSecuredItemList(),
    }));
}

void Player::checkFriend(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!nUID){
        throw fflerror("invalid zero UID");
    }

    switch(uidf::getUIDType(nUID)){
        case UID_NPC:
            {
                fnOp(FT_NEUTRAL);
                return;
            }
        case UID_PLY:
            {
                fnOp(isOffender(nUID) ? FT_ENEMY : FT_NEUTRAL);
                return;
            }
        case UID_MON:
            {
                if(!DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).tameable){
                    fnOp(FT_ENEMY);
                    return;
                }

                queryFinalMaster(nUID, [this, nUID, fnOp](uint64_t nFMasterUID)
                {
                    switch(uidf::getUIDType(nFMasterUID)){
                        case UID_PLY:
                            {
                                fnOp(isOffender(nUID) ? FT_ENEMY : FT_NEUTRAL);
                                return;
                            }
                        case UID_MON:
                            {
                                fnOp(FT_ENEMY);
                                return;
                            }
                        default:
                            {
                                throw fflerror("final master is not PLY nor MON");
                            }
                    }
                });
                return;
            }
        default:
            {
                throw fflerror("checking friend type for: %s", uidf::getUIDTypeCStr(nUID));
            }
    }
}

void Player::RequestKillPets()
{
    for(auto uid: m_slaveList){
        m_actorPod->forward(uid, {AM_MASTERKILL});
    }
    m_slaveList.clear();
}

void Player::postOnLoginOK()
{
    postBuildVersion();
    postNetMessage(SM_LOGINOK, cerealf::serialize(SDLoginOK
    {
        .uid = UID(),
        .mapID = mapID(),

        .x = X(),
        .y = Y(),
        .direction = Direction(),

        .desp
        {
            .wear = m_sdItemStorage.wear,
            .hair = m_hair,
            .hairColor = m_hairColor,
        },

        .name = m_name,
        .nameColor = m_nameColor,
    }, true));

    postExp();
    postNetMessage(SM_INVENTORY,        cerealf::serialize(m_sdItemStorage.inventory));
    postNetMessage(SM_BELT,             cerealf::serialize(m_sdItemStorage.belt));
    postNetMessage(SM_LEARNEDMAGICLIST, cerealf::serialize(m_sdLearnedMagicList));
    postNetMessage(SM_RUNTIMECONFIG,    cerealf::serialize(m_sdRuntimeConfig));
}

bool Player::hasInventoryItem(uint32_t itemID, uint32_t seqID, size_t count) const
{
    return m_sdItemStorage.inventory.has(itemID, seqID) >= count;
}

const SDItem &Player::addInventoryItem(SDItem item, bool keepSeqID)
{
    const auto &addedItem = m_sdItemStorage.inventory.add(std::move(item), keepSeqID);
    dbUpdateInventoryItem(addedItem);
    postNetMessage(SM_UPDATEITEM, cerealf::serialize(SDUpdateItem
    {
        .item = addedItem,
    }));
    return addedItem;
}

size_t Player::removeInventoryItem(const SDItem &item)
{
    return removeInventoryItem(item.itemID, item.seqID);
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    fflassert(seqID > 0);
    return removeInventoryItem(itemID, seqID, SIZE_MAX);
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    const auto &ir = DBCOM_ITEMRECORD(itemID);

    fflassert(ir);
    fflassert(count > 0);
    fflassert(!ir.isGold());

    size_t doneCount = 0;
    while(doneCount < count){
        const auto [removedCount, removedSeqID, itemPtr] = m_sdItemStorage.inventory.remove(itemID, seqID, count - doneCount);
        if(!removedCount){
            break;
        }

        if(itemPtr){
            dbUpdateInventoryItem(*itemPtr);
        }
        else{
            dbRemoveInventoryItem(itemID, removedSeqID);
        }

        doneCount += removedCount;
        reportRemoveItem(itemID, removedSeqID, removedCount);
    }
    return doneCount;
}

const SDItem &Player::findInventoryItem(uint32_t itemID, uint32_t seqID) const
{
    fflassert(DBCOM_ITEMRECORD(itemID));
    return m_sdItemStorage.inventory.find(itemID, seqID);
}

void Player::addSecuredItem(uint32_t itemID, uint32_t seqID)
{
    fflassert(findInventoryItem(itemID, seqID));
    dbSecureItem(itemID, seqID);
    removeInventoryItem(itemID, seqID);
}

void Player::removeSecuredItem(uint32_t itemID, uint32_t seqID)
{
    addInventoryItem(dbRetrieveSecuredItem(itemID, seqID), false);

    SMRemoveSecuredItem smRSI;
    std::memset(&smRSI, 0, sizeof(smRSI));

    smRSI.itemID = itemID;
    smRSI. seqID =  seqID;
    postNetMessage(SM_REMOVESECUREDITEM, smRSI);
}

void Player::setGold(size_t gold)
{
    m_sdItemStorage.gold = gold;
    g_dbPod->exec("update tbl_dbid set fld_gold = %llu where fld_dbid = %llu", to_llu(m_sdItemStorage.gold), to_llu(dbid()));
    reportGold();
}

void Player::setWLItem(int wltype, SDItem item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("bad wltype: %d", wltype);
    }

    m_sdItemStorage.wear.setWLItem(wltype, item);
    const auto sdEquipWearBuf = cerealf::serialize(SDEquipWear
    {
        .uid = UID(),
        .wltype = wltype,
        .item = item,
    });

    foreachInViewCO([sdEquipWearBuf, this](const COLocation &coLoc)
    {
        if(uidf::getUIDType(coLoc.uid) == UID_PLY){
            forwardNetPackage(coLoc.uid, SM_EQUIPWEAR, sdEquipWearBuf);
        }
    });
}

void Player::postBuildVersion()
{
    SMBuildVersion smBV;
    std::memset(&smBV, 0, sizeof(smBV));
    std::strcpy(smBV.version, getBuildSignature());
    postNetMessage(SM_BUILDVERSION, smBV);
}

void Player::postExp()
{
    SMExp smE;
    std::memset(&smE, 0, sizeof(smE));
    smE.exp = exp();
    postNetMessage(SM_EXP, smE);
}

bool Player::canWear(uint32_t itemID, int wltype) const
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wltype: %d", wltype);
    }

    if(!itemID){
        throw fflerror("invalid itemID: %llu", to_llu(itemID));
    }

    const auto &ir = DBCOM_ITEMRECORD(itemID);
    if(!ir){
        return false;
    }

    if(!ir.wearable(wltype)){
        return false;
    }

    if(wltype == WLG_DRESS && getClothGender(itemID) != gender()){
        return false;
    }

    // TODO
    // check item requirement

    return true;
}

std::vector<std::string> Player::parseNPCQuery(const char *query)
{
    fflassert(str_haschar(query));

    const char *beginPtr = query;
    const char *endPtr   = query + std::strlen(query);

    std::vector<std::string> result;
    while(true){
        beginPtr = std::find_if_not(beginPtr, endPtr, [](char chByte)
        {
            return chByte == ' ';
        });

        if(beginPtr == endPtr){
            break;
        }

        const char *donePtr = std::find(beginPtr, endPtr, ' ');
        result.emplace_back(beginPtr, donePtr);
        beginPtr = donePtr;
    }
    return result;
}
