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
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "sysconst.hpp"
#include "netdriver.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"

extern DBPod *g_dbPod;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

Player::Player(uint32_t nDBID,
        ServiceCore    *pServiceCore,
        ServerMap      *pServerMap,
        int             nMapX,
        int             nMapY,
        int             nDirection)
    : CharObject(pServiceCore, pServerMap, uidf::buildPlayerUID(nDBID), nMapX, nMapY, nDirection)
    , m_DBID(nDBID)
    , m_jobID(0)        // will provide after bind
    , m_channID(0)    // provide by bind
    , m_exp(0)
    , m_level(0)        // after bind
    , m_gold(0)
    , m_inventory()
    , m_name([this]() -> std::string
      {
          std::string result;
          dbAccess("tbl_dbid", "fld_name", [&result](std::string name) -> std::string
          {
              result = std::move(name);
              return {};
          });
          return result;
      }())
{
    m_HP    = 10;
    m_HPMax = 10;
    m_MP    = 10;
    m_MPMax = 10;

    m_stateTrigger.install([this, lastCheckTick = (uint32_t)(0)]() mutable -> bool
    {
        if(const auto currTick = g_monoServer->getCurrTick(); currTick >= (lastCheckTick + 1000)){
            RecoverHealth();
            lastCheckTick = currTick;
        }
        return false;
    });
}

Player::~Player()
{
    DBSavePlayer();
}

void Player::operateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                on_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_BADACTORPOD:
            {
                on_MPK_BADACTORPOD(rstMPK);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                on_MPK_NOTIFYNEWCO(rstMPK);
                break;
            }
        case MPK_CHECKMASTER:
            {
                on_MPK_CHECKMASTER(rstMPK);
                break;
            }
        case MPK_MAPSWITCH:
            {
                on_MPK_MAPSWITCH(rstMPK);
                break;
            }
        case MPK_NPCQUERY:
            {
                on_MPK_NPCQUERY(rstMPK);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                on_MPK_QUERYLOCATION(rstMPK);
                break;
            }
        case MPK_QUERYFRIENDTYPE:
            {
                on_MPK_QUERYFRIENDTYPE(rstMPK);
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
        case MPK_UPDATEHP:
            {
                on_MPK_UPDATEHP(rstMPK);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                on_MPK_DEADFADEOUT(rstMPK);
                break;
            }
        case MPK_SHOWDROPITEM:
            {
                on_MPK_SHOWDROPITEM(rstMPK);
                break;
            }
        case MPK_BINDCHANNEL:
            {
                on_MPK_BINDCHANNEL(rstMPK);
                break;
            }
        case MPK_SENDPACKAGE:
            {
                on_MPK_SENDPACKAGE(rstMPK);
                break;
            }
        case MPK_RECVPACKAGE:
            {
                on_MPK_RECVPACKAGE(rstMPK);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                on_MPK_QUERYCORECORD(rstMPK);
                break;
            }
        case MPK_BADCHANNEL:
            {
                on_MPK_BADCHANNEL(rstMPK);
                break;
            }
        case MPK_OFFLINE:
            {
                on_MPK_OFFLINE(rstMPK);
                break;
            }
        case MPK_REMOVEGROUNDITEM:
            {
                on_MPK_REMOVEGROUNDITEM(rstMPK);
                break;
            }
        case MPK_PICKUPOK:
            {
                on_MPK_PICKUPOK(rstMPK);
                break;
            }
        case MPK_CORECORD:
            {
                on_MPK_CORECORD(rstMPK);
                break;
            }
        case MPK_NOTIFYDEAD:
            {
                on_MPK_NOTIFYDEAD(rstMPK);
                break;
            }
        case MPK_NPCXMLLAYOUT:
            {
                on_MPK_NPCXMLLAYOUT(rstMPK);
                break;
            }
        case MPK_NPCSELL:
            {
                on_MPK_NPCSELL(rstMPK);
                break;
            }
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", mpkName(rstMPK.Type()));
                break;
            }
    }
}

void Player::operateNet(uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
        case CM_QUERYCORECORD   : net_CM_QUERYCORECORD   (nType, pData, nDataLen); break;
        case CM_REQUESTKILLPETS : net_CM_REQUESTKILLPETS (nType, pData, nDataLen); break;
        case CM_REQUESTSPACEMOVE: net_CM_REQUESTSPACEMOVE(nType, pData, nDataLen); break;
        case CM_ACTION          : net_CM_ACTION          (nType, pData, nDataLen); break;
        case CM_PICKUP          : net_CM_PICKUP          (nType, pData, nDataLen); break;
        case CM_PING            : net_CM_PING            (nType, pData, nDataLen); break;
        case CM_QUERYGOLD       : net_CM_QUERYGOLD       (nType, pData, nDataLen); break;
        case CM_NPCEVENT        : net_CM_NPCEVENT        (nType, pData, nDataLen); break;
        case CM_QUERYSELLITEM   : net_CM_QUERYSELLITEM   (nType, pData, nDataLen); break;
        default                 :                                                  break;
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
    amCOR.MapID = MapID();
    amCOR.action = ActionStand
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    };

    amCOR.Player.DBID  = DBID();
    amCOR.Player.JobID = JobID();
    amCOR.Player.Level = Level();

    m_actorPod->forward(toUID, {MPK_CORECORD, amCOR});
}

void Player::reportStand()
{
    reportAction(UID(), ActionStand
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    });
}

void Player::reportAction(uint64_t nUID, const ActionNode &action)
{
    if(true
            && nUID
            && ChannID()){

        SMAction smA;
        std::memset(&smA, 0, sizeof(smA));

        smA.UID = nUID;
        smA.MapID = MapID();
        smA.action = action;

        g_netDriver->Post(ChannID(), SM_ACTION, smA);
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
    SMUpdateHP smUHP;
    std::memset(&smUHP, 0, sizeof(smUHP));

    smUHP.UID   = UID();
    smUHP.MapID = MapID();
    smUHP.HP    = HP();
    smUHP.HPMax = HPMax();

    postNetMessage(SM_UPDATEHP, smUHP);
}

bool Player::InRange(int nRangeType, int nX, int nY)
{
    if(!m_map->ValidC(nX, nY)){
        return false;
    }

    switch(nRangeType){
        case RANGE_VISIBLE:
            {
                return mathf::LDistance2(X(), Y(), nX, nY) < 20 * 20;
            }
        case RANGE_ATTACK:
            {
                return mathf::LDistance2(X(), Y(), nX, nY) < 10 * 10;
            }
        default:
            {
                break;
            }
    }
    return false;
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
    amDFO.MapID = MapID();
    amDFO.X     = X();
    amDFO.Y     = Y();

    if(true
            && checkActorPod()
            && m_map
            && m_map->checkActorPod()){
        m_actorPod->forward(m_map->UID(), {MPK_DEADFADEOUT, amDFO});
    }

    deactivate();
    return true;
}

bool Player::DCValid(int, bool)
{
    return true;
}

DamageNode Player::GetAttackDamage(int nDC)
{
    switch(nDC){
        case DC_PHY_PLAIN:
            {
                return {UID(), nDC, 5, EC_NONE};
            }
        default:
            {
                break;
            }
    }

    return {};
}

bool Player::StruckDamage(const DamageNode &rstDamage)
{
    // hack for debug
    // make the player never die
    return true;

    if(rstDamage){
        m_HP = (std::max<int>)(0, HP() - rstDamage.Damage);
        reportHealth();
        dispatchHealth();

        if(HP() <= 0){
            goDie();
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
        amO.MapID = MapID();
        amO.X     = X();
        amO.Y     = Y();

        m_actorPod->forward(m_map->UID(), {MPK_OFFLINE, amO});
        return;
    }

    g_monoServer->addLog(LOGTYPE_WARNING, "Can't dispatch offline event");
}

void Player::reportOffline(uint64_t nUID, uint32_t nMapID)
{
    if(true
            && nUID
            && nMapID
            && ChannID()){

        SMOffline smO;
        smO.UID   = nUID;
        smO.MapID = nMapID;

        g_netDriver->Post(ChannID(), SM_OFFLINE, smO);
    }
}

bool Player::Offline()
{
    dispatchOffline();
    reportOffline(UID(), MapID());

    deactivate();
    return true;
}

bool Player::postNetMessage(uint8_t nHC, const void *pData, size_t nDataLen)
{
    if(ChannID()){
        return g_netDriver->Post(ChannID(), nHC, (const uint8_t *)(pData), nDataLen);
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
            && m_map->ValidC(nX, nY)){

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
                requestMove(nX1, nY1, MoveSpeed(), false, false, nullptr, [this]()
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
    retrieveLocation(stCMA.action.aimUID, [this, stCMA](const COLocation &rstLocation)
    {
        int nX0 = stCMA.action.x;
        int nY0 = stCMA.action.y;

        int nDCType = stCMA.action.extParam.attack.damageID;
        uint64_t nAimUID = stCMA.action.aimUID;

        if(rstLocation.MapID == MapID()){
            switch(nDCType){
                case DC_PHY_PLAIN:
                case DC_PHY_WIDESWORD:
                case DC_PHY_FIRESWORD:
                    {
                        switch(estimateHop(nX0, nY0)){
                            case 0:
                                {
                                    switch(mathf::LDistance2(nX0, nY0, rstLocation.X, rstLocation.Y)){
                                        case 1:
                                        case 2:
                                            {
                                                dispatchAttack(nAimUID, nDCType);
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
    if(cmA.action.type != ACTION_SPELL){
        throw fflerror("invalid action type: %s", actionName(cmA.action));
    }

    int nX = cmA.action.x;
    int nY = cmA.action.y;
    int nMagicID = cmA.action.extParam.spell.magicID;

    switch(nMagicID){
        case DBCOM_MAGICID(u8"灵魂火符"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.MapID  = MapID();
                smFM.Magic  = nMagicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = nX;
                smFM.Y      = nY;
                smFM.AimUID = cmA.action.aimUID;

                addDelay(800, [this, smFM]()
                {
                    g_netDriver->Post(ChannID(), SM_CASTMAGIC, smFM);
                });
                break;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.MapID  = MapID();
                smFM.Magic  = nMagicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = nX;
                smFM.Y      = nY;
                smFM.AimUID = cmA.action.aimUID;

                addDelay(1400, [this, smFM]()
                {
                    g_netDriver->Post(ChannID(), SM_CASTMAGIC, smFM);
                });
                break;
            }
        case DBCOM_MAGICID(u8"魔法盾"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.Magic = nMagicID;
                smFM.Speed = MagicSpeed();

                addDelay(800, [this, smFM]()
                {
                    g_netDriver->Post(ChannID(), SM_CASTMAGIC, smFM);
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
                smFM.MapID = MapID();
                smFM.Magic = nMagicID;
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
                smFM.MapID = MapID();
                smFM.Magic = nMagicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                addDelay(1000, [this, smFM]()
                {
                    addMonster(DBCOM_MONSTERID(u8"神兽"), smFM.AimX, smFM.AimY, false);
                });
                break;
            }
        default:
            {
                break;
            }
    }
}

void Player::onCMActionPickUp(CMAction stCMA)
{
    switch(estimateHop(stCMA.action.x, stCMA.action.y)){
        case 0:
            {
                AMPickUp amPU;
                amPU.X    = stCMA.action.x;
                amPU.Y    = stCMA.action.y;
                amPU.UID  = UID();
                amPU.ID   = stCMA.action.extParam.pickUp.itemID;
                amPU.DBID = 0;

                m_actorPod->forward(m_map->UID(), {MPK_PICKUP, amPU});
                return;
            }
        case 1:
            {
                requestMove(stCMA.action.x, stCMA.action.y, SYS_MAXSPEED, false, false,
                [this, stCMA]()
                {
                    onCMActionPickUp(stCMA);
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
}

int Player::MaxStep() const
{
    if(Horse()){
        return 3;
    }else{
        return 2;
    }
}

void Player::RecoverHealth()
{
    auto fnGetAdd = [](int nCurr, int nMax) -> int
    {
        if(true
                && nCurr >= 0
                && nMax  >= 0
                && nCurr <= nMax){

            auto nAdd = (std::max<int>)(nMax / 60, 1);
            return (std::min<int>)(nAdd, nMax - nCurr);
        }
        return 0;
    };

    auto nAddHP = fnGetAdd(m_HP, m_HPMax);
    auto nAddMP = fnGetAdd(m_MP, m_MPMax);

    if((nAddHP > 0) || (nAddMP > 0)){

        m_HP += nAddHP;
        m_MP += nAddMP;

        reportHealth();
    }
}

void Player::GainExp(int nExp)
{
    if(nExp){
        if((int)(m_exp) + nExp < 0){
            m_exp = 0;
        }else{
            m_exp += (uint32_t)(nExp);
        }

        auto nLevelExp = GetLevelExp();
        if(m_exp >= nLevelExp){
            m_exp    = m_exp - nLevelExp;
            m_level += 1;
        }
    }
}

uint32_t Player::GetLevelExp()
{
    auto fnGetLevelExp = [](int nLevel, int nJobID) -> int
    {
        if(nLevel > 0 && nJobID >= 0){
            return 1000;
        }
        return -1;
    };
    return fnGetLevelExp(Level(), JobID());
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
        amPCOI.MapID = m_map->ID();
        m_actorPod->forward(m_map->UID(), {MPK_PULLCOINFO, amPCOI});
    }
}

bool Player::CanPickUp(uint32_t, uint32_t)
{
    return true;
}

size_t Player::dbUpdate(const char *tableName, const char *fieldList, ...)
{
    if(true
            && (tableName && tableName[0] != '\0')
            && (fieldList && fieldList[0] != '\0')){

        std::string sqlCmd;
        str_format(fieldList, sqlCmd);
        return g_dbPod->exec("update %s set %s where fld_dbid = %llu", tableName, sqlCmd.c_str(), to_llu(DBID()));
    }
    throw fflerror("invalid arguments: tableName = %s, fieldList = %s", to_cstr(tableName), to_cstr(fieldList));
}

size_t Player::dbAccess(const char *tableName, const char *fieldName, std::function<std::string(std::string)> op)
{
    if(true
            && op
            && (tableName && tableName[0] != '\0')
            && (fieldName && fieldName[0] != '\0')){

        // if need to return a string we should do:
        //     return "\"xxxx\"";
        // then empty string should be "\"\"", not result.empty()

        size_t execCount = 0;
        auto query = g_dbPod->createQuery("select %s from %s where fld_dbid = %llu", fieldName, tableName, to_llu(DBID()));
        while(query.executeStep()){
            if(const auto result = op((std::string)(query.getColumn(0))); !result.empty()){
                g_dbPod->exec("update %s set %s = %s where fld_dbid = %llu", tableName, fieldName, result.c_str(), to_llu(DBID()));
            }
            execCount++;
        }
        return execCount;
    }
    throw fflerror("invalid arguments: tableName = %s, fieldName = %s, op = %s", to_cstr(tableName), to_cstr(fieldName), op ? "invocable" : "null");
}

bool Player::DBLoadPlayer()
{
    return true;
}

bool Player::DBSavePlayer()
{
    return dbUpdate("tbl_dbid", "fld_gold = %d, fld_level = %d", Gold(), Level());
}

void Player::reportGold()
{
    SMGold smG;
    std::memset(&smG, 0, sizeof(smG));

    smG.Gold = Gold();
    postNetMessage(SM_GOLD, smG);
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
                if(!DBCOM_MONSTERRECORD(uidf::getMonsterID(nUID)).tamable){
                    fnOp(FT_ENEMY);
                    return;
                }

                QueryFinalMaster(nUID, [this, nUID, fnOp](uint64_t nFMasterUID)
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
                throw fflerror("checking friend type for: %s", uidf::getUIDTypeString(nUID));
            }
    }
}

void Player::RequestKillPets()
{
    for(auto uid: m_slaveList){
        m_actorPod->forward(uid, {MPK_MASTERKILL});
    }
    m_slaveList.clear();
}

bool Player::sendNetBuf(uint8_t hc, const uint8_t *buf, size_t bufLen)
{
    return g_netDriver->Post(ChannID(), hc, buf, bufLen);
}
