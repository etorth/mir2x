/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57 PM
 *  Last Modified: 03/31/2017 11:47:20
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
#include <SDL2/SDL.h>

#include "log.hpp"
#include "monster.hpp"
#include "mathfunc.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "clientpathfinder.hpp"

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


Monster::Monster(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
    : Creature(nUID, pRun, nX, nY, nAction, nDirection, nSpeed)
    , m_MonsterID(nMonsterID)
    , m_LookIDN(0)
    , m_UpdateDelay(200.0)
    , m_LastUpdateTime(0.0)
{}

void Monster::Update()
{
    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > m_UpdateDelay + m_LastUpdateTime){
        // 1. record the update time
        m_LastUpdateTime = fTimeNow;

        // 2. logic update

        // 3. frame update
        auto nFrameCount = (int)(FrameCount());
        if(nFrameCount){
            switch(m_Action){
                case ACTION_STAND:
                    {
                        OnStand();
                        break;
                    }
                case ACTION_WALK:
                    {
                        OnWalk();
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
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

    int nShiftX = 0;
    int nShiftY = 0;
    EstimatePixelShift(&nShiftX, &nShiftY);

    {
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_INFO, "X = %d, Y = %d, Action = %d, Frame = %d, ShiftX = %d, ShiftY = %d", X(), Y(), m_Action, m_Frame, nShiftX, nShiftY);
    }

    extern SDLDevice *g_SDLDevice;
    if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
    g_SDLDevice->DrawTexture(pFrame1, m_X * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);
    g_SDLDevice->DrawTexture(pFrame0, m_X * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);
}

size_t Monster::FrameCount()
{
    return (s_knActionTableV[m_Action] >= 0) ? GetGInfoRecord(m_MonsterID).FrameCount(m_LookIDN, s_knActionTableV[m_Action], m_Direction) : 0;
}
