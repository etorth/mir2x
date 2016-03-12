/*
 * =====================================================================================
 *
 *       Filename: actor.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 03/12/2016 01:24:46
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

Actor::Actor(int nSID, int nUID, int nGenTime)
    : m_MapExt(nullptr)
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

Actor::~Actor()
{}

void Actor::SetHP(int nHP)
{
    m_HP = nHP;
}

int Actor::SID()
{
    return m_SID;
}

int Actor::UID()
{
    return m_UID;
}

int Actor::ScreenX()
{
    return X() - m_MapExt->ViewX();
}

int Actor::ScreenY()
{
    return Y() - m_MapExt->ViewY();
}

int Actor::X()
{
    return m_X;
}

int Actor::Y()
{
    return m_Y;
}

void Actor::SetNextPosition(int nX, int nY)
{
    m_X = nX;
    m_Y = nY;
}

void Actor::SetNextState(int nState)
{
    m_State = nState;
}

void Actor::SetState(int nState){
    m_State = nState;
}

void Actor::SetPosition(int nX, int nY)
{
    m_X = nX;
    m_Y = nY;
}

void Actor::SetDirection(int nDir)
{
    m_Direction = nDir;
}

int Actor::GenTime()
{
    return m_GenTime;
}

void Actor::SetMap(int nX, int nY, Mir2xMapExt *pMapExt)
{
    m_X   = nX;
    m_Y   = nY;
    m_MapExt = pMapExt;
}

void Actor::InnDraw(bool bBody, const std::function<void()> &fnDraw)
{
    uint32_t nKey = (bBody ? 0 : (1 << 14))
        + (((uint32_t)m_State      & 0X0000003F) << 23)
        + (((uint32_t)m_Direction  & 0X00000007) << 20)
        + (((uint32_t)m_FrameIndex & 0X0000003F) << 14)
        + (((uint32_t)m_LID        & 0X00001FFF) <<  0);

    fnDraw(m_X, m_Y, nKey);
}

void Actor::UpdateCurrentState()
{
    m_FrameIndex = ((m_FrameIndex + 1) % FrameCount());
}

void Actor::UpdateWithNewState()
{
}

int Actor::CalculateDirection(int nDX, int nDY)
{
    int nDirection = 0;
    if(nDX == 0){
        if(nDY > 0){
            nDirection = 4;
        }else{
            nDirection = 0;
        }
    }else{
        double dATan = std::atan(1.0 * nDY / nDX);
        if(nDX > 0){
            nDirection = std::lround(2.0 + dATan * 4.0 / 3.1416) % 8;
        }else{
            nDirection = std::lround(6.0 + dATan * 4.0 / 3.1416) % 8;
        }
    }
    return nDirection;
}

void Actor::EstimateNextPosition(int nDistance)
{
    double dDX[] = {+0.000, +0.707, +1.000, +0.707, +0.000, -0.707, -1.000, -0.707};
    double dDY[] = {-1.000, -0.707, +0.000, +0.707, +1.000, +0.707, +0.000, -0.707};

    m_EstimateNextX = m_X + std::lround(dDX[m_Direction] * nDistance);
    m_EstimateNextY = m_Y + std::lround(dDY[m_Direction] * nDistance);
}

int Actor::EstimateNextX()
{
    return m_EstimateNextX;
}

int Actor::EstimateNextY()
{
    return m_EstimateNextY;
}

void Actor::Goto(int nX, int nY)
{
    EstimateNextPosition();
    m_NextX = nX;
    m_NextY = nY;
}

bool Actor::TryStepMove(std::function<bool()> fnExtCheck)
{
    EstimateNextPosition();
    return m_MapExt->ValidPosition(m_EstimateX, m_EstimateY, this);
}



