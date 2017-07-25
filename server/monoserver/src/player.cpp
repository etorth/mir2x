/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 07/25/2017 11:03:02
 *
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
#include "netpod.hpp"
#include "player.hpp"
#include "threadpn.hpp"
#include "memorypn.hpp"
#include "sysconst.hpp"
#include "charobject.hpp"
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
    , m_SessionID(0)    // provide by bind
    , m_Level(0)        // after bind
{
    m_StateHook.Install("CheckTime", [this](){ For_CheckTime(); return false; });
    auto fnRegisterClass = [this]() -> void {
        if(!RegisterClass<Player, CharObject>()){
            extern MonoServer *g_MonoServer;
            g_MonoServer->AddLog(LOGTYPE_WARNING, "Class registration for <Player, CharObject> failed");
            g_MonoServer->Restart();
        }
    };
    static std::once_flag stFlag;
    std::call_once(stFlag, fnRegisterClass);

    m_HP    = 10;
    m_HPMax = 10;
    m_MP    = 10;
    m_MPMax = 10;
}

void Player::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
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
        case MPK_BINDSESSION:
            {
                On_MPK_BINDSESSION(rstMPK, rstFromAddr);
                break;
            }
        case MPK_NETPACKAGE:
            {
                On_MPK_NETPACKAGE(rstMPK, rstFromAddr);
                break;
            }
        case MPK_PULLCOINFO:
            {
                On_MPK_PULLCOINFO(rstMPK, rstFromAddr);
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
        case CM_QUERYCORECORD : Net_CM_QUERYCORECORD (nType, pData, nDataLen); break;
        case CM_ACTION        : Net_CM_ACTION        (nType, pData, nDataLen); break;
        default               :                                                break;
    }
}

void Player::For_CheckTime()
{
}

bool Player::Update()
{
    return true;
}

bool Player::Bind(uint32_t nSessionID)
{
    m_SessionID = nSessionID;

    extern NetPodN *g_NetPodN;
    g_NetPodN->Bind(m_SessionID, GetAddress());
    return true;
}

void Player::ReportCORecord(uint32_t nSessionID)
{
    if(nSessionID){
        SMCORecord stSMCOR;

        stSMCOR.Type = CREATURE_PLAYER;

        stSMCOR.Common.UID       = UID();
        stSMCOR.Common.MapID     = MapID();
        stSMCOR.Common.X         = X();
        stSMCOR.Common.Y         = Y();
        stSMCOR.Common.EndX      = X();
        stSMCOR.Common.EndY      = Y();
        stSMCOR.Common.Direction = Direction();
        stSMCOR.Common.Speed     = Speed();
        stSMCOR.Common.Action    = ACTION_STAND;

        stSMCOR.Player.DBID      = m_DBID;
        stSMCOR.Player.JobID     = m_JobID;
        stSMCOR.Player.Level     = m_Level;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(nSessionID, SM_CORECORD, stSMCOR);
    }else{
        extern MonoServer *g_MonoServer;
        g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid session id");
        g_MonoServer->Restart();
    }
}

void Player::ReportStand()
{
    if(m_SessionID){
        // any error found when checking motion
        // report an stand state to client for pull-back
        SMAction stSMAction;
        stSMAction.UID         = UID();
        stSMAction.MapID       = MapID();
        stSMAction.Action      = ACTION_STAND;
        stSMAction.ActionParam = 0;
        stSMAction.Speed       = SYS_DEFSPEED;
        stSMAction.Direction   = Direction();
        stSMAction.X           = X();
        stSMAction.Y           = Y();
        stSMAction.AimX        = X();
        stSMAction.AimY        = Y();

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(m_SessionID, SM_ACTION, stSMAction);
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

void Player::OnCMActionAttack(int nX0, int nY0, int nX1, int nY1, int nAttackParam, int nSpeed, int nDirection)
{
    if(true
            && m_Map
            && m_Map->ValidC(nX0, nY0)
            && m_Map->ValidC(nX1, nY1)
            
            && nSpeed >= SYS_MINSPEED
            && nSpeed <= SYS_MAXSPEED

            && nDirection > DIR_NONE
            && nDirection < DIR_MAX){

        // for client message action is for presenting
        // then all parameters should pass the checking here

        switch(nAttackParam){
            case DC_PHY_PLAIN:
            case DC_PHY_WIDESWORD:
            case DC_PHY_FIRE:
                {
                    switch(LDistance2(X(), Y(), nX1, nY1)){
                        case 1:
                        case 2:
                            {
                                DispatchAction({
                                        ACTION_ATTACK,
                                        nAttackParam,
                                        nSpeed,
                                        nDirection,
                                        X(),
                                        Y(),
                                        nX1,
                                        nY1,
                                        m_Map->ID()});

                                auto fnOnResp = [this](const MessagePack &rstRMPK, const Theron::Address &) -> void
                                {
                                    switch(rstRMPK.Type()){
                                        case MPK_UIDV:
                                            {
                                                AMUIDV stAMUIDV;
                                                std::memcpy(&stAMUIDV, rstRMPK.Data(), sizeof(stAMUIDV));

                                                for(size_t nIndex = 0; nIndex < sizeof(stAMUIDV.UIDV) / sizeof(stAMUIDV.UIDV[0]); ++nIndex){
                                                    if(auto nUID = stAMUIDV.UIDV[nIndex]){
                                                        DispatchAttack(nUID, DC_PHY_PLAIN);
                                                    }else{
                                                        break;
                                                    }
                                                }
                                            }
                                    }
                                };

                                AMQueryRectUIDV stAMQRUIDV;
                                stAMQRUIDV.MapID = m_Map->ID();
                                stAMQRUIDV.X     = nX1;
                                stAMQRUIDV.Y     = nY1;
                                stAMQRUIDV.W     = 1;
                                stAMQRUIDV.H     = 1;

                                m_ActorPod->Forward({MPK_QUERYRECTUIDV, stAMQRUIDV}, m_Map->GetAddress(), fnOnResp);
                                break;
                            }
                        default:
                            {
                                // ReportStand();
                                // return;
                            }
                    }

                }
            default:
                {
                    break;
                }
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
    if(rstDamage){
        m_HP = std::max<int>(0, m_HP - rstDamage.Damage);
        if(m_HP == 0){
            SetState(STATE_DEAD, STATE_DEAD);
        }
        return true;
    }
    return false;
}
