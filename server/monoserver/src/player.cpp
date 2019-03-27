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
#include "uidfunc.hpp"
#include "dbcomid.hpp"
#include "threadpn.hpp"
#include "memorypn.hpp"
#include "sysconst.hpp"
#include "netdriver.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"

extern DBPodN *g_DBPodN;
extern MonoServer *g_MonoServer;

Player::Player(uint32_t nDBID,
        ServiceCore    *pServiceCore,
        ServerMap      *pServerMap,
        int             nMapX,
        int             nMapY,
        int             nDirection)
    : CharObject(pServiceCore, pServerMap, UIDFunc::GetPlayerUID(nDBID), nMapX, nMapY, nDirection)
    , m_DBID(nDBID)
    , m_JobID(0)        // will provide after bind
    , m_ChannID(0)    // provide by bind
    , m_Exp(0)
    , m_Level(0)        // after bind
    , m_Gold(0)
    , m_Inventory()
{
    m_StateHook.Install("CheckTime", [this]() -> bool
    {
        For_CheckTime();
        return false;
    });

    m_HP    = 10;
    m_HPMax = 10;
    m_MP    = 10;
    m_MPMax = 10;

    m_StateHook.Install("RecoverHealth", [this, nLastTime = (uint32_t)(0)]() mutable -> bool
    {
        extern MonoServer *g_MonoServer;
        if(g_MonoServer->GetTimeTick() >= (nLastTime + 1000)){
            RecoverHealth();
            nLastTime = g_MonoServer->GetTimeTick();
        }
        return false;
    });
}

Player::~Player()
{
    DBSavePlayer();
}

void Player::OperateAM(const MessagePack &rstMPK)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK);
                break;
            }
        case MPK_BADACTORPOD:
            {
                On_MPK_BADACTORPOD(rstMPK);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                On_MPK_NOTIFYNEWCO(rstMPK);
                break;
            }
        case MPK_CHECKMASTER:
            {
                On_MPK_CHECKMASTER(rstMPK);
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
        case MPK_QUERYFRIENDTYPE:
            {
                On_MPK_QUERYFRIENDTYPE(rstMPK);
                break;
            }
        case MPK_EXP:
            {
                On_MPK_EXP(rstMPK);
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
        case MPK_UPDATEHP:
            {
                On_MPK_UPDATEHP(rstMPK);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                On_MPK_DEADFADEOUT(rstMPK);
                break;
            }
        case MPK_SHOWDROPITEM:
            {
                On_MPK_SHOWDROPITEM(rstMPK);
                break;
            }
        case MPK_BINDCHANNEL:
            {
                On_MPK_BINDCHANNEL(rstMPK);
                break;
            }
        case MPK_NETPACKAGE:
            {
                On_MPK_NETPACKAGE(rstMPK);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                On_MPK_QUERYCORECORD(rstMPK);
                break;
            }
        case MPK_BADCHANNEL:
            {
                On_MPK_BADCHANNEL(rstMPK);
                break;
            }
        case MPK_OFFLINE:
            {
                On_MPK_OFFLINE(rstMPK);
                break;
            }
        case MPK_REMOVEGROUNDITEM:
            {
                On_MPK_REMOVEGROUNDITEM(rstMPK);
                break;
            }
        case MPK_PICKUPOK:
            {
                On_MPK_PICKUPOK(rstMPK);
                break;
            }
        case MPK_CORECORD:
            {
                On_MPK_CORECORD(rstMPK);
                break;
            }
        case MPK_NOTIFYDEAD:
            {
                On_MPK_NOTIFYDEAD(rstMPK);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Unsupported message: %s", rstMPK.Name());
                break;
            }
    }
}

void Player::OperateNet(uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
        case CM_QUERYCORECORD   : Net_CM_QUERYCORECORD   (nType, pData, nDataLen); break;
        case CM_REQUESTSPACEMOVE: Net_CM_REQUESTSPACEMOVE(nType, pData, nDataLen); break;
        case CM_ACTION          : Net_CM_ACTION          (nType, pData, nDataLen); break;
        case CM_PICKUP          : Net_CM_PICKUP          (nType, pData, nDataLen); break;
        case CM_QUERYGOLD       : Net_CM_QUERYGOLD       (nType, pData, nDataLen); break;
        default                 :                                                  break;
    }
}

void Player::For_CheckTime()
{
}

bool Player::Update()
{
    return true;
}

void Player::ReportCORecord(uint64_t nUID)
{
    if(!nUID){
        return;
    }

    AMCORecord stAMCOR;
    std::memset(&stAMCOR, 0, sizeof(stAMCOR));

    // TODO: don't use OBJECT_PLAYER, we need translation
    //       rule of communication, the sender is responsible to translate

    // 1. set type
    stAMCOR.COType = CREATURE_PLAYER;

    // 2. set current action
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

    // 3. set specified co information
    stAMCOR.Player.DBID  = DBID();
    stAMCOR.Player.JobID = JobID();
    stAMCOR.Player.Level = Level();

    // don't reply to server map
    // even get co information pull request from map
    m_ActorPod->Forward(nUID, {MPK_CORECORD, stAMCOR});
}

void Player::ReportStand()
{
    ReportAction(UID(), ActionStand(X(), Y(), Direction()));
}

void Player::ReportAction(uint64_t nUID, const ActionNode &rstAction)
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

        extern NetDriver *g_NetDriver;
        g_NetDriver->Post(ChannID(), SM_ACTION, stSMA);
    }
}

void Player::ReportDeadUID(uint64_t nDeadUID)
{
    SMNotifyDead stSMND;
    std::memset(&stSMND, 0, sizeof(stSMND));

    stSMND.UID = nDeadUID;
    PostNetMessage(SM_NOTIFYDEAD, stSMND);
}

void Player::ReportHealth()
{
    SMUpdateHP stSMUHP;
    std::memset(&stSMUHP, 0, sizeof(stSMUHP));

    stSMUHP.UID   = UID();
    stSMUHP.MapID = MapID();
    stSMUHP.HP    = HP();
    stSMUHP.HPMax = HPMax();

    PostNetMessage(SM_UPDATEHP, stSMUHP);
}

bool Player::InRange(int nRangeType, int nX, int nY)
{
    if(!m_Map->ValidC(nX, nY)){
        return false;
    }

    switch(nRangeType){
        case RANGE_VISIBLE:
            {
                return MathFunc::LDistance2(X(), Y(), nX, nY) < 20 * 20;
            }
        case RANGE_ATTACK:
            {
                return MathFunc::LDistance2(X(), Y(), nX, nY) < 10 * 10;
            }
        default:
            {
                break;
            }
    }
    return false;
}

bool Player::GoDie()
{
    switch(GetState(STATE_NEVERDIE)){
        case 0:
            {
                switch(GetState(STATE_DEAD)){
                    case 0:
                        {
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

bool Player::GoGhost()
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
                                    && m_Map
                                    && m_Map->ActorPodValid()){
                                m_ActorPod->Forward(m_Map->UID(), {MPK_DEADFADEOUT, stAMDFO});
                            }

                            // 2. deactivate the actor here
                            //    disable the actorpod then no source can drive it
                            //    then current *this* can't be refered by any actor threads after this invocation
                            //    then MonoServer::EraseUID() is safe to delete *this*
                            //
                            //    don't do delete m_ActorPod to disable the actor
                            //    since currently we are in the actor thread which accquired by m_ActorPod
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
        ReportHealth();
        DispatchHealth();

        if(HP() <= 0){
            GoDie();
        }
        return true;
    }
    return false;
}

bool Player::ActionValid(const ActionNode &)
{
    return true;
}

void Player::CheckFriend(uint64_t nUID, const std::function<void(int)> &fnOnFriend)
{
    if(nUID){
        if(0){
            if(fnOnFriend){ fnOnFriend(FT_NONE); }
        }
    }
}

void Player::DispatchOffline()
{
    if(true
            && ActorPodValid()
            && m_Map
            && m_Map->ActorPodValid()){

        AMOffline stAMO;
        std::memset(&stAMO, 0, sizeof(stAMO));

        stAMO.UID   = UID();
        stAMO.MapID = MapID();
        stAMO.X     = X();
        stAMO.Y     = Y();

        m_ActorPod->Forward(m_Map->UID(), {MPK_OFFLINE, stAMO});
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't dispatch offline event");
}

void Player::ReportOffline(uint64_t nUID, uint32_t nMapID)
{
    if(true
            && nUID
            && nMapID
            && ChannID()){

        SMOffline stSMO;
        stSMO.UID   = nUID;
        stSMO.MapID = nMapID;

        extern NetDriver *g_NetDriver;
        g_NetDriver->Post(ChannID(), SM_OFFLINE, stSMO);
    }
}

bool Player::Offline()
{
    DispatchOffline();
    ReportOffline(UID(), MapID());

    Deactivate();
    return true;
}

bool Player::PostNetMessage(uint8_t nHC, const uint8_t *pData, size_t nDataLen)
{
    if(ChannID()){
        extern NetDriver *g_NetDriver;
        return g_NetDriver->Post(ChannID(), nHC, pData, nDataLen);
    }
    return false;
}

void Player::OnCMActionStand(CMAction stCMA)
{
    int nX = stCMA.X;
    int nY = stCMA.Y;
    int nDirection = stCMA.Direction;

    if(true
            && m_Map
            && m_Map->ValidC(nX, nY)){

        // server get report stand
        // means client is trying to re-sync
        // try client's current location and always response

        switch(EstimateHop(nX, nY)){
            case 1:
                {
                    RequestMove(nX, nY, SYS_MAXSPEED, false,
                    [this, stCMA]()
                    {
                        OnCMActionStand(stCMA);
                    },
                    [this]()
                    {
                        ReportStand();
                    });
                    return;
                }
            case 0:
            default:
                {
                    if(true
                            && nDirection > DIR_NONE
                            && nDirection < DIR_MAX){
                        m_Direction = nDirection;
                    }

                    ReportStand();
                    return;
                }
        }
    }
}

void Player::OnCMActionMove(CMAction stCMA)
{
    // server won't do any path finding
    // client should sent action with only one-hop movement

    int nX0 = stCMA.X;
    int nY0 = stCMA.Y;
    int nX1 = stCMA.AimX;
    int nY1 = stCMA.AimY;

    switch(EstimateHop(nX0, nY0)){
        case 0:
            {
                RequestMove(nX1, nY1, MoveSpeed(), false, [](){}, [this]()
                {
                    ReportStand();
                });
                return;
            }
        case 1:
            {
                RequestMove(nX0, nY0, SYS_MAXSPEED, false, [this, stCMA]()
                {
                    OnCMActionMove(stCMA);
                },
                [this]()
                {
                    ReportStand();
                });
                return;
            }
        default:
            {
                ReportStand();
                return;
            }
    }
}

void Player::OnCMActionAttack(CMAction stCMA)
{
    RetrieveLocation(stCMA.AimUID, [this, stCMA](const COLocation &rstLocation)
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
                        switch(EstimateHop(nX0, nY0)){
                            case 0:
                                {
                                    switch(MathFunc::LDistance2(nX0, nY0, rstLocation.X, rstLocation.Y)){
                                        case 1:
                                        case 2:
                                            {
                                                DispatchAttack(nAimUID, nDCType);
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
                                    RequestMove(nX0, nY0, SYS_MAXSPEED, false,
                                    [this, stCMA]()
                                    {
                                        OnCMActionAttack(stCMA);
                                    },
                                    [this]()
                                    {
                                        ReportStand();
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

void Player::OnCMActionSpell(CMAction stCMA)
{
    int nX = stCMA.X;
    int nY = stCMA.Y;
    int nMagicID = stCMA.ActionParam;

    switch(nMagicID){
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
                stSMFM.AimUID = stCMA.AimUID;

                Delay(1400, [this, stSMFM]()
                {
                    extern NetDriver *g_NetDriver;
                    g_NetDriver->Post(ChannID(), SM_FIREMAGIC, stSMFM);
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
                    extern NetDriver *g_NetDriver;
                    g_NetDriver->Post(ChannID(), SM_FIREMAGIC, stSMFM);
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
                    AddMonster(DBCOM_MONSTERID(u8"变异骷髅"), stSMFM.AimX, stSMFM.AimY, true);

                    // AddMonster will send ACTION_SPAWN to client
                    // client then use it to play the magic for 召唤骷髅, we don't send magic message here
                });
                break;
            }
        default:
            {
                break;
            }
    }

    // sync the location
    // for spelling magic location is not critical
    RequestMove(nX, nY, SYS_MAXSPEED, false, [](){}, [this]()
    {
        ReportStand();
    });
}

void Player::OnCMActionPickUp(CMAction stCMA)
{
    switch(EstimateHop(stCMA.X, stCMA.Y)){
        case 0:
            {
                AMPickUp stAMPU;
                stAMPU.X    = stCMA.X;
                stAMPU.Y    = stCMA.Y;
                stAMPU.UID  = UID();
                stAMPU.ID   = stCMA.ActionParam;
                stAMPU.DBID = 0;

                m_ActorPod->Forward(m_Map->UID(), {MPK_PICKUP, stAMPU});
                return;
            }
        case 1:
            {
                RequestMove(stCMA.X, stCMA.Y, SYS_MAXSPEED, false,
                [this, stCMA]()
                {
                    OnCMActionPickUp(stCMA);
                },
                [this]()
                {
                    ReportStand();
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

        ReportHealth();
    }
}

void Player::GainExp(int nExp)
{
    if(nExp){
        if((int)(m_Exp) + nExp < 0){
            m_Exp = 0;
        }else{
            m_Exp += (uint32_t)(nExp);
        }

        auto nLevelExp = GetLevelExp();
        if(m_Exp >= nLevelExp){
            m_Exp    = m_Exp - nLevelExp;
            m_Level += 1;
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
            && ActorPodValid()
            && m_Map->ActorPodValid()){

        AMPullCOInfo stAMPCOI;
        std::memset(&stAMPCOI, 0, sizeof(stAMPCOI));

        stAMPCOI.X     = X();
        stAMPCOI.Y     = Y();
        stAMPCOI.W     = nW;
        stAMPCOI.H     = nH;
        stAMPCOI.UID   = UID();
        stAMPCOI.MapID = m_Map->ID();
        m_ActorPod->Forward(m_Map->UID(), {MPK_PULLCOINFO, stAMPCOI});
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
            szExceptionStr = str_printf("Exception caught in Player::Update(%s, \"%s\"): %s", szTableName, szFieldList, e.what());
        }

        va_end(ap);
    }

    if(!szExceptionStr.empty()){
        g_MonoServer->AddLog(LOGTYPE_WARNING, "%s", szExceptionStr.c_str());
        return false;
    }

    g_DBPodN->CreateDBHDR()->QueryResult("update %s set %s where fld_dbid = %" PRIu32, szTableName, szSQLCommand.c_str(), DBID());
    return true;
}

bool Player::DBAccess(const char *szTableName, const char *szFieldName, std::function<std::string(const char *)> fnDBOperation)
{
    if(true
            && (szTableName && std::strlen(szTableName))
            && (szFieldName && std::strlen(szFieldName))){

        auto pDBHDR = g_DBPodN->CreateDBHDR();
        if(!pDBHDR->QueryResult("select %s from %s where fld_dbid = %" PRIu32, szFieldName, szTableName, DBID())){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_INFO, "No dbid created for this player: DBID = %" PRIu32, DBID());
            return false;
        }

        auto szRes = fnDBOperation(pDBHDR->Get<std::string>(szFieldName).c_str());

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

void Player::ReportGold()
{
    SMGold stSMG;
    std::memset(&stSMG, 0, sizeof(stSMG));

    stSMG.Gold = Gold();
    PostNetMessage(SM_GOLD, stSMG);
}

void Player::CheckFriendType(uint64_t nUID, std::function<void(int)> fnOp)
{
    if(!nUID){
        throw std::invalid_argument(str_fflprintf(": Invalid zero UID"));
    }

    switch(UIDFunc::GetUIDType(nUID)){
        case UID_PLY:
            {
                fnOp(IsOffender(nUID) ? FT_ENEMY : FT_NEUTRAL);
                return;
            }
        case UID_MON:
            {
                if(!DBCOM_MONSTERRECORD(UIDFunc::GetMonsterID(nUID)).Tamable){
                    fnOp(FT_ENEMY);
                    return;
                }

                QueryFinalMaster(nUID, [this, nUID, fnOp](uint64_t nFMasterUID)
                {
                    switch(UIDFunc::GetUIDType(nFMasterUID)){
                        case UID_PLY:
                            {
                                fnOp(IsOffender(nUID) ? FT_ENEMY : FT_NEUTRAL);
                                return;
                            }
                        case UID_MON:
                            {
                                fnOp(FT_ENEMY);
                                return;
                            }
                        default:
                            {
                                throw std::runtime_error(str_fflprintf(": Final master is not PLY nor MON"));
                            }
                    }
                });
                return;
            }
        default:
            {
                throw std::invalid_argument(str_fflprintf(": Checking friend type for: %s", UIDFunc::GetUIDTypeString(nUID)));
            }
    }
}
