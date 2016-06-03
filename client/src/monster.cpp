/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 8/31/2015 8:26:57 PM
 *  Last Modified: 06/02/2016 15:23:59
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

void Monster::Draw(std::function<void(uint64_t, int, int)> fnDraw)
{
    fnDraw(ShadowKey(), m_X, m_Y);
    fnDraw(BodyKey(), m_X, m_Y);
}

void Monster::QueryMonsterGInfo(uint32_t nMonsterID)
{
    extern Game *g_Game;
    g_Game->Send(CM_QUERYMONSTERGINFO, nMonsterID);
}
