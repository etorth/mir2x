/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 04/19/2017 18:27:34
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

#include "myhero.hpp"

MyHero::MyHero(uint32_t nUID, uint32_t nDBID, bool bMale, uint32_t nDressID, ProcessRun *pRun, const ActionNode &rstAction)
	: Hero(nUID, nDBID, bMale, nDressID, pRun, rstAction)
{}

bool MyHero::Update()
{
    return Hero::Update();
}

bool MyHero::RequestMove(int nX, int nY)
{
    m_MotionQueue.clear();
    return ParseMovePath(MOTION_WALK, 1, m_CurrMotion.EndX, m_CurrMotion.EndY, nX, nY);
}
