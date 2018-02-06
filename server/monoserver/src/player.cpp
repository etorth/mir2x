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
#include "dbcomid.hpp"
#include "threadpn.hpp"
#include "memorypn.hpp"
#include "sysconst.hpp"
#include "netdriver.hpp"
#include "charobject.hpp"
#include "friendtype.hpp"
#include "protocoldef.hpp"

Player::Player(uint32_t nDBID,
        ServiceCore    *pServiceCore,
        ServerMap      *pServerMap,
        int             nMapX,
        int             nMapY,
        int             nDirection,
        uint8_t         nLifeState)
    : CharObject(pServiceCore, pServerMap, nMapX, nMapY, nDirection, nLifeState)
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

void Player::OperateAM(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NOTIFYNEWCO:
            {
                On_MPK_NOTIFYNEWCO(rstMPK, rstFromAddr);
                break;
            }
        case MPK_MAPSWITCH:
            {
                On_MPK_MAPSWITCH(rstMPK, rstFromAddr);
                break;
            }
        case MPK_QUERYLOCATION:
            {
                On_MPK_QUERYLOCATION(rstMPK, rstFromAddr);
                break;
            }
        case MPK_EXP:
            {
                On_MPK_EXP(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK, rstFromAddr);
                break;
            }
        case MPK_ATTACK:
            {
                On_MPK_ATTACK(rstMPK, rstFromAddr);
                break;
            }
        case MPK_UPDATEHP:
            {
                On_MPK_UPDATEHP(rstMPK, rstFromAddr);
                break;
            }
        case MPK_DEADFADEOUT:
            {
                On_MPK_DEADFADEOUT(rstMPK, rstFromAddr);
                break;
            }
        case MPK_SHOWDROPITEM:
            {
                On_MPK_SHOWDROPITEM(rstMPK, rstFromAddr);
                break;
            }
        case MPK_BINDCHANNEL:
            {
                On_MPK_BINDCHANNEL(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NETPACKAGE:
            {
                On_MPK_NETPACKAGE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_QUERYCORECORD:
            {
                On_MPK_QUERYCORECORD(rstMPK, rstFromAddr);
                break;
            }
        case MPK_BADCHANNEL:
            {
                On_MPK_BADCHANNEL(rstMPK, rstFromAddr);
                break;
            }
        case MPK_OFFLINE:
            {
                On_MPK_OFFLINE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_REMOVEGROUNDITEM:
            {
                On_MPK_REMOVEGROUNDITEM(rstMPK, rstFromAddr);
                break;
            }
        case MPK_PICKUPOK:
            {
                On_MPK_PICKUPOK(rstMPK, rstFromAddr);
                break;
            }
        case MPK_CORECORD:
            {
                On_MPK_CORECORD(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NOTIFYDEAD:
            {
                On_MPK_NOTIFYDEAD(rstMPK, rstFromAddr);
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

void Player::ReportCORecord(uint32_t nUID)
{
    if(nUID){
        extern MonoServer *g_MonoServer;
        if(auto stUIDRecord = g_MonoServer->GetUIDRecord(nUID)){
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
            m_ActorPod->Forward({MPK_CORECORD, stAMCOR}, stUIDRecord.GetAddress());
        }
    }
}

void Player::ReportStand()
{
    ReportAction(UID(), ActionStand(X(), Y(), Direction()));
}

void Player::ReportAction(uint32_t nUID, const ActionNode &rstAction)
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

void Player::ReportHealth()
{
    if(ChannID()){
        SMUpdateHP stSMUHP;
        stSMUHP.UID   = UID();
        stSMUHP.MapID = MapID();
        stSMUHP.HP    = HP();
        stSMUHP.HPMax = HPMax();

        extern NetDriver *g_NetDriver;
        g_NetDriver->Post(ChannID(), SM_UPDATEHP, stSMUHP);
    }
}

bool Player::InRange(int, int, int)
{
    return true;
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
                            stAMDFO.UID   = UID();
                            stAMDFO.MapID = MapID();
                            stAMDFO.X     = X();
                            stAMDFO.Y     = Y();

                            if(true
                                    && ActorPodValid()
                                    && m_Map
                                    && m_Map->ActorPodValid()){
                                m_ActorPod->Forward({MPK_DEADFADEOUT, stAMDFO}, m_Map->GetAddress());
                            }

                            // 2. deactivate the actor here
                            //    disable the actorpod then no source can drive it
                            //    then current *this* can't be refered by any actor threads after this invocation
                            //    then MonoServer::EraseUID() is safe to delete *this*
                            //
                            //    don't do delete m_ActorPod to disable the actor
                            //    since currently we are in the actor thread which accquired by m_ActorPod
                            Deactivate();

                            // 3. without message driving it
                            //    the char object will be inactive and activities after this
                            GoSuicide();
                            return true;

                            // there is an time gap after Deactivate() and before deletion handler called in GoSuicide
                            // then during this gap even if the actor is scheduled we won't have data race anymore
                            // since we called Deactivate() which deregistered Innhandler refers *this*
                            //
                            // note that even if during this gap we have functions call GetAddress()
                            // we are still OK since m_ActorPod is still valid
                            // but if then send to this address, it will drain to the default message handler
                        }
                }
            }
        default:
            {
                return false;
            }
    }
}

bool Player::GoSuicide()
{
    if(true
            && GetState(STATE_DEAD)
            && GetState(STATE_GHOST)){

        // 1. register a operationi to the thread pool to delete
        // 2. don't pass *this* to any other threads, pass UID instead
        extern ThreadPN *g_ThreadPN;
        return g_ThreadPN->Add([nUID = UID()](){
            if(nUID){
                extern MonoServer *g_MonoServer;
                g_MonoServer->EraseUID(nUID);
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "Suicide with empty UID");
            }
        });

        // after this line
        // *this* is invalid and should never be refered
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "GoSuicide(this = %p, UID = %" PRIu32 ") failed", this, UID());
    return false;
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
    if(rstDamage){
        m_HP = std::max<int>(0, HP() - rstDamage.Damage);
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

void Player::CheckFriend(uint32_t nUID, const std::function<void(int)> &fnOnFriend)
{
    if(nUID){
        if(0){
            if(fnOnFriend){ fnOnFriend(FRIENDTYPE_NONE); }
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

        stAMO.UID   = UID();
        stAMO.MapID = MapID();
        stAMO.X     = X();
        stAMO.Y     = Y();

        m_ActorPod->Forward({MPK_OFFLINE, stAMO}, m_Map->GetAddress());
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "Can't dispatch offline event");
}

void Player::ReportOffline(uint32_t nUID, uint32_t nMapID)
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

    // 1. register a operationi to the thread pool to delete
    // 2. don't pass *this* to any other threads, pass UID instead
    extern ThreadPN *g_ThreadPN;
    return g_ThreadPN->Add([nUID = UID()](){
        if(nUID){
            extern MonoServer *g_MonoServer;
            g_MonoServer->EraseUID(nUID);
        }
        else{
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Offline with empty UID");
        }
    });
}

InvarData Player::GetInvarData() const
{
    InvarData stData;
    stData.Player.DBID = DBID();
    return stData;
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
        uint32_t nAimUID = stCMA.AimUID;

        if(rstLocation.MapID == MapID()){
            switch(nDCType){
                case DC_PHY_PLAIN:
                case DC_PHY_WIDESWORD:
                case DC_PHY_FIRESWORD:
                    {
                        switch(EstimateHop(nX0, nY0)){
                            case 0:
                                {
                                    switch(LDistance2(nX0, nY0, rstLocation.X, rstLocation.Y)){
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

                if(!PathFind::GetFrontLocation(&nFrontX, &nFrontY, X(), Y(), Direction(), 2)){
                    nFrontX = X() + 1;
                    nFrontY = Y() + 1;
                }

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

                    extern NetDriver *g_NetDriver;
                    g_NetDriver->Post(ChannID(), SM_FIREMAGIC, stSMFM);
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

                m_ActorPod->Forward({MPK_PICKUP, stAMPU}, m_Map->GetAddress());
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

int Player::MaxStep()
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

            auto nAdd = std::max<int>(nMax / 60, 1);
            return std::min<int>(nAdd, nMax - nCurr);
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
    // pull all co's on current map
    // in rectangle center on (X(), Y()) and (nW, nH)

    if(true
            && nW > 0
            && nH > 0
            && ActorPodValid()
            && m_Map->ActorPodValid()){

        AMPullCOInfo stAMPCOI;
        stAMPCOI.X     = X();
        stAMPCOI.Y     = Y();
        stAMPCOI.W     = nW;
        stAMPCOI.H     = nH;
        stAMPCOI.UID   = UID();
        stAMPCOI.MapID = m_Map->ID();
        m_ActorPod->Forward({MPK_PULLCOINFO, stAMPCOI}, m_Map->GetAddress());
    }
}

bool Player::CanPickUp(uint32_t, uint32_t)
{
    return true;
}

bool Player::DBUpdate(const char *szTableName, const char *szFieldList, ...)
{
    if(true
            && (szTableName && std::strlen(szTableName))
            && (szFieldList && std::strlen(szFieldList))){

        auto fnWriteDB = [this](const char *szTableName, const char *szSQLCommand) -> bool
        {
            extern DBPodN *g_DBPodN;
            auto pDBHDR = g_DBPodN->CreateDBHDR();

            if(!pDBHDR->Execute("update mir2x.%s set %s where fld_dbid = %" PRIu32, szTableName, szSQLCommand, DBID())){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
                return false;
            }
            return true;
        };

        int nCmdLen = 0;

        // 1. try static buffer
        //    give an enough size so we can hopefully stop here
        {
            char szSBuf[1024];

            va_list ap;
            va_start(ap, szFieldList);
            nCmdLen = std::vsnprintf(szSBuf, std::extent<decltype(szSBuf)>::value, szFieldList, ap);
            va_end(ap);

            if(nCmdLen >= 0){
                if((size_t)(nCmdLen + 1) < std::extent<decltype(szSBuf)>::value){
                    return fnWriteDB(szTableName, szSBuf);
                }else{
                    // do nothing
                    // have to try the dynamic buffer method
                }
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL table %s update command parsing failed: %s", szTableName, szFieldList);
                return false;
            }
        }

        // 2. try dynamic buffer
        //    use the parsed buffer size above to get enough memory
        while(true){
            std::vector<char> szDBuf(nCmdLen + 1 + 64);

            va_list ap;
            va_start(ap, szFieldList);
            nCmdLen = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szFieldList, ap);
            va_end(ap);

            if(nCmdLen >= 0){
                if((size_t)(nCmdLen + 1) < szDBuf.size()){
                    return fnWriteDB(szTableName, &(szDBuf[0]));
                }else{
                    szDBuf.resize(nCmdLen + 1 + 64);
                }
            }else{
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL table %s update command parsing failed: %s", szTableName, szFieldList);
                return false;
            }
        }
    }
    return false;
}

bool Player::DBAccess(const char *szTableName, const char *szFieldName, std::function<std::string(const char *)> fnDBOperation)
{
    if(true
            && (szTableName && std::strlen(szTableName))
            && (szFieldName && std::strlen(szFieldName))){

        extern DBPodN *g_DBPodN;
        auto pDBHDR = g_DBPodN->CreateDBHDR();

        if(!pDBHDR->Execute("select %s from mir2x.%s where fld_dbid = %" PRIu32, szFieldName, szTableName, DBID())){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
            return false;
        }

        if(pDBHDR->RowCount() < 1){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_INFO, "No dbid created for this player: DBID = %" PRIu32, DBID());
            return false;
        }

        pDBHDR->Fetch();
        auto szRes = fnDBOperation(pDBHDR->Get(szFieldName));

        // if need to return a string we should do:
        //     return "\"xxxx\"";
        // then empty string should be "\"\"", not szRes.empty()

        if(!szRes.empty()){
            if(!pDBHDR->Execute("update mir2x.%s set %s = %s where fld_dbid = %" PRIu32, szTableName, szFieldName, szRes.c_str(), DBID())){
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "SQL ERROR: (%d: %s)", pDBHDR->ErrorID(), pDBHDR->ErrorInfo());
                return false;
            }
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
