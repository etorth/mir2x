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
#include "colorf.hpp"
#include "dbcomid.hpp"
#include "sysconst.hpp"
#include "netdriver.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "buildconfig.hpp"

extern DBPod *g_dbPod;
extern NetDriver *g_netDriver;
extern MonoServer *g_monoServer;

Player::Player(const SDInitPlayer &initParam, ServiceCore *corePtr, ServerMap *mapPtr)
    : CharObject(corePtr, mapPtr, uidf::buildPlayerUID(initParam.dbid, true, initParam.jobList), initParam.x, initParam.y, DIR_DOWN)
    , m_exp(initParam.exp)
    , m_level(initParam.level)
    , m_name(initParam.name)
    , m_nameColor(initParam.nameColor)
    , m_hair(initParam.hair)
    , m_hairColor(initParam.hairColor)
{
    m_HP = initParam.hp;
    m_MP = initParam.mp;
    m_sdItemStorage.gold = initParam.gold;

    dbLoadWear();
    dbLoadBelt();
    dbLoadInventory();

    m_stateTrigger.install([this, lastCheckTick = (uint32_t)(0)]() mutable -> bool
    {
        if(const auto currTick = g_monoServer->getCurrTick(); currTick >= (lastCheckTick + 1000)){
            RecoverHealth();
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
        case AM_UPDATEHP:
            {
                on_AM_UPDATEHP(rstMPK);
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
        case CM_QUERYCORECORD    : net_CM_QUERYCORECORD    (nType, pData, nDataLen); break;
        case CM_REQUESTKILLPETS  : net_CM_REQUESTKILLPETS  (nType, pData, nDataLen); break;
        case CM_REQUESTSPACEMOVE : net_CM_REQUESTSPACEMOVE (nType, pData, nDataLen); break;
        case CM_ACTION           : net_CM_ACTION           (nType, pData, nDataLen); break;
        case CM_PICKUP           : net_CM_PICKUP           (nType, pData, nDataLen); break;
        case CM_PING             : net_CM_PING             (nType, pData, nDataLen); break;
        case CM_BUY              : net_CM_BUY              (nType, pData, nDataLen); break;
        case CM_QUERYGOLD        : net_CM_QUERYGOLD        (nType, pData, nDataLen); break;
        case CM_NPCEVENT         : net_CM_NPCEVENT         (nType, pData, nDataLen); break;
        case CM_QUERYSELLITEMLIST: net_CM_QUERYSELLITEMLIST(nType, pData, nDataLen); break;
        case CM_QUERYPLAYERWLDESP: net_CM_QUERYPLAYERWLDESP(nType, pData, nDataLen); break;
        default                  :                                                   break;
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
    amCOR.action = ActionStand
    {
        .x = X(),
        .y = Y(),
        .direction = Direction(),
    };

    amCOR.Player.Level = level();
    m_actorPod->forward(toUID, {AM_CORECORD, amCOR});
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
    SMUpdateHP smUHP;
    std::memset(&smUHP, 0, sizeof(smUHP));

    smUHP.UID   = UID();
    smUHP.mapID = mapID();
    smUHP.HP    = HP();
    smUHP.HPMax = HPMax();

    postNetMessage(SM_UPDATEHP, smUHP);
}

bool Player::InRange(int nRangeType, int nX, int nY)
{
    if(!m_map->validC(nX, nY)){
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

        if(rstLocation.mapID == mapID()){
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
                smFM.mapID  = mapID();
                smFM.Magic  = nMagicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = nX;
                smFM.Y      = nY;
                smFM.AimUID = cmA.action.aimUID;

                addDelay(800, [this, smFM]()
                {
                    g_netDriver->Post(channID(), SM_CASTMAGIC, smFM);
                });
                break;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                SMCastMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.mapID  = mapID();
                smFM.Magic  = nMagicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = nX;
                smFM.Y      = nY;
                smFM.AimUID = cmA.action.aimUID;

                addDelay(1400, [this, smFM]()
                {
                    g_netDriver->Post(channID(), SM_CASTMAGIC, smFM);
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
                    g_netDriver->Post(channID(), SM_CASTMAGIC, smFM);
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
                smFM.mapID = mapID();
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
    return fnGetLevelExp(level(), uidf::hasPlayerJob(UID(), JOB_TAOIST) ? JOB_TAOIST : JOB_WARRIOR);
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

bool Player::sendNetBuf(uint8_t hc, const uint8_t *buf, size_t bufLen)
{
    return g_netDriver->Post(channID(), hc, buf, bufLen);
}

void Player::postLoginOK()
{
    SMBuildVersion smBV;
    std::memset(&smBV, 0, sizeof(smBV));
    std::strcpy(smBV.version, getBuildSignature());
    postNetMessage(SM_BUILDVERSION, smBV);

    postNetMessage(SM_LOGINOK, cerealf::serialize<SDLoginOK>(SDLoginOK
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

    postNetMessage(SM_INVENTORY, cerealf::serialize(m_sdItemStorage.inventory, true));
    postNetMessage(SM_BELT,      cerealf::serialize(m_sdItemStorage.belt));
}

bool Player::hasInventoryItem(uint32_t itemID, uint32_t seqID, size_t count) const
{
    return m_sdItemStorage.inventory.has(itemID, seqID) >= count;
}

void Player::addInventoryItem(uint32_t itemID)
{
    addInventoryItem(SDItem
    {
        .itemID = itemID,
    }, false);
}

void Player::addInventoryItem(SDItem item, bool keepSeqID)
{
    const auto &addedItem = m_sdItemStorage.inventory.add(std::move(item), keepSeqID);
    dbUpdateInventoryItem(addedItem);
    postNetMessage(SM_ADDITEM, cerealf::serialize(SDAddItem
    {
        .item = addedItem,
    }));
}

size_t Player::removeInventoryItem(uint32_t itemID, uint32_t seqID, size_t count)
{
    if(!(DBCOM_ITEMRECORD(itemID) && count > 0)){
        throw fflerror("invalid arguments: itemID = %llu, seqID = %llu, count = %zu", to_llu(itemID), to_llu(seqID), count);
    }

    if(itemID == DBCOM_ITEMID(u8"金币")){
        throw fflerror("invalid item type: 金币");
    }

    size_t doneCount = 0;
    while(doneCount < count){
        const auto [removedCount, removedSeqID, itemPtr] = m_sdItemStorage.inventory.remove(itemID, seqID, count - doneCount);
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

void Player::setGold(size_t gold)
{
    m_sdItemStorage.gold = gold;
    g_dbPod->exec("update tbl_dbid set fld_gold = %llu where fld_dbid = %llu", to_llu(m_sdItemStorage.gold), to_llu(dbid()));
    reportGold();
}
