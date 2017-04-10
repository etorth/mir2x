/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57 PM
 *  Last Modified: 04/09/2017 23:58:53
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
//
std::unordered_map<uint32_t, MonsterGInfo> Monster::s_MonsterGInfoMap;
bool Monster::Update()
{
    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > m_UpdateDelay + m_LastUpdateTime){
        // 1. record the update time
        m_LastUpdateTime = fTimeNow;

        // 2. logic update

        // 3. motion update
        switch(m_CurrMotion.Motion){
            case MOTION_STAND:
                {
                    return MoveNextMotion();
                }
            default:
                {
                    return UpdateGeneralMotion(false);
                }
        }
    }
    return true;
}

bool Monster::Draw(int nViewX, int nViewY)
{
    if(ValidG()){
        auto nGfxID = GfxID(m_CurrMotion.Motion, m_CurrMotion.Direction);
        if(nGfxID >= 0){
            uint32_t nKey0 = 0X00000000 + (LookID() << 12) + ((uint32_t)(nGfxID) << 5) + m_CurrMotion.Frame; // body
            uint32_t nKey1 = 0X01000000 + (LookID() << 12) + ((uint32_t)(nGfxID) << 5) + m_CurrMotion.Frame; // shadow

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
            g_SDLDevice->DrawTexture(pFrame1, X() * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);
            g_SDLDevice->DrawTexture(pFrame0, X() * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);
        }
    }

    return true;
}

size_t Monster::MotionFrameCount()
{
    return (GfxID(m_CurrMotion.Motion, m_CurrMotion.Direction) < 0) ? 0 : GetGInfoRecord(m_MonsterID).FrameCount(m_LookIDN, GfxID(m_CurrMotion.Motion, m_CurrMotion.Direction));
}

bool Monster::ParseNewAction(const ActionNode &rstAction)
{
    m_MotionQueue.clear();
    if(ActionValid(rstAction)){
        if(LDistance2(m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y)){
            if(!ParseMovePath(MOTION_WALK, 1, m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y)){
                return false;
            }
        }

        switch(rstAction.Action){
            case ACTION_STAND:
                {
                    m_MotionQueue.push_back({MOTION_STAND, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_MOVE:
                {
                    m_MotionQueue.push_back({MOTION_WALK, rstAction.Direction, rstAction.Speed, rstAction.X, rstAction.Y, rstAction.EndX, rstAction.EndY});
                    break;
                }
            case ACTION_ATTACK:
                {
                    m_MotionQueue.push_back({MOTION_ATTACK, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_UNDERATTACK:
                {
                    m_MotionQueue.push_back({MOTION_UNDERATTACK, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_DIE:
                {
                    m_MotionQueue.push_back({MOTION_DIE, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            default:
                {
                    return false;
                }
        }

        return MotionQueueValid();
    }
    return false;
}

bool Monster::Location(int *pX, int *pY)
{
    switch(m_CurrMotion.Motion){
        case MOTION_WALK:
            {
                auto nX0        = m_CurrMotion.X;
                auto nY0        = m_CurrMotion.Y;
                auto nX1        = m_CurrMotion.EndX;
                auto nY1        = m_CurrMotion.EndY;
                auto nFrame     = m_CurrMotion.Frame;
                auto nDirection = m_CurrMotion.Direction;

                if(pX){ *pX = (nFrame < ((int)(MotionFrameCount()) - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 1))) ? nX0 : nX1; }
                if(pY){ *pY = (nFrame < ((int)(MotionFrameCount()) - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 1))) ? nY0 : nY1; }

                return true;
            }
        default:
            {
                if(pX){ *pX = m_CurrMotion.X; }
                if(pY){ *pY = m_CurrMotion.Y; }

                return true;
            }
    }
}

int32_t Monster::GfxID(int nMotion, int nDirection)
{
    static const std::unordered_map<int, int> stActionGfxIDRecord = {
        {MOTION_STAND,       0},
        {MOTION_WALK,        1},
        {MOTION_ATTACK,      2},
        {MOTION_UNDERATTACK, 3},
        {MOTION_DIE,         4}};

    if(stActionGfxIDRecord.find(nMotion) != stActionGfxIDRecord.end()){
        switch(nDirection){
            case DIR_UP         : return ( 0 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_DOWN       : return ( 4 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_LEFT       : return ( 6 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_RIGHT      : return ( 2 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_UPLEFT     : return ( 7 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_UPRIGHT    : return ( 1 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_DOWNLEFT   : return ( 5 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_DOWNRIGHT  : return ( 3 + (stActionGfxIDRecord.at(nMotion) << 3));
            case DIR_NONE       : return (-1 + 0);
            default             : return (-1 + 0);
        }
    }

    return -1;
}

bool Monster::ActionValid(const ActionNode &rstAction)
{
    auto nDistance = LDistance2(rstAction.X, rstAction.Y, rstAction.EndX, rstAction.EndY);
    switch(rstAction.Action){
        case ACTION_STAND:
        case ACTION_ATTACK:
        case ACTION_UNDERATTACK:
        case ACTION_DIE:
            {
                switch(rstAction.Direction){
                    case DIR_UP:
                    case DIR_UPRIGHT:
                    case DIR_RIGHT:
                    case DIR_DOWNRIGHT:
                    case DIR_DOWN:
                    case DIR_DOWNLEFT:
                    case DIR_LEFT:
                    case DIR_UPLEFT:
                        {
                            return nDistance ? false : true;
                        }
                    case DIR_NONE:
                    default:
                        {
                            return false;
                        }
                }
            }
        case ACTION_MOVE:
            {
                switch(rstAction.Direction){
                    case DIR_UP:
                    case DIR_UPRIGHT:
                    case DIR_RIGHT:
                    case DIR_DOWNRIGHT:
                    case DIR_DOWN:
                    case DIR_DOWNLEFT:
                    case DIR_LEFT:
                    case DIR_UPLEFT:
                        {
                            return ((nDistance == 1) || (nDistance == 2)) ? true : false;
                        }
                    case DIR_NONE:
                    default:
                        {
                            return false;
                        }
                }
                break;
            }
        default:
            {
                return false;
            }
    }
}

bool Monster::MotionValid(const MotionNode &rstMotion)
{
    auto nDistance = LDistance2(rstMotion.X, rstMotion.Y, rstMotion.EndX, rstMotion.EndY);
    switch(rstMotion.Motion){
        case MOTION_STAND:
        case MOTION_ATTACK:
        case MOTION_UNDERATTACK:
        case MOTION_DIE:
            {
                switch(rstMotion.Direction){
                    case DIR_UP:
                    case DIR_UPRIGHT:
                    case DIR_RIGHT:
                    case DIR_DOWNRIGHT:
                    case DIR_DOWN:
                    case DIR_DOWNLEFT:
                    case DIR_LEFT:
                    case DIR_UPLEFT:
                        {
                            return nDistance ? false : true;
                        }
                    case DIR_NONE:
                    default:
                        {
                            return false;
                        }
                }
            }
        case MOTION_WALK:
            {
                switch(rstMotion.Direction){
                    case DIR_UP:
                    case DIR_UPRIGHT:
                    case DIR_RIGHT:
                    case DIR_DOWNRIGHT:
                    case DIR_DOWN:
                    case DIR_DOWNLEFT:
                    case DIR_LEFT:
                    case DIR_UPLEFT:
                        {
                            return ((nDistance == 1) || (nDistance == 2)) ? true : false;
                        }
                    case DIR_NONE:
                    default:
                        {
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

bool Monster::ParseNewState(const StateNode &)
{
    return true;
}
