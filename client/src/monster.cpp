/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57 PM
 *  Last Modified: 03/30/2017 14:22:29
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


Monster::Monster(uint32_t nMonsterID, uint32_t nUID, ProcessRun *pRun)
    : Creature(nUID, pRun)
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
        m_LogicUpdateTime = fTimeNow;
    }

    // 2. time for frame update
    if(fTimeNow > m_FrameUpdateTime + m_FrameDelay){
        auto nFrameCount = (int)(FrameCount());
        if(nFrameCount){
            switch(m_Action){
                case ACTION_WALK:
                    {
                        if(m_Frame == (nFrameCount - (((m_Direction == DIR_UPLEFT) ? 2 : 5) + 1))){
                            int nX, nY;
                            EstimateLocation(Speed(), &nX, &nY);
                            if(m_ProcessRun->CanMove(true, nX, nY)){
                                ResetLocation(nX, nY);
                                m_Frame = (m_Frame + 1) % nFrameCount;
                            }else if(m_ProcessRun->CanMove(false, nX, nY)){
                                // move-able but some one is on the way
                                // we just stay here and wait
                            }else{
                                m_Frame  = 0;
                                m_Action = ACTION_STAND;
                            }
                        }else if(m_Frame == ((int)(FrameCount()) - 1)){
                            m_Frame  = 0;
                            m_Action = ACTION_STAND;

                            // if((X() != m_MoveDstX) || (Y() != m_MoveDstY)){
                            //     ClientPathFinder stPathFinder(false);
                            //     if(stPathFinder.Search(X(), Y(), m_MoveDstX, m_MoveDstX)){
                            //         if(auto pNode = stPathFinder.GetSolutionStart()){
                            //             if((pNode = stPathFinder.GetSolutionNext())){
                            //                 int nDX = pNode->X() - X() + 1;
                            //                 int nDY = pNode->Y() - Y() + 1;
                            //
                            //                 const static int nDirV[][3] = {
                            //                     {DIR_UPLEFT,    DIR_UP,         DIR_UPRIGHT  },
                            //                     {DIR_LEFT,      DIR_UNKNOWN,    DIR_RIGHT    },
                            //                     {DIR_DOWNLEFT,  DIR_DOWN,       DIR_DOWNRIGHT}};
                            //                 m_Speed     = m_NextSpeed;
                            //                 m_Direction = nDirV[nDY][nDX];
                            //             }
                            //         }
                            //     }
                            // }else{
                            //     m_Frame  = 0;
                            //     m_Action = ACTION_STAND;
                            // }
                        }else{
                            m_Frame = (m_Frame + 1 ) % nFrameCount;
                        }
                        break;
                    }
                case ACTION_STAND:
                    {
                        m_Frame = (m_Frame + 1 ) % ((int)(FrameCount()));
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
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
