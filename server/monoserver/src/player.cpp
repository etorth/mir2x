/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 05/30/2016 13:25:38
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
#include "charobject.hpp"

Player::Player(uint32_t nGUID, uint32_t nJobID)
    : CharObject()
    , m_GUID(nGUID)
    , m_JobID(nJobID)
    , m_SessionID(0)
{
    m_RMAddress = Theron::Address::Null();
    Install("CheckTime", [this](){ For_CheckTime(); });

    ResetType(OBJECT_PLAYER, true);
}

Player::~Player()
{
}

void Player::Operate(const MessagePack &rstMPK, const Theron::Address &rstFromAddr)
{
    switch(rstMPK.Type()){
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
        case MPK_HI:
            {
                On_MPK_HI(rstMPK, rstFromAddr);
                break;
            }
        default:
            {
                extern MonoServer *g_MonoServer;
                g_MonoServer->AddLog(LOGTYPE_WARNING, "unsupported message: type = %d, id = %d, resp = %d", rstMPK.Type(), rstMPK.ID(), rstMPK.Respond());
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
    switch(nType){
        case OBJECT_ANIMAL: return true;
        default: return false;
    }
    return false;
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
