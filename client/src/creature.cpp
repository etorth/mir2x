/*
 * =====================================================================================
 *
 *       Filename: creature.cpp
 *        Created: 08/31/2015 10:45:48 PM
 *  Last Modified: 03/26/2017 12:06:09
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

Creature::Creature(uint32_t nUID, uint32_t nAddTime)
    : m_UID(nUID)
    , m_AddTime(nAddTime)
    , m_X(0)
    , m_Y(0)
    , m_MapID(0)
    , m_LogicDelay(120.0)
    , m_FrameDelay(180.0)
    , m_LogicUpdateTime(0.0)
    , m_FrameUpdateTime(0.0)
    , m_Frame(0)
    , m_Speed(0)
    , m_Action(0)
    , m_Direction(0)
{}

Creature::~Creature()
{}

void Creature::EstimateLocation(int nDistance, int *pNextX, int *pNextY)
{
    int nDX[] = { 0, +1, +1, +1,  0, -1, -1, -1};
    int nDY[] = {-1, -1,  0, +1, +1, +1,  0, -1};

    if(pNextX){ *pNextX = m_X + nDistance * nDX[m_Direction]; }
    if(pNextY){ *pNextY = m_Y + nDistance * nDY[m_Direction]; }
}
