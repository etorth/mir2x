/*
 * =====================================================================================
 *
 *       Filename: myhero.cpp
 *        Created: 08/31/2015 08:52:57 PM
 *  Last Modified: 04/20/2017 00:18:40
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

#include "log.hpp"
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

bool MyHero::MoveNextMotion()
{
    if(m_MotionQueue.empty()){
        m_CurrMotion.Motion = MOTION_STAND;
        m_CurrMotion.Speed  = 0;
        m_CurrMotion.X      = m_CurrMotion.EndX;
        m_CurrMotion.Y      = m_CurrMotion.EndY;
        m_CurrMotion.Frame  = 0;

        return true;
    }

    if(MotionQueueValid()){
        m_CurrMotion = m_MotionQueue.front();
        m_MotionQueue.pop_front();
        return true;
    }

    // oops we get invalid motion queue
    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_INFO, "Invalid motion queue:");

    m_CurrMotion.Print();
    for(auto &rstMotion: m_MotionQueue){
        rstMotion.Print();
    }

    extern Log *g_Log;
    g_Log->AddLog(LOGTYPE_FATAL, "Current motion is not valid");
    return false;
}
