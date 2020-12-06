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

extern DBPod *g_DBPod;
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
          DBAccess("tbl_dbid", "fld_name", [&result](const char8_t *name) -> std::u8string
          {
              result = to_cstr(name);
              return {};
          });
          return result;
      }())
{
    m_HP    = 10;
    m_HPMax = 10;
    m_MP    = 10;
    m_MPMax = 10;

    m_stateHook.Install("RecoverHealth", [this, nLastTime = (uint32_t)(0)]() mutable -> bool
    {
        if(g_monoServer->getCurrTick() >= (nLastTime + 1000)){
            RecoverHealth();
            nLastTime = g_monoServer->getCurrTick();
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
        case MPK_NETPACKAGE:
            {
                on_MPK_NETPACKAGE(rstMPK);
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
        default:
            {
                g_monoServer->addLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
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

    stAMCOR.Player.DBID  = DBID();
    stAMCOR.Player.JobID = JobID();
    stAMCOR.Player.Level = Level();

    m_actorPod->forward(toUID, {MPK_CORECORD, stAMCOR});
}

void Player::reportStand()
{
    reportAction(UID(), ActionStand(X(), Y(), Direction()));
}

void Player::reportAction(uint64_t nUID, const ActionNode &rstAction)
{
    if(true
            && nUID
            && ChannID()){

        SMAction stSMA;
        std::memset(&stSMA, 0, sizeof(stSMA));

        stSMA.UID   = nUID;
        stSMA.MapID = MapID();

        stSMA.Action    = rstAction.Action;
        stSMA.Speed     = rstAction.Speed;
        stSMA.Direction = rstAction.Direction;

        stSMA.X    = rstAction.X;
        stSMA.Y    = rstAction.Y;
        stSMA.AimX = rstAction.AimX;
        stSMA.AimY = rstAction.AimY;

        stSMA.AimUID      = rstAction.AimUID;
        stSMA.ActionParam = rstAction.ActionParam;

        g_netDriver->Post(ChannID(), SM_ACTION, stSMA);
    }
}

void Player::reportDeadUID(uint64_t nDeadUID)
{
    SMNotifyDead stSMND;
    std::memset(&stSMND, 0, sizeof(stSMND));

    stSMND.UID = nDeadUID;
    postNetMessage(SM_NOTIFYDEAD, stSMND);
}

void Player::reportHealth()
{
    SMUpdateHP stSMUHP;
    std::memset(&stSMUHP, 0, sizeof(stSMUHP));

    stSMUHP.UID   = UID();
    stSMUHP.MapID = MapID();
    stSMUHP.HP    = HP();
    stSMUHP.HPMax = HPMax();

    postNetMessage(SM_UPDATEHP, stSMUHP);
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
    switch(GetState(STATE_NEVERDIE)){
        case 0:
            {
                switch(GetState(STATE_DEAD)){
                    case 0:
                        {
                            SetState(STATE_DEAD, 1);
                            Delay(2 * 1000, [this](){ goGhost(); });
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

bool Player::goGhost()
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
                                    && checkActorPod()
                                    && m_map
                                    && m_map->checkActorPod()){
                                m_actorPod->forward(m_map->UID(), {MPK_DEADFADEOUT, stAMDFO});
                            }

                            // 2. deactivate the actor here
                            //    disable the actorpod then no source can drive it
                            //    then current *this* can't be refered by any actor threads after this invocation
                            //    then MonoServer::EraseUID() is safe to delete *this*
                            //
                            //    don't do delete m_actorPod to disable the actor
                            //    since currently we are in the actor thread which accquired by m_actorPod
                            deactivate();
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

        AMOffline stAMO;
        std::memset(&stAMO, 0, sizeof(stAMO));

        stAMO.UID   = UID();
        stAMO.MapID = MapID();
        stAMO.X     = X();
        stAMO.Y     = Y();

        m_actorPod->forward(m_map->UID(), {MPK_OFFLINE, stAMO});
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

        SMOffline stSMO;
        stSMO.UID   = nUID;
        stSMO.MapID = nMapID;

        g_netDriver->Post(ChannID(), SM_OFFLINE, stSMO);
    }
}

bool Player::Offline()
{
    dispatchOffline();
    reportOffline(UID(), MapID());

    deactivate();
    return true;
}

bool Player::postNetMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    if(ChannID()){
        return g_netDriver->Post(ChannID(), nHC, pData, nDataLen);
    }
    return false;
}

void Player::onCMActionStand(CMAction stCMA)
{
    int nX = stCMA.X;
    int nY = stCMA.Y;
    int nDirection = stCMA.Direction;

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
                    if(true
                            && nDirection >= DIR_BEGIN
                            && nDirection <  DIR_END){
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

    int nX0 = stCMA.X;
    int nY0 = stCMA.Y;
    int nX1 = stCMA.AimX;
    int nY1 = stCMA.AimY;

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
    retrieveLocation(stCMA.AimUID, [this, stCMA](const COLocation &rstLocation)
    {
        int nX0 = stCMA.X;
        int nY0 = stCMA.Y;

        int nDCType = stCMA.ActionParam;
        uint64_t nAimUID = stCMA.AimUID;

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
    if(cmA.Action != ACTION_SPELL){
        throw fflerror("invalid action type: %d", cmA.Action);
    }

    int nX = cmA.X;
    int nY = cmA.Y;
    int nMagicID = cmA.ActionParam;

    switch(nMagicID){
        case DBCOM_MAGICID(u8"灵魂火符"):
            {
                SMFireMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID    = UID();
                smFM.MapID  = MapID();
                smFM.Magic  = nMagicID;
                smFM.Speed  = MagicSpeed();
                smFM.X      = nX;
                smFM.Y      = nY;
                smFM.AimUID = cmA.AimUID;

                Delay(800, [this, smFM]()
                {
                    g_netDriver->Post(ChannID(), SM_FIREMAGIC, smFM);
                });
                break;
            }
        case DBCOM_MAGICID(u8"雷电术"):
            {
                SMFireMagic stSMFM;
                std::memset(&stSMFM, 0, sizeof(stSMFM));

                stSMFM.UID    = UID();
                stSMFM.MapID  = MapID();
                stSMFM.Magic  = nMagicID;
                stSMFM.Speed  = MagicSpeed();
                stSMFM.X      = nX;
                stSMFM.Y      = nY;
                stSMFM.AimUID = cmA.AimUID;

                Delay(1400, [this, stSMFM]()
                {
                    g_netDriver->Post(ChannID(), SM_FIREMAGIC, stSMFM);
                });
                break;
            }
        case DBCOM_MAGICID(u8"魔法盾"):
            {
                SMFireMagic stSMFM;
                std::memset(&stSMFM, 0, sizeof(stSMFM));

                stSMFM.UID   = UID();
                stSMFM.Magic = nMagicID;
                stSMFM.Speed = MagicSpeed();

                Delay(800, [this, stSMFM]()
                {
                    g_netDriver->Post(ChannID(), SM_FIREMAGIC, stSMFM);
                });
                break;
            }
        case DBCOM_MAGICID(u8"召唤骷髅"):
            {
                int nFrontX = -1;
                int nFrontY = -1;
                PathFind::GetFrontLocation(&nFrontX, &nFrontY, X(), Y(), Direction(), 2);

                SMFireMagic stSMFM;
                std::memset(&stSMFM, 0, sizeof(stSMFM));

                stSMFM.UID   = UID();
                stSMFM.MapID = MapID();
                stSMFM.Magic = nMagicID;
                stSMFM.Speed = MagicSpeed();
                stSMFM.AimX  = nFrontX;
                stSMFM.AimY  = nFrontY;

                Delay(600, [this, stSMFM]()
                {
                    addMonster(DBCOM_MONSTERID(u8"变异骷髅"), stSMFM.AimX, stSMFM.AimY, false);

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

                SMFireMagic smFM;
                std::memset(&smFM, 0, sizeof(smFM));

                smFM.UID   = UID();
                smFM.MapID = MapID();
                smFM.Magic = nMagicID;
                smFM.Speed = MagicSpeed();
                smFM.AimX  = nFrontX;
                smFM.AimY  = nFrontY;

                Delay(1000, [this, smFM]()
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
    switch(estimateHop(stCMA.X, stCMA.Y)){
        case 0:
            {
                AMPickUp stAMPU;
                stAMPU.X    = stCMA.X;
                stAMPU.Y    = stCMA.Y;
                stAMPU.UID  = UID();
                stAMPU.ID   = stCMA.ActionParam;
                stAMPU.DBID = 0;

                m_actorPod->forward(m_map->UID(), {MPK_PICKUP, stAMPU});
                return;
            }
        case 1:
            {
                requestMove(stCMA.X, stCMA.Y, SYS_MAXSPEED, false, false,
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

        AMPullCOInfo stAMPCOI;
        std::memset(&stAMPCOI, 0, sizeof(stAMPCOI));

        stAMPCOI.X     = X();
        stAMPCOI.Y     = Y();
        stAMPCOI.W     = nW;
        stAMPCOI.H     = nH;
        stAMPCOI.UID   = UID();
        stAMPCOI.MapID = m_map->ID();
        m_actorPod->forward(m_map->UID(), {MPK_PULLCOINFO, stAMPCOI});
    }
}

bool Player::CanPickUp(uint32_t, uint32_t)
{
    return true;
}

bool Player::DBUpdate(const char *szTableName, const char *szFieldList, ...)
{
    if(false
            || (!szTableName || !std::strlen(szTableName))
            || (!szFieldList || !std::strlen(szFieldList))){
        return false;
    }

    std::string szSQLCommand;
    std::string szExceptionStr;
    {
        va_list ap;
        va_start(ap, szFieldList);

        try{
            szSQLCommand = str_vprintf(szFieldList, ap);
        }catch(const std::exception &e){
            szExceptionStr = str_printf("Exception caught in Player::DBUpdate(%s, \"%s\"): %s", szTableName, szFieldList, e.what());
        }

        va_end(ap);
    }

    if(!szExceptionStr.empty()){
        g_monoServer->addLog(LOGTYPE_WARNING, "%s", szExceptionStr.c_str());
        return false;
    }

    g_DBPod->CreateDBHDR()->QueryResult("update %s set %s where fld_dbid = %" PRIu32, szTableName, szSQLCommand.c_str(), DBID());
    return true;
}

bool Player::DBAccess(const char *szTableName, const char *szFieldName, std::function<std::u8string(const char8_t *)> fnDBOperation)
{
    if(true
            && (szTableName && std::strlen(szTableName))
            && (szFieldName && std::strlen(szFieldName))){

        auto pDBHDR = g_DBPod->CreateDBHDR();
        if(!pDBHDR->QueryResult("select %s from %s where fld_dbid = %" PRIu32, szFieldName, szTableName, DBID())){
            g_monoServer->addLog(LOGTYPE_INFO, "No dbid created for this player: DBID = %" PRIu32, DBID());
            return false;
        }

        auto szRes = fnDBOperation(pDBHDR->Get<std::u8string>(szFieldName).c_str());

        // if need to return a string we should do:
        //     return "\"xxxx\"";
        // then empty string should be "\"\"", not szRes.empty()

        if(!szRes.empty()){
            pDBHDR->QueryResult("update %s set %s = %s where fld_dbid = %" PRIu32, szTableName, szFieldName, szRes.c_str(), DBID());
            return true;
        }
    }
    return false;
}

bool Player::DBLoadPlayer()
{
    return true;
}

bool Player::DBSavePlayer()
{
    return DBUpdate("tbl_dbid", "fld_gold = %d, fld_level = %d", Gold(), Level());
}

void Player::reportGold()
{
    SMGold stSMG;
    std::memset(&stSMG, 0, sizeof(stSMG));

    stSMG.Gold = Gold();
    postNetMessage(SM_GOLD, stSMG);
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
