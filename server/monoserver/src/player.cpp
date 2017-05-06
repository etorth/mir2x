/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/05/2017 18:09:45
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

#include "netpod.hpp"
#include "player.hpp"
#include "memorypn.hpp"
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
        case MPK_ACTION:
            {
                On_MPK_ACTION(rstMPK, rstFromAddr);
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
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: %s", rstMPK.Name());
                g_MonoServer->Restart();
                break;
            }
    }
}

bool Player::Update()
{
    return true;
}

uint8_t Player::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Player::ResetState(uint8_t nState, uint8_t nThisState)
{
    m_StateV[nState] = nThisState;
    return true;
}

uint32_t Player::NameColor()
{
    return 0XFFFFFFFF;
}

const char *Player::CharName()
{
    return "hello";
}

int Player::Range(uint8_t)
{
    return 20;
}

void Player::OperateNet(uint8_t nType, const uint8_t *pData, size_t nDataLen)
{
    switch(nType){
        case CM_QUERYMONSTERGINFO: Net_CM_QUERYMONSTERGINFO(nType, pData, nDataLen); break;
        case CM_ACTION           : Net_CM_ACTION           (nType, pData, nDataLen); break;
        default                  :                                                   break;
    }
}

void Player::For_CheckTime()
{
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
    // any error found when checking motion
    // report an stand state to client for pull-back
    SMAction stSMAction;
    stSMAction.UID         = UID();
    stSMAction.MapID       = MapID();
    stSMAction.Action      = ACTION_STAND;
    stSMAction.ActionParam = 0;
    stSMAction.Speed       = 0;
    stSMAction.Direction   = Direction();
    stSMAction.X           = X();
    stSMAction.Y           = Y();
    stSMAction.EndX        = X();
    stSMAction.EndY        = Y();

    extern NetPodN *g_NetPodN;
    g_NetPodN->Send(m_SessionID, SM_ACTION, stSMAction);
}
