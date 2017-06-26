/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 09/03/2015 03:49:00
 *  Last Modified: 06/25/2017 18:26:39
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

#include "log.hpp"
#include "hero.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "motionnode.hpp"
#include "pngtexoffdbn.hpp"

Hero::Hero(uint32_t nUID, uint32_t nDBID, bool bGender, uint32_t nDress, ProcessRun *pRun, const ActionNode &rstAction)
    : Creature(nUID, pRun)
    , m_DBID(nDBID)
    , m_Gender(bGender)
    , m_Horse(0)
    , m_Weapon(5)
    , m_Hair(0)
    , m_HairColor(0)
    , m_Dress(nDress - nDress + 6)
    , m_DressColor(0)
    , m_OnHorse(false)
{
    m_CurrMotion.Motion    = MOTION_STAND;
    m_CurrMotion.Speed     = 0;
    m_CurrMotion.Direction = DIR_UP;
    m_CurrMotion.X         = rstAction.X;
    m_CurrMotion.Y         = rstAction.Y;
    m_CurrMotion.EndX      = rstAction.EndX;
    m_CurrMotion.EndY      = rstAction.EndY;

    if(!ParseNewAction(rstAction, true)){
        extern Log *g_Log;
        g_Log->AddLog(LOGTYPE_FATAL, "Construct hero failed");
    }
}

bool Hero::Draw(int nViewX, int nViewY)
{
    auto nDress     = m_Dress;
    auto nMotion    = m_CurrMotion.Motion;
    auto nDirection = m_CurrMotion.Direction;

    auto nGfxID = GfxID(nDress, nMotion, nDirection);
    if(nGfxID < 0){
        m_CurrMotion.Print();
        return false;
    }

    // human gfx indexing
    // 04 - 00 :     frame : max =  32
    // 07 - 05 : direction : max =  08 : +
    // 13 - 08 :    motion : max =  64 : +----> GfxID
    // 21 - 14 :     dress : max = 256 
    //      22 :       sex :
    //      23 :    shadow :
    uint32_t nKey0 = ((uint32_t)(0) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxID & 0X01FFFF)) << 5) + m_CurrMotion.Frame;
    uint32_t nKey1 = ((uint32_t)(1) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxID & 0X01FFFF)) << 5) + m_CurrMotion.Frame;

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

    auto fnDrawWeapon = [this, nGfxID, nViewX, nViewY, nShiftX, nShiftY](bool bShadow) -> void
    {
        // in DB weapon index starts from 0
        // and in client m_Weapon = 0 means ``no weapon used" and 
        if(m_Weapon){
            // 04 - 00 :     frame : max =  32
            // 07 - 05 : direction : max =  08 : +
            // 13 - 08 :    motion : max =  64 : +----> GfxID
            // 21 - 14 :    weapon : max = 256 
            //      22 :    gender :
            //      23 :    shadow :
            uint32_t nWeaponGfxID = (((uint32_t)(m_Weapon - 1) & 0X00FF) << 9) + ((uint32_t)(nGfxID) & 0X01FF);
            uint32_t nWeaponKey   = (((uint32_t)(bShadow ? 1 : 0)) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (nWeaponGfxID << 5) + m_CurrMotion.Frame;

            int nWeaponDX = 0;
            int nWeaponDY = 0;

            extern SDLDevice *g_SDLDevice;
            extern PNGTexOffDBN *g_WeaponDBN;
            auto pWeapon = g_WeaponDBN->Retrieve(nWeaponKey, &nWeaponDX, &nWeaponDY);

            if(pWeapon && bShadow){ SDL_SetTextureAlphaMod(pWeapon, 128); }
            g_SDLDevice->DrawTexture(pWeapon, X() * SYS_MAPGRIDXP + nWeaponDX - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nWeaponDY - nViewY + nShiftY);
        }
    };


    fnDrawWeapon(true);

    extern SDLDevice *g_SDLDevice;
    if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
    g_SDLDevice->DrawTexture(pFrame1, X() * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);


    if(m_Weapon && WeaponOrder(((nGfxID & 0X01F8) >> 3) + 1, (nGfxID & 0X07) + 1, m_CurrMotion.Frame)){
        fnDrawWeapon(false);
    }

    g_SDLDevice->DrawTexture(pFrame0, X() * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);

    if(m_Weapon && !WeaponOrder(((nGfxID & 0X01F8) >> 3) + 1, (nGfxID & 0X07) + 1, m_CurrMotion.Frame)){
        fnDrawWeapon(false);
    }

    // draw HP bar
    // if current m_HPMqx is zero we draw full bar
    {
        extern PNGTexDBN *g_PNGTexDBN;
        auto pBar0 = g_PNGTexDBN->Retrieve(0XFF0014);
        auto pBar1 = g_PNGTexDBN->Retrieve(0XFF0015);

        int nW = -1;
        int nH = -1;
        SDL_QueryTexture(pBar1, nullptr, nullptr, &nW, &nH);
        g_SDLDevice->DrawTexture(pBar1,
                X() * SYS_MAPGRIDXP - nViewX + nShiftX +  7,
                Y() * SYS_MAPGRIDYP - nViewY + nShiftY - 53,
                0,
                0,
                (int)(std::lround(nW * (m_HPMax ? std::min<double>(1.0, (1.0 * m_HP) / m_HPMax) : 1.0))),
                nH);

        g_SDLDevice->DrawTexture(pBar0,
                X() * SYS_MAPGRIDXP - nViewX + nShiftX +  7,
                Y() * SYS_MAPGRIDYP - nViewY + nShiftY - 53);
    }
    return true;
}

bool Hero::Update()
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
                    if(m_MotionQueue.empty()){
                        return AdvanceMotionFrame(1);
                    }else{
                        // move to next motion will reset frame as 0
                        // if current there is no more motion pending
                        // it will add a MOTION_STAND
                        //
                        // we don't want to reset the frame here
                        return MoveNextMotion();
                    }
                }
            default:
                {
                    return UpdateGeneralMotion(false);
                }
        }
    }
    return true;
}

int32_t Hero::GfxID(int nDress, int nMotion, int nDirection)
{
    static const std::unordered_map<int, int> stMotionGfxIDRecord = {
        {MOTION_STAND,              0},
        {MOTION_WALK,              21},
        {MOTION_RUN,               22},

        {MOTION_CASTMAGIC0,         2},
        {MOTION_CASTMAGIC1,         3},

        {MOTION_BEFOREATTACK,       7},
        {MOTION_UNDERATTACK,       15},
        {MOTION_DIE,                7},

        {MOTION_ONEHANDATTACK0,     8},
        {MOTION_ONEHANDATTACK1,     9},
        {MOTION_ONEHANDATTACK2,    10},
        {MOTION_ONEHANDATTACK3,    11},

        {MOTION_TWOHANDATTACK0,    12},
        {MOTION_TWOHANDATTACK1,    13},
        {MOTION_TWOHANDATTACK2,    14},
        {MOTION_TWOHANDATTACK3,    15}};

    if(stMotionGfxIDRecord.find(nMotion) != stMotionGfxIDRecord.end()){
        switch(nDirection){
            case DIR_UP         : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 0;
            case DIR_DOWN       : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 4;
            case DIR_LEFT       : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 6;
            case DIR_RIGHT      : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 2;
            case DIR_UPLEFT     : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 7;
            case DIR_UPRIGHT    : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 1;
            case DIR_DOWNLEFT   : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 5;
            case DIR_DOWNRIGHT  : return ((nDress & 0XFF) << 9) + (stMotionGfxIDRecord.at(nMotion) << 3) + 3;
            case DIR_NONE       : break;
            default             : break;
        }
    }

    return -1;
}

bool Hero::MotionValid(const MotionNode &rstMotion)
{
    auto nDistance = LDistance2(rstMotion.X, rstMotion.Y, rstMotion.EndX, rstMotion.EndY);
    switch(rstMotion.Motion){
        case MOTION_STAND:

        case MOTION_CASTMAGIC0:
        case MOTION_CASTMAGIC1:

        case MOTION_BEFOREATTACK:
        case MOTION_UNDERATTACK:
        case MOTION_DIE:

        case MOTION_ONEHANDATTACK0:
        case MOTION_ONEHANDATTACK1:
        case MOTION_ONEHANDATTACK2:
        case MOTION_ONEHANDATTACK3:
        case MOTION_TWOHANDATTACK0:
        case MOTION_TWOHANDATTACK1:
        case MOTION_TWOHANDATTACK2:
        case MOTION_TWOHANDATTACK3:
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
        case MOTION_HORSEWALK:
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
        case MOTION_RUN:
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
                            return ((nDistance == 4) || (nDistance == 8)) ? true : false;
                        }
                    case DIR_NONE:
                    default:
                        {
                            return false;
                        }
                }
            }
        case MOTION_HORSERUN:
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
                            return ((nDistance == 9) || (nDistance == 18)) ? true : false;
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

bool Hero::ParseNewAction(const ActionNode &rstAction, bool)
{
    // currently we ignore the local or remote flag for Hero
    // at it later if we support ACTION_PUSH which changes hero locations

    // 1. clean current motion queue
    //    any new action will erase all pending motions
    m_MotionQueue.clear();

    // 2. parse action into motions
    //    and assign the ActionNode::ID -> MotionNode::ID
    if(ActionValid(rstAction)){
        if(LDistance2(m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y)){
            if(!ParseMovePath(MOTION_WALK, 1, m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y)){
                return false;
            }
        }

        switch(rstAction.Action){
            case ACTION_STAND:
                {
                    m_MotionQueue.push_back({MOTION_STAND, 0, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_MOVE:
                {
                    if(!ParseMovePath(rstAction.ActionParam, rstAction.Speed, rstAction.X, rstAction.Y, rstAction.EndX, rstAction.EndY)){ extern Log *g_Log;
                        g_Log->AddLog(LOGTYPE_WARNING, "Parse move action into motion failed");
                        return false;
                    }
                    break;
                }
            case ACTION_ATTACK:
                {
                    m_MotionQueue.push_back({MOTION_ATTACK, 0, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_UNDERATTACK:
                {
                    m_MotionQueue.push_back({MOTION_UNDERATTACK, 0, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_DIE:
                {
                    m_MotionQueue.push_back({MOTION_DIE, 0, rstAction.Direction, rstAction.X, rstAction.Y});
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

bool Hero::Location(int *pX, int *pY)
{
    switch(m_CurrMotion.Motion){
        case MOTION_WALK:
        case MOTION_RUN:
        case MOTION_HORSEWALK:
        case MOTION_HORSERUN:
            {
                auto nX0        = m_CurrMotion.X;
                auto nY0        = m_CurrMotion.Y;
                auto nX1        = m_CurrMotion.EndX;
                auto nY1        = m_CurrMotion.EndY;
                auto nFrame     = m_CurrMotion.Frame;
                auto nDirection = m_CurrMotion.Direction;


                if(pX){ *pX = (nFrame < ((int)(MotionFrameCount()) - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 0))) ? nX0 : nX1; }
                if(pY){ *pY = (nFrame < ((int)(MotionFrameCount()) - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 0))) ? nY0 : nY1; }

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

bool Hero::ActionValid(const ActionNode &rstAction)
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
                if(nDistance){
                    switch(rstAction.Direction){
                        case DIR_NONE:
                            {
                                // for action move we shouldn't have direction
                                // direction should be parsed in MotionNode
                                return true;
                            }
                        case DIR_UP:
                        case DIR_UPRIGHT:
                        case DIR_RIGHT:
                        case DIR_DOWNRIGHT:
                        case DIR_DOWN:
                        case DIR_DOWNLEFT:
                        case DIR_LEFT:
                        case DIR_UPLEFT:
                        default:
                            {
                                return false;
                            }
                    }
                }else{ return false; }
            }
        default:
            {
                return false;
            }
    }
}

size_t Hero::MotionFrameCount()
{
    if(GfxID(m_Dress, m_CurrMotion.Motion, m_CurrMotion.Direction) >= 0){
        switch(m_CurrMotion.Motion){
            case MOTION_STAND       : { return 4; }
            case MOTION_WALK        : { return 6; }
            case MOTION_RUN         : { return 6; }
            case MOTION_UNDERATTACK : { return 3; }
            default                 : { return 1; }
        }
    }else{ return 0; }
}

bool Hero::Moving()
{
    return false
        || m_CurrMotion.Motion == MOTION_RUN
        || m_CurrMotion.Motion == MOTION_WALK;
}

bool Hero::CanFocus(int, int)
{
    return true;
}

bool Hero::WeaponOrder(int nMotion, int nDirection, int nFrame)
{
    // for player there are 33 motions, each motion has 8 directions
    // and each directions has 10 frames at most:
    //
    //      table_size = 33 * 8 * 10 = 2640
    //
    // each line of the table is for one motion at one frame
    // each eight lines stands for one motion
    //
    // and this table use gfx index rather than motion index
    // I'm planning to conform the GfxID and MotionID by
    //
    //      GfxID = MotionID - 1
    //
    // zero means nothing or invalid
    // otherwise I need to sreach in table every time
    const static uint8_t s_WeaponOrder[2640]
    {
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 
        1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 
        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 
        1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 
        1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 
        0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 
        0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 
        0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
        0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 
        1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 
        1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 
        0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 
        0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 
        0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 
        0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 
        1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    };

    return (s_WeaponOrder[(nMotion - 1) * 80 + (nDirection - 1) * 10 + nFrame] == 0) ? false : true;
}
