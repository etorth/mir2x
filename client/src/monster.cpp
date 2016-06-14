/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57 PM
 *  Last Modified: 06/13/2016 17:19:45
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
#include <SDL2/SDL.h>

// static monster global info map
std::unordered_map<uint32_t, MonsterGInfo> Monster::s_MonsterGInfoMap;

Monster::Monster(uint32_t nUID, uint32_t nAddTime, uint32_t nMonsterID)
    : Creature(nUID, nAddTime)
    , m_MonsterID(nMonsterID)
    , m_LookIDN(0)
{}

Monster::~Monster()
{}

void Monster::Update()
{
    if(SDL_GetTicks() < m_UpdateTime + m_FrameDelay){ return; }

    // ok now it's time to update
    m_UpdateTime = SDL_GetTicks();
    uint32_t nFrameCount = FrameCount();
    if(nFrameCount){
        m_Frame = (m_Frame + 1) % nFrameCount;
    }
}

void Monster::Draw(int nViewX, int nViewY)
{
    // 0. check the validness of graphical resource
    //    please check it or all you get will be LookID = 0
    if(!ValidG()){ return; }

    // 1. ok draw it
    uint32_t nBaseKey = (LookID() << 12) + (m_State << 8) + (m_Direction << 5);
    uint32_t nKey0 = 0X00000000 + nBaseKey + m_Frame; // body
    uint32_t nKey1 = 0X01000000 + nBaseKey + m_Frame; // shadow

    int nDX = 0;
    int nDY = 0;

    extern PNGTexOffDBN *g_PNGTexOffDBN;
    auto pFrame0 = g_PNGTexOffDBN->Retrieve(nKey0, &nDX, &nDY);
    auto pFrame1 = g_PNGTexOffDBN->Retrieve(nKey1, &nDX, &nDY);

    extern SDLDevice *g_SDLDevice;
    g_SDLDevice->DrawTexture(pFrame1, m_X + nDX - nViewX, m_Y + nDY - nViewY);
    g_SDLDevice->DrawTexture(pFrame0, m_X + nDX - nViewX, m_Y + nDY - nViewY);
}
