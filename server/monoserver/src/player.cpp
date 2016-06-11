/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 06/11/2016 03:26:17
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

Player::Player(uint32_t nGUID, uint32_t nJobID)
    : CharObject()
    , m_GUID(nGUID)
    , m_JobID(nJobID)
    , m_SessionID(0)
    , m_Level(0)
{
    m_RMAddress = Theron::Address::Null();
    m_StateHook.Install("CheckTime", [this](){ For_CheckTime(); return false; });

    ResetType(OBJECT_PLAYER, true);
    ResetType(OBJECT_HUMAN,  true);
}

Player::~Player()
{
}

void Player::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
        case MPK_HI:
            {
                On_MPK_HI(rstMPK, rstFromAddr);
                break;
            }
        case MPK_METRONOME:
            {
                On_MPK_METRONOME(rstMPK, rstFromAddr);
                break;
            }
        case MPK_BINDSESSION:
            {
                On_MPK_BINDSESSION(rstMPK, rstFromAddr);
                break;
            }
        case MPK_UPDATECOINFO:
            {
                On_MPK_UPDATECOINFO(rstMPK, rstFromAddr);
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

bool Player::Type(uint8_t nType)
{
    return m_TypeV[nType];
}

bool Player::ResetType(uint8_t nType, bool bThisType)
{
    m_TypeV[nType] = bThisType;
    return bThisType;
}

bool Player::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Player::ResetState(uint8_t nState, bool bThisState)
{
    m_StateV[nState] = bThisState;
    return bThisState;
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

void Player::OperateNet(uint8_t nType, const uint8_t * pData, size_t nDataLen)
{
    switch(nType){
        case CM_MOTION: Net_CM_MOTION(nType, pData, nDataLen); break;
        default: break;
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
        extern MemoryPN *g_MemoryPN;
        auto pMem = (SMCORecord *)g_MemoryPN->Get(sizeof(SMCORecord));

        pMem->Type = OBJECT_PLAYER;

        pMem->Common.MapX  = X();
        pMem->Common.MapY  = Y();
        pMem->Common.R     = R();
        pMem->Common.MapID = MapID();

        pMem->Player.GUID      = m_GUID;
        pMem->Player.JobID     = m_JobID;
        pMem->Player.Level     = m_Level;
        pMem->Player.Direction = m_Direction;

        extern NetPodN *g_NetPodN;
        g_NetPodN->Send(nSessionID, SM_CORECORD, (uint8_t *)pMem, sizeof(SMCORecord), [pMem](){ g_MemoryPN->Free(pMem); });
        return;
    }

    extern MonoServer *g_MonoServer;
    g_MonoServer->AddLog(LOGTYPE_WARNING, "invalid session id");
    g_MonoServer->Restart();
}
