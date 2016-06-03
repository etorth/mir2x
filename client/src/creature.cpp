/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 06/03/2016 11:59:15
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
#include <algorithm>
#include <tinyxml2.h>
#include <SDL2/SDL.h>

#include "creature.hpp"

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

Creature::Creature(uint32_t nUID, uint32_t nAddTime)
    : m_UID(nUID)
    , m_AddTime(nAddTime)
    , m_X(0)
    , m_Y(0)
    , m_R(0)
    , m_MapID(0)
    , m_UpdateTime(0.0)
    , m_FrameDelay(120.0)
    , m_Frame(0)
    , m_State(0)
    , m_Speed(0)
    , m_Direction(0)
    , m_MotionState(0)
{}

Creature::~Creature()
{}

void Creature::EstimateLocation(int nDistance, int *pNextX, int *pNextY)
{
    double dDX[] = {+0.000, +0.707, +1.000, +0.707, +0.000, -0.707, -1.000, -0.707};
    double dDY[] = {-1.000, -0.707, +0.000, +0.707, +1.000, +0.707, +0.000, -0.707};

    if(pNextX){ *pNextX = m_X + std::lround(dDX[m_Direction] * nDistance); }
    if(pNextY){ *pNextY = m_Y + std::lround(dDY[m_Direction] * nDistance); }
}
