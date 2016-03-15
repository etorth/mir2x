/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 9/3/2015 3:49:00 AM
 *  Last Modified: 03/15/2016 00:11:08
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

#include "hero.hpp"
#include <SDL2/SDL.h>

Hero::Hero(int nSID, int nUID, int nGenTime)
    : Actor(nSID, nUID, nGenTime)
    , m_LookID(0)
{}

Hero::~Hero()
{}

void Hero::Draw()
{
    InnDraw(3, m_LookID * 2 + m_SID % 2, m_State, m_Direction, m_FrameIndex, m_X, m_Y);
}

void Hero::Update()
{
    UpdateMotion(m_Speed);
    if()

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

void Hero::UpdateCurrentState()
{
    m_FrameIndex = ((m_FrameIndex + 1) % FrameCount());
    switch(m_State){
        case 21:
            {
                UpdateMotion(10);
                break;
            }
        default:
            {
                break;
            }
    }
}

void Hero::UpdateWithNewState()
{
}

int Hero::FrameCount()
{
    // means every gender can have totally 512 different looks
    // since we have 1024 available, use all of them
    int nGender = (m_SID & 0X000003FF) % 2;
    return Actor::FrameCount(3, nGender * 512 + m_LookID);
}
