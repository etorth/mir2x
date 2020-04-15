/*
 * =====================================================================================
 *
 *       Filename: hero.cpp
 *        Created: 09/03/2015 03:49:00
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
#include "mathf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "motionnode.hpp"
#include "attachmagic.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"

extern Log *g_Log;
extern SDLDevice *g_SDLDevice;
extern PNGTexDB *g_ProgUseDB;
extern PNGTexOffDB *g_HeroDB;
extern PNGTexOffDB *g_WeaponDB;

Hero::Hero(uint64_t nUID, uint32_t nDBID, bool bGender, uint32_t nDress, ProcessRun *pRun, const ActionNode &rstAction)
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
    m_currMotion = {
        MOTION_STAND,
        0,
        DIR_UP,
        rstAction.X,
        rstAction.Y
    };

    if(!ParseAction(rstAction)){
        throw fflerror("construct hero failed");
    }
}

bool Hero::draw(int viewX, int viewY, int)
{
    auto nDress     = m_Dress;
    auto nMotion    = m_currMotion.motion;
    auto nDirection = m_currMotion.direction;

    auto nGfxDressID = GfxDressID(nDress, nMotion, nDirection);
    if(nGfxDressID < 0){
        m_currMotion.print();
        return false;
    }

    // human gfx dress id indexing
    // 04 - 00 :     frame : max =  32
    // 07 - 05 : direction : max =  08 : +
    // 13 - 08 :    motion : max =  64 : +
    // 21 - 14 :     dress : max = 256 : +----> GfxDressID
    //      22 :       sex :
    //      23 :    shadow :
    uint32_t nKey0 = ((uint32_t)(0) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + m_currMotion.frame;
    uint32_t nKey1 = ((uint32_t)(1) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + m_currMotion.frame;

    int nDX0 = 0;
    int nDY0 = 0;
    int nDX1 = 0;
    int nDY1 = 0;

    auto pFrame0 = g_HeroDB->Retrieve(nKey0, &nDX0, &nDY0);
    auto pFrame1 = g_HeroDB->Retrieve(nKey1, &nDX1, &nDY1);

    const auto [shiftX, shiftY] = getShift();
    auto fnDrawWeapon = [this, viewX, viewY, shiftX, shiftY](bool bShadow)
    {
        // 04 - 00 :     frame : max =  32
        // 07 - 05 : direction : max =  08 : +
        // 13 - 08 :    motion : max =  64 : +
        // 21 - 14 :    weapon : max = 256 : +----> GfxWeaponID
        //      22 :    gender :
        //      23 :    shadow :
        auto nGfxWeaponID = GfxWeaponID(m_Weapon, m_currMotion.motion, m_currMotion.direction);
        if(nGfxWeaponID >= 0){
            uint32_t nWeaponKey = (((uint32_t)(bShadow ? 1 : 0)) << 23) + (((uint32_t)(m_Gender ? 1 : 0)) << 22) + ((nGfxWeaponID & 0X01FFFF) << 5) + m_currMotion.frame;

            int nWeaponDX = 0;
            int nWeaponDY = 0;

            auto pFrame = g_WeaponDB->Retrieve(nWeaponKey, &nWeaponDX, &nWeaponDY);

            if(pFrame && bShadow){ SDL_SetTextureAlphaMod(pFrame, 128); }
            g_SDLDevice->DrawTexture(pFrame, X() * SYS_MAPGRIDXP + nWeaponDX - viewX + shiftX, Y() * SYS_MAPGRIDYP + nWeaponDY - viewY + shiftY);
        }
    };

    fnDrawWeapon(true);

    if(pFrame1){
        SDL_SetTextureAlphaMod(pFrame1, 128);
    }
    g_SDLDevice->DrawTexture(pFrame1, X() * SYS_MAPGRIDXP + nDX1 - viewX + shiftX, Y() * SYS_MAPGRIDYP + nDY1 - viewY + shiftY);

    if(true
            && m_Weapon
            && WeaponOrder(m_currMotion.motion, m_currMotion.direction, m_currMotion.frame) == 1){
        fnDrawWeapon(false);
    }

    g_SDLDevice->DrawTexture(pFrame0, X() * SYS_MAPGRIDXP + nDX0 - viewX + shiftX, Y() * SYS_MAPGRIDYP + nDY0 - viewY + shiftY);

    if(true
            && m_Weapon
            && WeaponOrder(m_currMotion.motion, m_currMotion.direction, m_currMotion.frame) == 0){
        fnDrawWeapon(false);
    }

    // draw attached magics
    for(auto pMagic: m_attachMagicList){
        pMagic->Draw(X() * SYS_MAPGRIDXP - viewX + shiftX, Y() * SYS_MAPGRIDYP - viewY + shiftY);
    }

    // draw HP bar
    // if current m_HPMqx is zero we draw full bar
    switch(m_currMotion.motion){
        case MOTION_DIE:
            {
                break;
            }
        default:
            {
                auto pBar0 = g_ProgUseDB->Retrieve(0X00000014);
                auto pBar1 = g_ProgUseDB->Retrieve(0X00000015);

                int nW = -1;
                int nH = -1;
                SDL_QueryTexture(pBar1, nullptr, nullptr, &nW, &nH);
                g_SDLDevice->DrawTexture(pBar1,
                        X() * SYS_MAPGRIDXP - viewX + shiftX +  7,
                        Y() * SYS_MAPGRIDYP - viewY + shiftY - 53,
                        0,
                        0,
                        (int)(std::lround(nW * (m_HPMax ? (std::min<double>)(1.0, (1.0 * m_HP) / m_HPMax) : 1.0))),
                        nH);

                g_SDLDevice->DrawTexture(pBar0,
                        X() * SYS_MAPGRIDXP - viewX + shiftX +  7,
                        Y() * SYS_MAPGRIDYP - viewY + shiftY - 53);
            }
    }
    return true;
}

bool Hero::update(double fUpdateTime)
{
    // 1. independent from time control
    //    attached magic could take different speed
    UpdateAttachMagic(fUpdateTime);

    // 2. update this monster
    //    need fps control for current motion
    const double fTimeNow = SDL_GetTicks() * 1.0;
    if(fTimeNow > CurrMotionDelay() + m_LastUpdateTime){

        // 1. record update time
        //    needed for next update
        m_LastUpdateTime = fTimeNow;

        // 2. do the update
        switch(m_currMotion.motion){
            case MOTION_STAND:
                {
                    if(StayIdle()){
                        return AdvanceMotionFrame(1);
                    }
                    else{
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
                    if(StayIdle()){
                        return UpdateMotion(false);
                    }
                    else{
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

bool Hero::motionValid(const MotionNode &rstMotion) const
{
    if(true
            && rstMotion.motion > MOTION_NONE
            && rstMotion.motion < MOTION_MAX

            && rstMotion.direction > DIR_NONE
            && rstMotion.direction < DIR_MAX 

            && m_ProcessRun
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.x,    rstMotion.y)
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.endX, rstMotion.endY)

            && rstMotion.speed >= SYS_MINSPEED
            && rstMotion.speed <= SYS_MAXSPEED

            && rstMotion.frame >= 0
            && rstMotion.frame <  motionFrameCount(rstMotion.motion, rstMotion.direction)){

        auto nLDistance2 = mathf::LDistance2(rstMotion.x, rstMotion.y, rstMotion.endX, rstMotion.endY);
        switch(rstMotion.motion){
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

bool Hero::ParseAction(const ActionNode &rstAction)
{
    m_LastActive = SDL_GetTicks();

    m_currMotion.speed = SYS_MAXSPEED;
    m_motionQueue.clear();

    const int endX   = m_forceMotionQueue.empty() ? m_currMotion.endX      : m_forceMotionQueue.back().endX;
    const int endY   = m_forceMotionQueue.empty() ? m_currMotion.endY      : m_forceMotionQueue.back().endY;
    const int endDir = m_forceMotionQueue.empty() ? m_currMotion.direction : m_forceMotionQueue.back().direction;

    // 1. prepare before parsing action
    //    additional movement added if necessary but in rush
    switch(rstAction.Action){
        case ACTION_DIE:
        case ACTION_STAND:
            {
                // take this as cirtical
                // server use ReportStand() to do location sync
                const auto motionQueue = MakeMotionWalkQueue(endX, endY, rstAction.X, rstAction.Y, SYS_MAXSPEED);
                m_forceMotionQueue.insert(m_forceMotionQueue.end(), motionQueue.begin(), motionQueue.end());
                break;
            }
        case ACTION_MOVE:
        case ACTION_SPELL:
        case ACTION_ATTACK:
            {
                m_motionQueue = MakeMotionWalkQueue(endX, endY, rstAction.X, rstAction.Y, SYS_MAXSPEED);
                break;
            }
        case ACTION_SPACEMOVE2:
        case ACTION_PICKUP:
        default:
            {
                break;
            }
    }

    // 2. parse the action
    //    now motion list is at the right grid
    switch(rstAction.Action){
        case ACTION_STAND:
            {
                m_motionQueue.emplace_back(
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
                    m_motionQueue.push_back(stMotionNode);
                }else{
                    return false;
                }
                break;
            }
        case ACTION_SPACEMOVE2:
            {
                m_currMotion = MotionNode
                {
                    OnHorse() ? MOTION_ONHORSESTAND : MOTION_STAND,
                    0,
                    m_currMotion.direction,
                    rstAction.X,
                    rstAction.Y,
                };

                AddAttachMagic(DBCOM_MAGICID(u8"瞬息移动"), 0, EGS_DONE);
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
                            auto fnGetSpellDir = [this](int nX0, int nY0, int nX1, int nY1) -> int
                            {
                                switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
                                    case 0:
                                        {
                                            return m_currMotion.direction;
                                        }
                                    default:
                                        {
                                            return PathFind::GetDirection(nX0, nY0, nX1, nY1);
                                        }
                                }
                            };

                            int nDir = DIR_NONE;
                            if(m_ProcessRun->CanMove(true, 0, rstAction.AimX, rstAction.AimY)){
                                nDir = fnGetSpellDir(rstAction.X, rstAction.Y, rstAction.AimX, rstAction.AimY);
                            }else if(rstAction.AimUID){
                                if(auto pCreature = m_ProcessRun->RetrieveUID(rstAction.AimUID)){
                                    nDir = fnGetSpellDir(rstAction.X, rstAction.Y, pCreature->X(), pCreature->Y());
                                }
                            }

                            if(nDir > DIR_NONE && nDir < DIR_MAX){
                                m_motionQueue.emplace_back(nMotionSpell, 0, nDir, SYS_DEFSPEED, rstAction.X, rstAction.Y);
                                AddAttachMagic(rstAction.ActionParam, 0, rstGfxEntry.Stage);
                            }
                        }
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

                if(auto pCreature = m_ProcessRun->RetrieveUID(rstAction.AimUID)){
                    auto nX   = pCreature->X();
                    auto nY   = pCreature->Y();
                    auto nDir = PathFind::GetDirection(rstAction.X, rstAction.Y, nX, nY);

                    if(nDir > DIR_NONE && nDir < DIR_MAX){
                        m_motionQueue.emplace_back(nMotion,           0, nDir, rstAction.X, rstAction.Y);
                        m_motionQueue.emplace_back(MOTION_ATTACKMODE, 0, nDir, rstAction.X, rstAction.Y);
                    }
                }else{
                    return false;
                }

                break;
            }
        case ACTION_HITTED:
            {
                m_motionQueue.emplace_front(
                        OnHorse() ? MOTION_ONHORSEHITTED : MOTION_HITTED,
                        0,
                        endDir,
                        endX,
                        endY);
                break;
            }
        case ACTION_PICKUP:
            {
                PickUp();
                break;
            }
        case ACTION_DIE:
            {
                m_forceMotionQueue.emplace_back(
                        OnHorse() ? MOTION_ONHORSEDIE : MOTION_DIE,
                        0,
                        rstAction.Direction,
                        rstAction.X,
                        rstAction.Y);
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

std::tuple<int, int> Hero::location() const
{
    if(!motionValid(m_currMotion)){
        throw fflerror("current motion is invalid");
    }

    switch(m_currMotion.motion){
        case MOTION_WALK:
        case MOTION_RUN:
        case MOTION_ONHORSEWALK:
        case MOTION_ONHORSERUN:
            {
                const auto nX0        = m_currMotion.x;
                const auto nY0        = m_currMotion.y;
                const auto nX1        = m_currMotion.endX;
                const auto nY1        = m_currMotion.endY;
                const auto nFrame     = m_currMotion.frame;
                const auto nDirection = m_currMotion.direction;

                switch(auto frameCountMoving = motionFrameCount(m_currMotion.motion, m_currMotion.direction)){
                    case 6:
                        {
                            return
                            {
                                (nFrame < (frameCountMoving - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 0))) ? nX0 : nX1,
                                (nFrame < (frameCountMoving - (((nDirection == DIR_UPLEFT) ? 2 : 5) + 0))) ? nY0 : nY1,
                            };
                        }
                    default:
                        {
                            throw fflerror("moving frame should have frame count 6");
                        }
                }
            }
        default:
            {
                return {m_currMotion.x, m_currMotion.y};
            }
    }
}

int Hero::motionFrameCount(int motion, int direction) const
{
    if(!(motion > MOTION_NONE && motion < MOTION_MAX)){
        return -1;
    }

    if(!(direction > DIR_NONE && direction < DIR_MAX)){
        return -1;
    }

    switch(motion){
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

bool Hero::moving()
{
    return false
        || m_currMotion.motion == MOTION_RUN
        || m_currMotion.motion == MOTION_WALK
        || m_currMotion.motion == MOTION_ONHORSERUN
        || m_currMotion.motion == MOTION_ONHORSEWALK;
}

bool Hero::canFocus(int nPointX, int nPointY) const
{
    switch(currMotion().motion){
        case MOTION_DIE:
            {
                return false;
            }
        default:
            {
                break;
            }
    }

    const auto nDress     = Dress();
    const auto nGender    = Gender();
    const auto nMotion    = currMotion().motion;
    const auto nDirection = currMotion().direction;

    const auto nGfxDressID = GfxDressID(nDress, nMotion, nDirection);
    if(nGfxDressID >= 0){

        int nDX0 = 0;
        int nDY0 = 0;

        uint32_t nKey0 = (((uint32_t)(nGender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + currMotion().frame;

        auto pFrame0 = g_HeroDB->Retrieve(nKey0, &nDX0, &nDY0);
        const auto [shiftX, shiftY] = getShift();

        int nStartX = X() * SYS_MAPGRIDXP + nDX0 + shiftX;
        int nStartY = Y() * SYS_MAPGRIDYP + nDY0 + shiftY;

        int nW = 0;
        int nH = 0;
        SDL_QueryTexture(pFrame0, nullptr, nullptr, &nW, &nH);

        int nMaxTargetW = SYS_MAPGRIDXP + SYS_TARGETRGN_GAPX;
        int nMaxTargetH = SYS_MAPGRIDYP + SYS_TARGETRGN_GAPY;

        return ((nW >= nMaxTargetW) ? mathf::pointInSegment(nPointX, (nStartX + (nW - nMaxTargetW) / 2), nMaxTargetW) : mathf::pointInSegment(nPointX, nStartX, nW))
            && ((nH >= nMaxTargetH) ? mathf::pointInSegment(nPointY, (nStartY + (nH - nMaxTargetH) / 2), nMaxTargetH) : mathf::pointInSegment(nPointY, nStartY, nH));
    }
    return false;
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
            && m_ProcessRun->CanMove(true, 0, nX0, nY0)
            && m_ProcessRun->CanMove(true, 0, nX1, nY1)

            && nSpeed >= SYS_MINSPEED
            && nSpeed <= SYS_MAXSPEED){

        static const int nDirV[][3] = {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

        int nSDX = 1 + (nX1 > nX0) - (nX1 < nX0);
        int nSDY = 1 + (nY1 > nY0) - (nY1 < nY0);

        int nMotion = MOTION_NONE;
        switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
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

int Hero::GfxWeaponID(int nWeapon, int nMotion, int nDirection) const
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

int Hero::GfxDressID(int nDress, int nMotion, int nDirection) const
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
    if(motionValid(m_currMotion)){
        switch(m_currMotion.motion){
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
