/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/29/2016 23:42:43
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

#include "player.hpp"
#include "charobject.hpp"

Player::Player(uint32_t nUID, uint32_t nAddTime, uint32_t nGUID, uint32_t nSID)
    : CharObject(nUID, nAddTime)
    , m_SID(nSID)
    , m_GUID(nGUID)
{
}

Player::~Player()
{
}

void Player::Operate(const MessagePack &, const Theron::Address &)
{
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

bool Player::SetType(uint8_t nType, bool bThisType)
{
    m_TypeV[nType] = 1;
    return bThisType;
}

bool Player::State(uint8_t nState)
{
    return m_StateV[nState];
}

bool Player::SetState(uint8_t nState, bool bThisState)
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
