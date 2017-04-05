/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57 PM
 *  Last Modified: 04/05/2017 14:30:44
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
Monster::Monster(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun, int nX, int nY, int nAction, int nDirection, int nSpeed)
    : Creature(nUID, pRun, nX, nY, nAction, nDirection, nSpeed)
    , m_MonsterID(nMonsterID)
    , m_LookIDN(0)
    , m_UpdateDelay(200.0)
    , m_LastUpdateTime(0.0)
{}

bool Monster::UpdateMotion()
{
    if(m_MotionList.empty()){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Empty motion list");
        return false;
    }

    if(MotionValid(m_MotionList.front().Motion, m_MotionList.front().Direction)){
        switch(m_MotionList.front().Motion){
            case MOTION_STAND:
                {
                    return UpdateMotionOnStand();
                }
            case MOTION_WALK:
                {
                    return UpdateMotionOnWalk();
                }
            case MOTION_ATTACK:
                {
                    return UpdateMotionOnAttack();
                }
            case MOTION_UNDERATTACK:
                {
                    return UpdateMotionOnUnderAttack();
                }
            case MOTION_DIE:
                {
                    return UpdateMotionOnDie();
                }
            default:
                {
                    return false;
                }
        }
    }
    return false;
}

void Monster::Update()
{
    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > m_UpdateDelay + m_LastUpdateTime){
        // 1. record the update time
        m_LastUpdateTime = fTimeNow;

        // 2. logic update

        // 3. motion update
        UpdateMotion();
    }
}

void Monster::Draw(int nViewX, int nViewY)
{
    if(ValidG()){
        auto nGfxID = GfxID();
        if(nGfxID >= 0){
            uint32_t nKey0 = 0X00000000 + (LookID() << 12) + ((uint32_t)(nGfxID) << 5) + m_Frame; // body
            uint32_t nKey1 = 0X01000000 + (LookID() << 12) + ((uint32_t)(nGfxID) << 5) + m_Frame; // shadow

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

            extern SDLDevice *g_SDLDevice;
            if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
            g_SDLDevice->DrawTexture(pFrame1, m_X * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);
            g_SDLDevice->DrawTexture(pFrame0, m_X * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, m_Y * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);
        }
    }
}

size_t Monster::MotionFrameCount()
{
    return (GfxID() < 0) ? 0 : GetGInfoRecord(m_MonsterID).FrameCount(m_LookIDN, GfxID());
}

bool Monster::OnReportAction(int nAction, int, int nDirection, int nSpeed, int nX, int nY)
{
    if(EraseNextMotion()){
        if(LDistance2(m_MotionList.front().NextX, m_MotionList.front().NextY, nX, nY)){
            if(!ParseMovePath(MOTION_WALK, nSpeed, m_MotionList.front().NextX, m_MotionList.front().NextY, nX, nY)){
                return false;
            }
        }

        switch(nAction){
            case ACTION_STAND:
                {
                    m_MotionList.push_back({MOTION_STAND, nDirection, 0, nX, nY, nX, nY});
                    return true;
                }
            case ACTION_MOVE:
                {
                    m_MotionList.push_back({MOTION_STAND, m_MotionList.back().Direction, 0, nX, nY, nX, nY});
                    return true;
                }
            case ACTION_ATTACK:
                {
                    m_MotionList.push_back({MOTION_ATTACK, nDirection, 0, nX, nY, nX, nY});
                    return true;
                }
            case ACTION_UNDERATTACK:
                {
                    m_MotionList.push_back({MOTION_UNDERATTACK, nDirection, 0, nX, nY, nX, nY});
                    return true;
                }
            case ACTION_DIE:
                {
                    m_MotionList.push_back({MOTION_DIE, nDirection, 0, nX, nY, nX, nY});
                    return true;
                }
            default:
                {
                    return false;
                }
        }
    }
    return false;
}

bool Monster::UpdateMotionOnStand()
{
    switch(m_MotionList.size()){
        case 0:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Empty motion list");
                return false;
            }
        case 1:
            {
                if(true
                        && m_MotionList.front().Motion == MOTION_STAND
                        && m_MotionList.front().X      == X()
                        && m_MotionList.front().Y      == Y()){
                    return AdvanceMotionFrame(1);
                }else{
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_FATAL, "Invalid motion list");
                    return false;
                }
            }
        default:
            {
                if(true
                        && (std::next(m_MotionList.begin(), 0)->Motion == MOTION_STAND)
                        && (std::next(m_MotionList.begin(), 0)->X      == X())
                        && (std::next(m_MotionList.begin(), 0)->Y      == Y())
                        && (std::next(m_MotionList.begin(), 1)->X      == X())
                        && (std::next(m_MotionList.begin(), 1)->Y      == Y())){
                    m_Frame = 0;
                    m_MotionList.pop_front();
                    return true;
                }else{
                    extern Log *g_Log;
                    g_Log->AddLog(LOGTYPE_FATAL, "Invalid motion list");
                    return false;
                }
            }
    }
}

bool Monster::UpdateMotionOnWalk()
{
    switch(m_MotionList.size()){
        case 0:
            {
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Empty motion list");
                return false;
            }
        default:
            {
                if((m_MotionList.front().Motion == MOTION_WALK)){
                    auto nFrameCount = (int)(MotionFrameCount());
                    if(nFrameCount > 0){
                        if(m_Frame == (nFrameCount - 1)){
                            return MoveNextMotion();
                        }else{
                            if(m_Frame == (nFrameCount - (((m_MotionList.front().Direction == DIR_UPLEFT) ? 2 : 5) + 1))){
                                m_X = m_MotionList.front().NextX;
                                m_Y = m_MotionList.front().NextY;
                            }
                            return AdvanceMotionFrame(1);
                        }
                    }
                }

                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_FATAL, "Invalid motion list");
                return false;

            }
    }
}

bool Monster::UpdateMotionOnAttack()
{
    return false;
}

bool Monster::UpdateMotionOnUnderAttack()
{
    return false;
}

bool Monster::UpdateMotionOnDie()
{
    return false;
}

bool Monster::OnReportState()
{
    return false;
}
