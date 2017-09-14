/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 09/03/2015 03:49:00
 *  Last Modified: 09/14/2017 00:01:16
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
#include "dbcomid.hpp"
#include "mathfunc.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "pngtexdbn.hpp"
#include "processrun.hpp"
#include "motionnode.hpp"
#include "attachmagic.hpp"
#include "dbcomrecord.hpp"
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

bool Hero::Draw(int nViewX, int nViewY, int)
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

    extern PNGTexOffDBN *g_HeroDBN;
    auto pFrame0 = g_HeroDBN->Retrieve(nKey0, &nDX0, &nDY0);
    auto pFrame1 = g_HeroDBN->Retrieve(nKey1, &nDX1, &nDY1);

    int nShiftX = 0;
    int nShiftY = 0;
    GetShift(&nShiftX, &nShiftY);

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

    // draw attached magics
    for(auto pMagic: m_AttachMagicList){
        pMagic->Draw(X() * SYS_MAPGRIDXP - nViewX + nShiftX, Y() * SYS_MAPGRIDYP - nViewY + nShiftY);
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
        auto fTimeDelay  = fTimeNow - m_LastUpdateTime;
        m_LastUpdateTime = fTimeNow;

        // 2. attached magic update
        UpdateAttachMagic(fTimeDelay);

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
            case MOTION_HITTED:
                {
                    if(m_MotionQueue.empty()){
                        return UpdateMotion(false);
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
                    return UpdateMotion(false);
                }
        }
    }
    return true;
}

bool Hero::MotionValid(const MotionNode &rstMotion) const
{
    if(true
            && rstMotion.Motion > MOTION_NONE
            && rstMotion.Motion < MOTION_MAX

            && rstMotion.Direction > DIR_NONE
            && rstMotion.Direction < DIR_MAX 

            && m_ProcessRun
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.X,    rstMotion.Y)
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.EndX, rstMotion.EndY)

            && rstMotion.Speed >= SYS_MINSPEED
            && rstMotion.Speed <= SYS_MAXSPEED

            && rstMotion.Frame >= 0
            && rstMotion.Frame <  MotionFrameCount(rstMotion.Motion, rstMotion.Direction)){

        auto nLDistance2 = LDistance2(rstMotion.X, rstMotion.Y, rstMotion.EndX, rstMotion.EndY);
        switch(rstMotion.Motion){
            case MOTION_STAND:
                {
                    return !OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_SPELL0:
            case MOTION_SPELL1:
                {
                    return !OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_ARROWATTACK:
            case MOTION_HOLD:
            case MOTION_PUSHBACK:
            case MOTION_PUSHBACKFLY:
                {
                    return false;
                }
            case MOTION_ATTACKMODE:
                {
                    return !OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_CUT:
                {
                    return false;
                }
            case MOTION_ONEVSWING:
            case MOTION_TWOVSWING:
            case MOTION_ONEHSWING:
            case MOTION_TWOHSWING:
            case MOTION_SPEARVSWING:
            case MOTION_SPEARHSWING:
            case MOTION_HITTED:
            case MOTION_WHEELWIND:
            case MOTION_RANDSWING:
                {
                    return !OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_BACKDROPKICK:
                {
                    return false;
                }
            case MOTION_DIE:
                {
                    return !OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_ONHORSEDIE:
                {
                    return  OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_WALK:
                {
                    return !OnHorse() && (nLDistance2 == 1 || nLDistance2 == 2);
                }
            case MOTION_RUN:
                {
                    return !OnHorse() && (nLDistance2 == 4 || nLDistance2 == 8);
                }
            case MOTION_MOODEPO:
            case MOTION_ROLL:
            case MOTION_FISHSTAND:
            case MOTION_FISHHAND:
            case MOTION_FISHTHROW:
            case MOTION_FISHPULL:
                {
                    return false;
                }
            case MOTION_ONHORSESTAND:
                {
                    return OnHorse() && (nLDistance2 == 0);
                }
            case MOTION_ONHORSEWALK:
                {
                    return OnHorse() && (nLDistance2 == 1 || nLDistance2 == 2);
                }
            case MOTION_ONHORSERUN:
                {
                    return OnHorse() && (nLDistance2 == 9 || nLDistance2 == 18);
                }
            case MOTION_ONHORSEHITTED:
                {
                    return OnHorse() && (nLDistance2 == 0);
                }
            default:
                {
                    break;
                }
        }
    }

    return false;
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
            case ACTION_SPELL:
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
            case ACTION_SPACEMOVE:
                {
                    m_MotionQueue.clear();
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
                    m_MotionQueue.emplace_back(
                            OnHorse() ? MOTION_ONHORSESTAND : MOTION_STAND,
                            0,
                            rstAction.Direction,
                            rstAction.X,
                            rstAction.Y);
                    break;
                }
            case ACTION_MOVE:
                {
                    if(auto stMotionNode = MakeMotionWalk(rstAction.X, rstAction.Y, rstAction.AimX, rstAction.AimY, rstAction.Speed)){
                        m_MotionQueue.push_back(stMotionNode);
                    }
                    break;
                }
            case ACTION_SPACEMOVE:
                {
                    m_CurrMotion = MotionNode
                    {
                        OnHorse() ? MOTION_ONHORSESTAND : MOTION_STAND,
                        0,
                        m_CurrMotion.Direction,
                        rstAction.X,
                        rstAction.Y,
                    };

                    break;
                }
            case ACTION_SPELL:
                {
                    int nMotionSpell = MOTION_NONE;
                    if(auto &rstMR = DBCOM_MAGICRECORD(rstAction.ActionParam)){
                        if(auto &rstGfxEntry = rstMR.GetGfxEntry(u8"启动")){
                            switch(rstGfxEntry.Motion){
                                case 0  : nMotionSpell = MOTION_SPELL0; break;
                                case 1  : nMotionSpell = MOTION_SPELL1; break;
                                default : nMotionSpell = MOTION_NONE;   break;
                            }

                            if(nMotionSpell != MOTION_NONE){
                                m_MotionQueue.emplace_back(nMotionSpell, 0, rstAction.Direction, rstAction.X, rstAction.Y);
                            }

                            AddAttachMagic(rstAction.ActionParam, 0, rstGfxEntry.Stage);
                        }
                    }
                    break;
                }
            case ACTION_ATTACK:
                {
                    int nMotion = -1;
                    switch(rstAction.ActionParam){
                        default:
                            {
                                nMotion = MOTION_ONEVSWING;
                                break;
                            }
                    }

                    m_MotionQueue.emplace_back(
                            nMotion,
                            0,
                            rstAction.Direction,
                            rstAction.X,
                            rstAction.Y);

                    m_MotionQueue.emplace_back(
                            MOTION_ATTACKMODE,
                            0,
                            rstAction.Direction,
                            rstAction.X,
                            rstAction.Y);
                    break;
                }
            case ACTION_UNDERATTACK:
                {
                    m_MotionQueue.emplace_front(
                            OnHorse() ? MOTION_ONHORSEHITTED : MOTION_HITTED,
                            0,
                            m_CurrMotion.Direction,
                            m_CurrMotion.EndX,
                            m_CurrMotion.EndY);
                    break;
                }
            case ACTION_DIE:
                {
                    m_MotionQueue.emplace_back(
                            OnHorse() ? MOTION_ONHORSEDIE : MOTION_DIE,
                            0,
                            m_CurrMotion.Direction,
                            m_CurrMotion.EndX,
                            m_CurrMotion.EndY);
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
    if(MotionValid(m_CurrMotion)){
        switch(m_CurrMotion.Motion){
            case MOTION_WALK:
            case MOTION_RUN:
            case MOTION_ONHORSEWALK:
            case MOTION_ONHORSERUN:
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

bool Hero::ActionValid(const ActionNode &rstAction, bool bRemote) const
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
            case ACTION_SPACEMOVE:
                {
                    return true;
                }
            case ACTION_STAND:
            case ACTION_SPELL:
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
            case MOTION_STAND		    : return  4;
            case MOTION_ARROWATTACK		: return  6;
            case MOTION_SPELL0		    : return  5;
            case MOTION_SPELL1		    : return  5;
            case MOTION_HOLD		    : return  1;
            case MOTION_PUSHBACK		: return  1;
            case MOTION_PUSHBACKFLY		: return  1;
            case MOTION_ATTACKMODE		: return  3;
            case MOTION_CUT		        : return  2;
            case MOTION_ONEVSWING		: return  6;
            case MOTION_TWOVSWING		: return  6;
            case MOTION_ONEHSWING		: return  6;
            case MOTION_TWOHSWING		: return  6;
            case MOTION_SPEARVSWING     : return  6;
            case MOTION_SPEARHSWING     : return  6;
            case MOTION_HITTED          : return  3;
            case MOTION_WHEELWIND       : return 10;
            case MOTION_RANDSWING       : return 10;
            case MOTION_BACKDROPKICK    : return 10;
            case MOTION_DIE             : return 10;
            case MOTION_ONHORSEDIE      : return 10;
            case MOTION_WALK            : return  6;
            case MOTION_RUN             : return  6;
            case MOTION_MOODEPO         : return  6;
            case MOTION_ROLL            : return 10;
            case MOTION_FISHSTAND       : return  4;
            case MOTION_FISHHAND        : return  3;
            case MOTION_FISHTHROW       : return  8;
            case MOTION_FISHPULL        : return  8;
            case MOTION_ONHORSESTAND    : return  4;
            case MOTION_ONHORSEWALK     : return  6;
            case MOTION_ONHORSERUN      : return  6;
            case MOTION_ONHORSEHITTED   : return  3;
            default                     : return -1;
        }
    }

    return -1;
}

bool Hero::Moving()
{
    return false
        || m_CurrMotion.Motion == MOTION_RUN
        || m_CurrMotion.Motion == MOTION_WALK
        || m_CurrMotion.Motion == MOTION_ONHORSERUN
        || m_CurrMotion.Motion == MOTION_ONHORSEWALK;
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
        #include "weaponorder.inc"
    };

    auto nGfxMotionID = GfxMotionID(nMotion);
    if(nGfxMotionID >= 0){
        return s_WeaponOrder[nGfxMotionID * 80 + (nDirection - (DIR_NONE + 1)) * 10 + nFrame];
    }else{ return -1; }
}

MotionNode Hero::MakeMotionWalk(int nX0, int nY0, int nX1, int nY1, int nSpeed) const
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
                    nMotion = (OnHorse() ? MOTION_ONHORSEWALK : MOTION_WALK);
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
                    nMotion = MOTION_ONHORSERUN;
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

int Hero::GfxMotionID(int nMotion) const
{
    return ((nMotion > MOTION_NONE) && (nMotion < MOTION_MAX)) ? (nMotion - (MOTION_NONE + 1)) : -1;
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
    return OnHorse() ? 3 : 2;
}

int Hero::CurrStep() const
{
    if(MotionValid(m_CurrMotion)){
        switch(m_CurrMotion.Motion){
            case MOTION_WALK:
            case MOTION_ONHORSEWALK:
                {
                    return 1;
                }
            case MOTION_RUN:
                {
                    return 2;
                }
            case MOTION_ONHORSERUN:
                {
                    return 3;
                }
            default:
                {
                    return 0;
                }
        }
    }

    return -1;
}
