/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 8/31/2015 8:26:57 PM
 *  Last Modified: 09/08/2015 7:36:09 AM
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
#include "monster.hpp"
#include <SDL.h>

Monster::Monster(int nSID, int nUID, int nGenTime)
    : Actor(nSID, nUID, nGenTime)
{}

void Monster::UpdateCurrentState()
{
    m_FrameIndex = ((m_FrameIndex + 1) % FrameCount());
    // switch(m_CurrentState){
    //     case STATE_STAND:
    //         {
    //             m_CurrentFrameIndex = 
    //                 (m_CurrentFrameIndex + 1) % m_CurrentActionSet->FrameCount();
    //             break;
    //         }
    //     case STATE_WALK:
    //         {
    //             if(!TryNextWalk()){
    //             }
    //         }
    //     default:
    //         break;
    // }
}

void Monster::UpdateWithNewState()
{
}

void Monster::Update()
{
	// printf("monster update here\n");
    if(SDL_GetTicks() < m_UpdateTime + m_FrameUpdateDelay){
        return;
    }

    m_UpdateTime = SDL_GetTicks();

    if(m_NextState < 0){
        // no required state, keep what'd going on
        UpdateCurrentState();
    }else{
        UpdateWithNewState();
    }
}

int Monster::GenTime()
{
    return m_GenTime;
}

void Monster::Draw()
{
    InnDraw(1, m_SID & 0X00000CFF, m_State, m_Direction, m_FrameIndex, m_X, m_Y);
}

int Monster::FrameCount()
{
    // int nPrecode = ((uint32_t)m_SID & 0XFFFFFC00) >> 10;
    // int nLID     = ((uint32_t)m_SID & 0X000003FF);
    return Actor::FrameCount(1, (uint32_t)m_SID & 0X000003FF);
}
