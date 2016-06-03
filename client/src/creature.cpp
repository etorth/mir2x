/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 06/02/2016 13:56:50
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

#include <string>

#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "actor.hpp"

//  +----------+-------+-----------+-------+--------+-----+
//  | reserved | state | direction | frame | shadow | LID |
//  +----------+-------+-----------+-------+--------+-----+
//  | 32       | 29    | 23        | 20    | 14     | 13  |
//  +----------+-------+-----------+-------+--------+-----+
//
//  4 bits:   16: reserved for colouration effect
//  5 bits:   32: state
//  3 bits:    8: direction
//  6 bits:   64: frame
//  1 bits:    2: shadow/body
// 13 bits: 8192: LID

Creature::Creature(int nSID, int nUID, int nGenTime)
    : m_MapExt(nullptr)
    , m_LID(0)
    , m_SID(nSID)
    , m_UID(nUID)
    , m_GenTime(nGenTime)
    , m_X(0)
    , m_Y(0)
    , m_HP(0)
    , m_FrameUpdateDelay(120.0)
    , m_FrameIndex(0)
    , m_Direction(0)
    , m_State(0)
    , m_NextState(-1)
    , m_UpdateTime(0.0)
    , m_Speed(100)
{
}

Creature::~Creature()
{}

void Creature::SetHP(int nHP)
{
    m_HP = nHP;
}

int Creature::SID()
{
    return m_SID;
}

int Creature::UID()
{
    return m_UID;
}

int Creature::ScreenX()
{
    return X() - m_MapExt->ViewX();
}

int Creature::ScreenY()
{
    return Y() - m_MapExt->ViewY();
}

int Creature::X()
{
    return m_X;
}

int Creature::Y()
{
    return m_Y;
}

void Creature::SetNextPosition(int nX, int nY)
{
    m_X = nX;
    m_Y = nY;
}

void Creature::SetNextState(int nState)
{
    m_State = nState;
}

void Creature::SetState(int nState){
    m_State = nState;
}

void Creature::SetPosition(int nX, int nY)
{
    m_X = nX;
    m_Y = nY;
}

void Creature::SetDirection(int nDir)
{
    m_Direction = nDir;
}

int Creature::GenTime()
{
    return m_GenTime;
}

void Creature::SetMap(int nX, int nY, Mir2xMapExt *pMapExt)
{
    m_X   = nX;
    m_Y   = nY;
    m_MapExt = pMapExt;
}

void Creature::InnDraw(bool bBody, // draw body or shadow
        const std::function<void(int, int, uint32_t, uint32_t)> &fnDraw)
{
    uint32_t nKey = (bBody ? 0 : (1 << 14))
        + (((uint32_t)m_State      & 0X0000003F) << 23)
        + (((uint32_t)m_Direction  & 0X00000007) << 20)
        + (((uint32_t)m_FrameIndex & 0X0000003F) << 14)
        + (((uint32_t)m_LID        & 0X00001FFF) <<  0);

    fnDraw(m_X, m_Y, nKey, 0XFFFFFFFF);
}

void Creature::UpdateCurrentState()
{
    m_FrameIndex = ((m_FrameIndex + 1) % FrameCount());
}

void Creature::UpdateWithNewState()
{
}

int Creature::CalculateDirection(int nDX, int nDY)
{
}

void Creature::EstimateNextPosition(int nDistance)
{
    double dDX[] = {+0.000, +0.707, +1.000, +0.707, +0.000, -0.707, -1.000, -0.707};
    double dDY[] = {-1.000, -0.707, +0.000, +0.707, +1.000, +0.707, +0.000, -0.707};

    m_EstimateNextX = m_X + std::lround(dDX[m_Direction] * nDistance);
    m_EstimateNextY = m_Y + std::lround(dDY[m_Direction] * nDistance);
}

bool Creature::TryStepMove(int nDistance)
{
    EstimateNextPosition(nDistance);
    return m_MapExt->ValidPosition(m_EstimateNextX, m_EstimateNextY, *this);
}
