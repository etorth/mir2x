/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 09/03/2015 03:49:00
 *  Last Modified: 07/06/2017 17:56:09
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
#include "processrun.hpp"
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
    m_CurrMotion = {
        MOTION_STAND,
        0,
        DIR_UP,
        rstAction.X,
        rstAction.Y
    };

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

    auto nGfxDressID = GfxDressID(nDress, nMotion, nDirection);
    if(nGfxDressID < 0){
        m_CurrMotion.Print();
        return false;
    }

    // human gfx dress id indexing
    // 04 - 00 :     frame : max =  32
    // 07 - 05 : direction : max =  08 : +
    // 13 - 08 :    motion : max =  64 : +
    // 21 - 14 :     dress : max = 256 : +----> GfxDressID
    //      22 :       sex :
    //      23 :    shadow :
    uint32_t nKey0 = ((uint32_t)(0) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + m_CurrMotion.Frame;
    uint32_t nKey1 = ((uint32_t)(1) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + m_CurrMotion.Frame;

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

    auto fnDrawWeapon = [this, nViewX, nViewY, nShiftX, nShiftY](bool bShadow) -> void
    {
        // 04 - 00 :     frame : max =  32
        // 07 - 05 : direction : max =  08 : +
        // 13 - 08 :    motion : max =  64 : +
        // 21 - 14 :    weapon : max = 256 : +----> GfxWeaponID
        //      22 :    gender :
        //      23 :    shadow :
        auto nGfxWeaponID = GfxWeaponID(m_Weapon, m_CurrMotion.Motion, m_CurrMotion.Direction);
        if(nGfxWeaponID >= 0){
            uint32_t nWeaponKey = (((uint32_t)(bShadow ? 1 : 0)) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + ((nGfxWeaponID & 0X01FFFF) << 5) + m_CurrMotion.Frame;

            int nWeaponDX = 0;
            int nWeaponDY = 0;

            extern SDLDevice *g_SDLDevice;
            extern PNGTexOffDBN *g_WeaponDBN;
            auto pFrame = g_WeaponDBN->Retrieve(nWeaponKey, &nWeaponDX, &nWeaponDY);

            if(pFrame && bShadow){ SDL_SetTextureAlphaMod(pFrame, 128); }
            g_SDLDevice->DrawTexture(pFrame, X() * SYS_MAPGRIDXP + nWeaponDX - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nWeaponDY - nViewY + nShiftY);
        }
    };

    fnDrawWeapon(true);

    extern SDLDevice *g_SDLDevice;
    if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }
    g_SDLDevice->DrawTexture(pFrame1, X() * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY);

    if(true
            && m_Weapon
            && WeaponOrder(m_CurrMotion.Motion, m_CurrMotion.Direction, m_CurrMotion.Frame) == 1){
        fnDrawWeapon(false);
    }

    g_SDLDevice->DrawTexture(pFrame0, X() * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX, Y() * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY);

    if(true
            && m_Weapon
            && WeaponOrder(m_CurrMotion.Motion, m_CurrMotion.Direction, m_CurrMotion.Frame) == 0){
        fnDrawWeapon(false);
    }

    // draw HP bar
    // if current m_HPMqx is zero we draw full bar
    switch(m_CurrMotion.Motion){
        case MOTION_DIE:
            {
                break;
            }
        default:
            {
                extern PNGTexDBN *g_ProgUseDBN;
                auto pBar0 = g_ProgUseDBN->Retrieve(0X00000014);
                auto pBar1 = g_ProgUseDBN->Retrieve(0X00000015);

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
    }
    return true;
}

bool Hero::Update()
{
    auto fnGetUpdateDelay = [](int nSpeed, double fStandDelay) -> double
    {
        nSpeed = std::max<int>(SYS_MINSPEED, nSpeed);
        nSpeed = std::min<int>(SYS_MAXSPEED, nSpeed);

        return fStandDelay * 100.0 / nSpeed;
    };

    double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > fnGetUpdateDelay(m_CurrMotion.Speed, m_UpdateDelay) + m_LastUpdateTime){
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
            case MOTION_UNDERATTACK:
                {
                    if(m_MotionQueue.empty()){
                        return UpdateGeneralMotion(false);
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

bool Hero::MotionValid(const MotionNode &rstMotion)
{
    if(true
            && rstMotion.Motion > MOTION_NONE
            && rstMotion.Motion < MOTION_MAX

            && rstMotion.Direction > DIR_NONE
            && rstMotion.Direction < DIR_MAX 

            && m_ProcessRun
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.X,    rstMotion.Y)
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.EndX, rstMotion.EndY)

            && rstMotion.Speed > SYS_MINSPEED
            && rstMotion.Speed < SYS_MAXSPEED

            && rstMotion.Frame >= 0
            && rstMotion.Frame <  MotionFrameCount(rstMotion.Motion, rstMotion.Direction)){

        auto nLDistance2 = LDistance2(rstMotion.X, rstMotion.Y, rstMotion.EndX, rstMotion.EndY);
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
                    return nLDistance2 == 0;
                }
            case MOTION_WALK:
                {
                    return !m_OnHorse && (nLDistance2 == 1 || nLDistance2 == 2);
                }
            case MOTION_HORSEWALK:
                {
                    return  m_OnHorse && (nLDistance2 == 1 || nLDistance2 == 2);
                }
            case MOTION_RUN:
                {
                    return !m_OnHorse && (nLDistance2 == 4 || nLDistance2 == 8);
                }
            case MOTION_HORSERUN:
                {
                    return  m_OnHorse && (nLDistance2 == 9 || nLDistance2 == 18);
                }
            default:
                {
                    return false;
                }
        }
    }else{ return false; }
}

bool Hero::ParseNewAction(const ActionNode &rstAction, bool bRemote)
{
    // currently we ignore the local or remote flag for Hero
    // but we have to support it if we support ACTION_PUSH which changes hero's location

    if(ActionValid(rstAction, bRemote)){

        // 1. prepare before parsing action
        //    additional movement added if necessary but in rush
        switch(rstAction.Action){
            case ACTION_STAND:
            case ACTION_MOVE:
            case ACTION_ATTACK:
                {
                    // 1. clean all pending motions
                    m_MotionQueue.clear();

                    // 2. move to the proper place
                    //    ParseMovePath() will find a valid path and check creatures, means
                    //    1. all nodes are valid grid
                    //    2. prefer path without creatures on the way

                    switch(LDistance2(m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y)){
                        case 0:
                            {
                                break;
                            }
                        default:
                            {
                                auto stPathNodeV = ParseMovePath(m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y, true, true);
                                switch(stPathNodeV.size()){
                                    case 0:
                                    case 1:
                                        {
                                            // 0 means error
                                            // 1 means can't find a path here since we know LDistance2 != 0
                                            return false;
                                        }
                                    default:
                                        {
                                            // we get a path
                                            // make a motion list for the path

                                            for(size_t nIndex = 1; nIndex < stPathNodeV.size(); ++nIndex){
                                                auto nX0 = stPathNodeV[nIndex - 1].X;
                                                auto nY0 = stPathNodeV[nIndex - 1].Y;
                                                auto nX1 = stPathNodeV[nIndex    ].X;
                                                auto nY1 = stPathNodeV[nIndex    ].Y;

                                                if(auto stMotionNode = MakeMotionWalk(nX0, nY0, nX1, nY1, 200)){
                                                    m_MotionQueue.push_back(stMotionNode);
                                                }else{
                                                    m_MotionQueue.clear();
                                                    return false;
                                                }
                                            }
                                            break;
                                        }
                                }
                                break;
                            }
                    }
                    break;
                }
            case ACTION_UNDERATTACK:
            default:
                {
                    break;
                }
        }

        // 2. parse the action
        switch(rstAction.Action){
            case ACTION_STAND:
                {
                    m_MotionQueue.push_back({MOTION_STAND, 0, rstAction.Direction, rstAction.X, rstAction.Y});
                    break;
                }
            case ACTION_MOVE:
                {
                    if(auto stMotionNode = MakeMotionWalk(rstAction.X, rstAction.Y, rstAction.AimX, rstAction.AimY, rstAction.Speed)){
                        m_MotionQueue.push_back(stMotionNode);
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
                    m_MotionQueue.push_front({MOTION_UNDERATTACK, 0, m_CurrMotion.Direction, m_CurrMotion.EndX, m_CurrMotion.EndY});
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

        // 3. after action parse
        //    verify the whole motion queue
        return MotionQueueValid();
    }

    // if action is not valid
    // we ignore it and won't clean the pending motion queue
    return false;
}

bool Hero::Location(int *pX, int *pY)
{
    if(true
            && (m_CurrMotion.Motion > MOTION_NONE)
            && (m_CurrMotion.Motion < MOTION_MAX )){

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

                    switch(auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction)){
                        case 6:
                            {
                                if(pX){ *pX = (nFrame < (nFrameCount - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 0))) ? nX0 : nX1; }
                                if(pY){ *pY = (nFrame < (nFrameCount - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 0))) ? nY0 : nY1; }

                                return true;
                            }
                        default:
                            {
                                return false;
                            }
                    }
                }
            default:
                {
                    if(pX){ *pX = m_CurrMotion.X; }
                    if(pY){ *pY = m_CurrMotion.Y; }

                    return true;
                }
        }
    }

    return false;
}

bool Hero::ActionValid(const ActionNode &rstAction, bool bRemote)
{
    // action check should be much looser than motion check
    // since we don't intend to make continuous action list for parsing
    auto fnCheckDir = [](int nDirection) -> bool
    {
        return (nDirection > DIR_NONE) && (nDirection < DIR_MAX);
    };
    
    if(true
            && rstAction.Action > ACTION_NONE
            && rstAction.Action < ACTION_MAX ){

        switch(rstAction.Action){
            case ACTION_MOVE:
                {
                    // for the move action, we allow the move to an invalid location
                    // but this location should be on current map

                    if(rstAction.MapID != m_ProcessRun->MapID()){
                        return false;
                    }else{
                        if(m_ProcessRun->OnMap(rstAction.MapID, rstAction.X, rstAction.Y)){
                            return true;
                        }else{
                            if(bRemote){
                                return false;
                            }else{
                                return true;
                            }
                        }
                    }
                }
            case ACTION_ATTACK:
                {
                    // attack a target on current map
                    // allow to attack an invalid location if it's magic attack

                    if(rstAction.MapID != m_ProcessRun->MapID()){
                        return false;
                    }else{
                        if(m_ProcessRun->OnMap(rstAction.MapID, rstAction.X, rstAction.Y)){
                            return true;
                        }else{
                            switch(rstAction.ActionParam){
                                case DC_PHY_PLAIN:
                                    {
                                        return false;
                                    }
                                default:
                                    {
                                        return true;
                                    }
                            }
                        }
                    }
                }
            case ACTION_STAND:
            case ACTION_UNDERATTACK:
            case ACTION_DIE:
            default:
                {
                    return true
                        && m_ProcessRun->OnMap(rstAction.MapID, rstAction.X, rstAction.Y)
                        && fnCheckDir(rstAction.Direction);
                }
        }
    }

    return false;
}

int Hero::MotionFrameCount(int nMotion, int nDirection) const
{
    if(true
            && (nMotion > MOTION_NONE)
            && (nMotion < MOTION_MAX )

            && (nDirection > DIR_NONE)
            && (nDirection < DIR_MAX)){
        switch(nMotion){
            case MOTION_STAND       : { return  4; }
            case MOTION_WALK        : { return  6; }
            case MOTION_RUN         : { return  6; }
            case MOTION_ATTACK      : { return  6; }
            case MOTION_UNDERATTACK : { return  3; }
            default                 : { return -1; }
        }
    }else{ return -1; }
}

bool Hero::Moving()
{
    return false
        || m_CurrMotion.Motion == MOTION_RUN
        || m_CurrMotion.Motion == MOTION_WALK
        || m_CurrMotion.Motion == MOTION_HORSERUN
        || m_CurrMotion.Motion == MOTION_HORSEWALK;
}

bool Hero::CanFocus(int, int)
{
    return true;
}

int Hero::WeaponOrder(int nMotion, int nDirection, int nFrame)
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
    // I need to convert nMotion -> nGfxMotionID before get the table item

    const static int s_WeaponOrder[2640]
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

    auto nGfxMotionID = GfxMotionID(nMotion);
    if(nGfxMotionID >= 0){
        return s_WeaponOrder[nGfxMotionID * 80 + (nDirection - (DIR_NONE + 1)) * 10 + nFrame];
    }else{ return -1; }
}

MotionNode Hero::MakeMotionWalk(int nX0, int nY0, int nX1, int nY1, int nSpeed)
{
    if(true
            && m_ProcessRun
            && m_ProcessRun->CanMove(false, nX0, nY0)
            && m_ProcessRun->CanMove(false, nX1, nY1)

            && nSpeed >= SYS_MINSPEED
            && nSpeed <= SYS_MAXSPEED){

        static const int nDirV[][3] = {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

        int nSDX = 1 + (nX1 > nX0) - (nX1 < nX0);
        int nSDY = 1 + (nY1 > nY0) - (nY1 < nY0);

        int nMotion = MOTION_NONE;
        switch(LDistance2(nX0, nY0, nX1, nY1)){
            case 1:
            case 2:
                {
                    nMotion = (m_OnHorse ? MOTION_HORSEWALK : MOTION_WALK);
                    break;
                }
            case 4:
            case 8:
                {
                    nMotion = MOTION_RUN;
                    break;
                }
            case  9:
            case 18:
                {
                    nMotion = MOTION_HORSERUN;
                    break;
                }
            default:
                {
                    return {};
                }
        }

        return {nMotion, 0, nDirV[nSDY][nSDX], nSpeed, nX0, nY0, nX1, nY1};
    }

    return {};
}

int Hero::GfxMotionID(int nMotion)
{
    if((nMotion > MOTION_NONE) && (nMotion < MOTION_MAX)){
        static const std::unordered_map<char, int> stGfxMotionIDRecord = {
            {(char)(MOTION_STAND                ),      0},
            {(char)(MOTION_WALK                 ),     21},
            {(char)(MOTION_RUN                  ),     22},
            {(char)(MOTION_ATTACK               ),      9},

            {(char)(MOTION_CASTMAGIC0           ),      2},
            {(char)(MOTION_CASTMAGIC1           ),      3},

            {(char)(MOTION_BEFOREATTACK         ),      7},
            {(char)(MOTION_UNDERATTACK          ),     15},
            {(char)(MOTION_DIE                  ),      7},

            {(char)(MOTION_ONEHANDATTACK0       ),      8},
            {(char)(MOTION_ONEHANDATTACK1       ),      9},
            {(char)(MOTION_ONEHANDATTACK2       ),     10},
            {(char)(MOTION_ONEHANDATTACK3       ),     11},

            {(char)(MOTION_TWOHANDATTACK0       ),     12},
            {(char)(MOTION_TWOHANDATTACK1       ),     13},
            {(char)(MOTION_TWOHANDATTACK2       ),     14},
            {(char)(MOTION_TWOHANDATTACK3       ),     15}};
        return (stGfxMotionIDRecord.find((char)(nMotion)) != stGfxMotionIDRecord.end()) ? stGfxMotionIDRecord.at((char)(nMotion)) : -1;
    }else{ return -1; }
}

int Hero::GfxWeaponID(int nWeapon, int nMotion, int nDirection)
{
    static_assert(sizeof(int) > 2, "GfxWeaponID() overflows because of sizeof(int) too small");
    if(true
            && (nWeapon    > WEAPON_NONE && nWeapon    < WEAPON_MAX )
            && (nMotion    > MOTION_NONE && nMotion    < MOTION_MAX )
            && (nDirection > DIR_NONE    && nDirection < DIR_MAX    )){
        auto nGfxMotionID = GfxMotionID(nMotion);
        if(nGfxMotionID >= 0){
            return ((nWeapon - (WEAPON_NONE + 1)) << 9) + (nGfxMotionID << 3) + (nDirection - (DIR_NONE + 1));
        }
    }
    return -1;
}

int Hero::GfxDressID(int nDress, int nMotion, int nDirection)
{
    static_assert(sizeof(int) > 2, "GfxDressID() overflows because of sizeof(int) too small");
    if(true
            && (nDress     > DRESS_NONE  && nDress     < DRESS_MAX  )
            && (nMotion    > MOTION_NONE && nMotion    < MOTION_MAX )
            && (nDirection > DIR_NONE    && nDirection < DIR_MAX    )){
        auto nGfxMotionID = GfxMotionID(nMotion);
        if(nGfxMotionID >= 0){
            return ((nDress - (DRESS_NONE + 1)) << 9) + (nGfxMotionID << 3) + (nDirection - (DIR_NONE + 1));
        }
    }
    return -1;
}

int Hero::MaxStep() const
{
    return m_OnHorse ? 3 : 2;
}
