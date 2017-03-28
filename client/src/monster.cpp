/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57 PM
 *  Last Modified: 03/27/2017 17:25:41
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

// this table is cooperate with enum ActionType
// I think this would cause tons of bugs to me :(
static int s_knActionTableV[] = {
   -1,      // [0]: ACTION_UNKNOWN
    0,      // [1]: ACTION_STAND
    1,      // [1]: ACTION_WALK
    2,      // [1]: ACTION_ATTACK
    3       // [1]: ACTION_DIE
};


Monster::Monster(uint32_t nMonsterID, uint32_t nUID, uint32_t nAddTime)
    : Creature(nUID, nAddTime)
    , m_MonsterID(nMonsterID)
    , m_LookIDN(0)
{}

Monster::~Monster()
{}

void Monster::Update()
{
    // 1. get current time, we split logic and frame update
    double fTimeNow = SDL_GetTicks() * 1.0;

    // 2. time for logic update
    if(fTimeNow > m_LogicUpdateTime + m_LogicDelay){
        double fDiffTime = fTimeNow - m_LogicUpdateTime;

        // location estimation
        if(m_Action == ACTION_WALK){
            int nX, nY;
            EstimateLocation((int)(Speed() * fDiffTime / 1000.0), &nX, &nY);
            ResetLocation(MapID(), nX, nY);
        }

        m_LogicUpdateTime = fTimeNow;
    }

    // 2. time for frame update
    if(fTimeNow > m_FrameUpdateTime + m_FrameDelay){
        uint32_t nFrameCount = FrameCount();
        if(nFrameCount){
            m_Frame = (m_Frame + 1) % nFrameCount;
        }

        m_FrameUpdateTime = fTimeNow;
    }
}

void Monster::Draw(int nViewX, int nViewY)
{
    // 0. check the validness of graphical resource
    //    please check it or all you get will be LookID = 0
    if(!ValidG()){ return; }

    // 1. ok draw it, check table
    // 2. to check whether the graphical resource support this action
    if(s_knActionTableV[m_Action] < 0){ return; }

    uint32_t nBaseKey = (LookID() << 12) + (((uint32_t)(s_knActionTableV[m_Action])) << 8) + (m_Direction << 5);
    uint32_t nKey0 = 0X00000000 + nBaseKey + m_Frame; // body
    uint32_t nKey1 = 0X01000000 + nBaseKey + m_Frame; // shadow

    int nDX0 = 0;
    int nDY0 = 0;
    int nDX1 = 0;
    int nDY1 = 0;

    extern PNGTexOffDBN *g_PNGTexOffDBN;
    auto pFrame0 = g_PNGTexOffDBN->Retrieve(nKey0, &nDX0, &nDY0);
    auto pFrame1 = g_PNGTexOffDBN->Retrieve(nKey1, &nDX1, &nDY1);

    extern SDLDevice *g_SDLDevice;
    if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
    g_SDLDevice->DrawTexture(pFrame1, m_X * SYS_MAPGRIDXP + nDX1 - nViewX, m_Y * SYS_MAPGRIDYP + nDY1 - nViewY);
    g_SDLDevice->DrawTexture(pFrame0, m_X * SYS_MAPGRIDXP + nDX0 - nViewX, m_Y * SYS_MAPGRIDYP + nDY0 - nViewY);
}

size_t Monster::FrameCount()
{
    if(s_knActionTableV[m_Action] < 0){ return 0; }
    return GetGInfoRecord(m_MonsterID).FrameCount(m_LookIDN, s_knActionTableV[m_Action], m_Direction);
}
