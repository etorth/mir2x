/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 9/3/2015 3:49:00 AM
 *  Last Modified: 04/05/2017 14:29:51
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
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "motionnode.hpp"
#include "pngtexoffdbn.hpp"

Hero::Hero(uint32_t nUID, uint32_t nDBID, bool bMale, uint32_t nDressID, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
    : Creature(nUID, pRun, nX, nY, nAction, nDirection, nSpeed)
    , m_DBID(nDBID)
    , m_Male(bMale)
    , m_DressID(nDressID)
{}

void Hero::Draw(int nViewX, int nViewY)
{
    auto nGfxID = GfxID();
    if(nGfxID >= 0){
        // human gfx indexing
        // 04 - 00 :     frame : max =  32
        // 07 - 05 : direction : max =  08 : +
        // 13 - 08 :     state : max =  64 : +----> GfxID
        // 21 - 14 :     dress : max = 256 
        //      22 :       sex :
        //      23 :    shadow :
        uint32_t nKey0 = ((uint32_t)(1) << 23) + (((uint32_t)(m_Male ? 1 : 0)) << 22) + ((uint32_t)(m_DressID & 0XFF) << 14) + (((uint32_t)(nGfxID & 0X01FF)) << 5) + m_Frame; // body
        uint32_t nKey1 = ((uint32_t)(0) << 23) + (((uint32_t)(m_Male ? 1 : 0)) << 22) + ((uint32_t)(m_DressID & 0XFF) << 14) + (((uint32_t)(nGfxID & 0X01FF)) << 5) + m_Frame; // body

        int nDX0 = 0;
        int nDY0 = 0;
        int nDX1 = 0;
        int nDY1 = 0;

        extern PNGTexOffDBN *g_HeroGfxDBN;
        auto pFrame0 = g_HeroGfxDBN->Retrieve(nKey0, &nDX0, &nDY0);
        auto pFrame1 = g_HeroGfxDBN->Retrieve(nKey1, &nDX1, &nDY1);

        int nShiftX = 0;
        int nShiftY = 0;
        EstimatePixelShift(&nShiftX, &nShiftY);

        extern SDLDevice *g_SDLDevice;
        if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
        g_SDLDevice->DrawTexture(pFrame1, m_X * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);
        g_SDLDevice->DrawTexture(pFrame0, m_X * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);
    }
}

void Hero::Update()
{
}

bool Hero::OnReportAction(int nAction, int, int nDirection, int nSpeed, int nX, int nY)
{
    if(EraseNextMotion()){
        switch(nAction){
            case ACTION_STAND:
                {
                    switch(LDistance2(X(), Y(), nX, nY)){
                        case 0:
                            {
                                m_MotionList.push_back({MOTION_STAND, nDirection, nX, nY});
                                return true;
                            }
                        default:
                            {
                                ParseMovePath(MOTION_WALK, 5, X(), Y(), nX, nY);
                                m_MotionList.push_back({MOTION_STAND, nDirection, nX, nY});
                                return true;
                            }
                    }
                }
            case ACTION_MOVE:
                {
                    return ParseMovePath(MOTION_WALK, nSpeed, m_MotionList.front().NextX, m_MotionList.front().NextY, nX, nY);
                }
            case ACTION_ATTACK:
                {
                    switch(LDistance2(X(), Y(), nX, nY)){
                        case 0:
                            {
                                m_MotionList.push_back({MOTION_ATTACK, nDirection, nX, nY});
                                return true;
                            }
                        default:
                            {
                                if(ParseMovePath(MOTION_WALK, 5, X(), Y(), nX, nY)){
                                    m_MotionList.push_back({MOTION_ATTACK, nDirection, nX, nY});
                                    return true;
                                }
                                return false;
                            }
                    }
                }
            case ACTION_UNDERATTACK:
                {
                    switch(LDistance2(X(), Y(), nX, nY)){
                        case 0:
                            {
                                m_MotionList.push_back({MOTION_UNDERATTACK, nDirection, nX, nY});
                                return true;
                            }
                        default:
                            {
                                if(ParseMovePath(MOTION_WALK, 5, X(), Y(), nX, nY)){
                                    m_MotionList.push_back({MOTION_UNDERATTACK, nDirection, nX, nY});
                                    return true;
                                }
                                return false;
                            }
                    }
                }
            case ACTION_DIE:
                {
                    switch(LDistance2(X(), Y(), nX, nY)){
                        case 0:
                            {
                                m_MotionList.push_back({MOTION_DIE, nDirection, nX, nY});
                                return true;
                            }
                        default:
                            {
                                if(ParseMovePath(MOTION_WALK, 5, X(), Y(), nX, nY)){
                                    m_MotionList.push_back({MOTION_DIE, nDirection, nX, nY});
                                    return true;
                                }
                                return false;
                            }
                    }
                }
            default:
                {
                    return false;
                }
        }
    }
    return false;
}

bool Hero::OnReportState()
{
    return true;
}

bool Hero::UpdateMotionOnStand()
{
    return false;
}

bool Hero::UpdateMotionOnWalk()
{
    return false;
}

bool Hero::UpdateMotionOnAttack()
{
    return false;
}

bool Hero::UpdateMotionOnUnderAttack()
{
    return false;
}

bool Hero::UpdateMotionOnDie()
{
    return false;
}

bool Hero::UpdateMotion()
{
    return false;
}
