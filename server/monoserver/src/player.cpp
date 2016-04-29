/*
 * =====================================================================================
 *
 *       Filename: player.cpp
 *        Created: 04/07/2016 03:48:41 AM
 *  Last Modified: 04/29/2016 00:13:35
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
