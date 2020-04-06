/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57
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
#include "toll.hpp"
#include "dbcomid.hpp"
#include "monster.hpp"
#include "uidfunc.hpp"
#include "mathfunc.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"
#include "clientpathfinder.hpp"

extern Log *g_Log;
extern PNGTexDB *g_ProgUseDB;
extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_MonsterDB;
extern ClientArgParser *g_clientArgParser;

Monster::Monster(uint64_t nUID, ProcessRun *pRun)
    : Creature(nUID, pRun)
{
    condcheck(nUID);
    condcheck(pRun);

    if(g_clientArgParser->enableDrawUID){
        m_nameBoard.setText("%s(%llu)", DBCOM_MONSTERRECORD(MonsterID()).Name, to_LLU(UID()));
    }
    else{
        m_nameBoard.setText("%s", DBCOM_MONSTERRECORD(MonsterID()).Name);
    }
}

bool Monster::Update(double fUpdateTime)
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
        switch(m_CurrMotion.Motion){
            case MOTION_MON_STAND:
                {
                    if(StayIdle()){
                        return AdvanceMotionFrame(1);
                    }
                    else{
                        // move to next motion will reset frame as 0
                        // if current there is no more motion pending
                        // it will add a MOTION_MON_STAND
                        //
                        // we don't want to reset the frame here
                        return MoveNextMotion();
                    }
                }
            case MOTION_MON_DIE:
                {
                    const auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
                    if(nFrameCount > 0){
                        if((m_CurrMotion.Frame + 1) == nFrameCount){
                            switch(m_CurrMotion.FadeOut){
                                case 0:
                                    {
                                        break;
                                    }
                                case 255:
                                    {
                                        // deactivated if FadeOut reach 255
                                        // next update will auotmatically delete it
                                        break;
                                    }
                                default:
                                    {
                                        auto nNextFadeOut = 0;
                                        nNextFadeOut = (std::max<int>)(1, m_CurrMotion.FadeOut + 10);
                                        nNextFadeOut = (std::min<int>)(nNextFadeOut, 255);

                                        m_CurrMotion.FadeOut = nNextFadeOut;
                                        break;
                                    }
                            }
                            return true;
                        }else{
                            return AdvanceMotionFrame(1);
                        }
                    }
                    return false;
                }
            default:
                {
                    return UpdateMotion(false);
                }
        }
    }
    return true;
}

bool Monster::Draw(int nViewX, int nViewY, int nFocusMask)
{
    // monster graphics retrieving key structure
    //
    //   3322 2222 2222 1111 1111 1100 0000 0000
    //   1098 7654 3210 9876 5432 1098 7654 3210
    //   ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^
    //   |||| |||| |||| |||| |||| |||| |||| ||||
    //             |||| |||| |||| |||| |||+-++++-----------     frame : max =   32
    //             |||| |||| |||| |||| +++----------------- direction : max =    8 -+
    //             |||| |||| |||| ++++---------------------    motion : max =   16 -+
    //             |+++-++++-++++--------------------------      look : max = 2048 -+------> GfxID
    //             +---------------------------------------    shadow : max =    2
    //

    auto nGfxID = GfxID(m_CurrMotion.Motion, m_CurrMotion.Direction);
    if(nGfxID >= 0){
        uint32_t nKey0 = ((uint32_t)(0) << 23) + ((uint32_t)(nGfxID & 0X03FFFF) << 5) + m_CurrMotion.Frame; // body
        uint32_t nKey1 = ((uint32_t)(1) << 23) + ((uint32_t)(nGfxID & 0X03FFFF) << 5) + m_CurrMotion.Frame; // shadow

        int nDX0 = 0;
        int nDY0 = 0;
        int nDX1 = 0;
        int nDY1 = 0;

        auto pFrame0 = g_MonsterDB->Retrieve(nKey0, &nDX0, &nDY0);
        auto pFrame1 = g_MonsterDB->Retrieve(nKey1, &nDX1, &nDY1);

        int nShiftX = 0;
        int nShiftY = 0;
        GetShift(&nShiftX, &nShiftY);

        // always reset the alpha mode for each texture because texture is shared
        // one texture to draw can be configured with different alpha mode for other creatures
        if(pFrame0){ SDL_SetTextureAlphaMod(pFrame0, 255); }
        if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }

        if(true
                && (m_CurrMotion.Motion  == MOTION_MON_DIE)
                && (m_CurrMotion.FadeOut  > 0             )){
            // FadeOut :    0 : normal
            //         : 1-255: fadeOut
            if(pFrame0){ SDL_SetTextureAlphaMod(pFrame0, (255 - m_CurrMotion.FadeOut) / 1); }
            if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, (255 - m_CurrMotion.FadeOut) / 2); }
        }

        auto fnBlendFrame = [](SDL_Texture *pTexture, int nFocusChan, int nX, int nY)
        {
            if(true
                    && pTexture
                    && nFocusChan >= 0
                    && nFocusChan <  FOCUS_MAX){

                // if provided channel as 0
                // just blend it using the original color

                auto stColor = FocusColor(nFocusChan);
                if(!SDL_SetTextureColorMod(pTexture, stColor.r, stColor.g, stColor.b)){
                    g_SDLDevice->DrawTexture(pTexture, nX, nY);
                }
            }
        };

        int nBlendX0 = X() * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX;
        int nBlendY0 = Y() * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY;
        int nBlendX1 = X() * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX;
        int nBlendY1 = Y() * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY;

        fnBlendFrame(pFrame1, 0, nBlendX1, nBlendY1);
        fnBlendFrame(pFrame0, 0, nBlendX0, nBlendY0);

        for(int nFocusChan = 1; nFocusChan < FOCUS_MAX; ++nFocusChan){
            if(nFocusMask & (1 << nFocusChan)){
                fnBlendFrame(pFrame0, nFocusChan, nBlendX0, nBlendY0);
            }
        }

        // draw attached magics
        for(auto pMagic: m_AttachMagicList){
            pMagic->Draw(X() * SYS_MAPGRIDXP - nViewX + nShiftX, Y() * SYS_MAPGRIDYP - nViewY + nShiftY);
        }

        // draw HP bar
        // if current m_HPMqx is zero we draw full bar
        if(m_CurrMotion.Motion != MOTION_MON_DIE){
            auto pBar0 = g_ProgUseDB->Retrieve(0X00000014);
            auto pBar1 = g_ProgUseDB->Retrieve(0X00000015);

            int nBarW = -1;
            int nBarH = -1;
            SDL_QueryTexture(pBar1, nullptr, nullptr, &nBarW, &nBarH);

            const int nDrawBarXP = X() * SYS_MAPGRIDXP - nViewX + nShiftX +  7;
            const int nDrawBarYP = Y() * SYS_MAPGRIDYP - nViewY + nShiftY - 53;

            g_SDLDevice->DrawTexture(pBar1,
                    nDrawBarXP,
                    nDrawBarYP,
                    0,
                    0,
                    (int)(std::lround(nBarW * (m_HPMax ? (std::min<double>)(1.0, (1.0 * m_HP) / m_HPMax) : 1.0))),
                    nBarH);

            g_SDLDevice->DrawTexture(pBar0, nDrawBarXP, nDrawBarYP);

            if(g_clientArgParser->alwaysDrawName || (nFocusMask & (1 << FOCUS_MOUSE))){
                const int nLW = m_nameBoard.W();
                const int nLH = m_nameBoard.H();
                const int nDrawNameXP = nDrawBarXP + nBarW / 2 - nLW / 2;
                const int nDrawNameYP = nDrawBarYP + 20;
                m_nameBoard.drawEx(nDrawNameXP, nDrawNameYP, 0, 0, nLW, nLH);
            }
        }
    }
    return false;
}

int Monster::MotionFrameCount(int nMotion, int nDirection) const
{
    auto nGfxID = GfxID(nMotion, nDirection);
    if(nGfxID >= 0){

        switch(nMotion){
            case MOTION_MON_STAND:
                {
                    return 4;
                }
            case MOTION_MON_WALK:
                {
                    return 6;
                }
            case MOTION_MON_ATTACK0:
                {
                    return 6;
                }
            case MOTION_MON_HITTED:
                {
                    switch(LookID()){
                        default:
                            {
                                return 2;
                            }
                    }
                }
            case MOTION_MON_DIE:
                {
                    switch(LookID()){
                        default:
                            {
                                return 10;
                            }
                    }
                }
            case MOTION_MON_ATTACK1:
                {
                    return 6;
                }
            case MOTION_MON_SPELL0:
            case MOTION_MON_SPELL1:
                {
                    return 10;
                }
            case MOTION_MON_APPEAR:
                {
                    switch(LookID()){
                        default:
                            {
                                return 10;
                            }
                    }
                }
            case MOTION_MON_SPECIAL:
                {
                    switch(LookID()){
                        default:
                            {
                                return 6;
                            }
                    }
                }
            default:
                {
                    break;
                }
        }
    }

    return -1;
}

bool Monster::ParseAction(const ActionNode &rstAction)
{
    m_LastActive = SDL_GetTicks();

    // try find pending motion die
    // ignore all the following actions till the MOTION_MON_DIE done

    for(const auto &m: m_forceMotionQueue){
        if(m.Motion == MOTION_MON_DIE){
            return true;
        }
    }

    for(const auto &m: m_MotionQueue){
        if(m.Motion == MOTION_MON_DIE){
            throw fflerror("Found MOTION_MON_DIE in pending motion queue");
        }
    }

    // make current motion super-fast
    // can presents those ignored actions, helpful for debug
    m_CurrMotion.Speed = SYS_MAXSPEED;
    m_MotionQueue.clear();

    const int endX = m_forceMotionQueue.empty() ? m_CurrMotion.EndX : m_forceMotionQueue.back().EndX;
    const int endY = m_forceMotionQueue.empty() ? m_CurrMotion.EndY : m_forceMotionQueue.back().EndY;

    // 1. prepare before parsing action
    //    additional movement added if necessary but in rush
    switch(rstAction.Action){
        case ACTION_STAND:
        case ACTION_MOVE:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                m_MotionQueue = MakeMotionWalkQueue(endX, endY, rstAction.X, rstAction.Y, SYS_MAXSPEED);
                break;
            }
        case ACTION_DIE:
            {
                const auto motionQueue = MakeMotionWalkQueue(endX, endY, rstAction.X, rstAction.Y, SYS_MAXSPEED);
                m_forceMotionQueue.insert(m_forceMotionQueue.end(), motionQueue.begin(), motionQueue.end());
                break;
            }
        default:
            {
                break;
            }
    }

    // 2. parse the action
    switch(rstAction.Action){
        case ACTION_DIE:
            {
                m_forceMotionQueue.emplace_back(MOTION_MON_DIE, 0, rstAction.Direction, rstAction.X, rstAction.Y);
                m_forceMotionQueue.back().FadeOut = rstAction.ActionParam;
                break;
            }
        case ACTION_STAND:
            {
                m_MotionQueue.emplace_back(MOTION_MON_STAND, 0, rstAction.Direction, rstAction.X, rstAction.Y);
                break;
            }
        case ACTION_HITTED:
            {
                m_MotionQueue.emplace_back(MOTION_MON_HITTED, 0, rstAction.Direction, rstAction.X, rstAction.Y);
                break;
            }
        case ACTION_MOVE:
            {
                if(auto stMotionNode = MakeMotionWalk(rstAction.X, rstAction.Y, rstAction.AimX, rstAction.AimY, rstAction.Speed)){
                    m_MotionQueue.push_back(stMotionNode);
                }
                break;
            }
        case ACTION_SPACEMOVE2:
            {
                m_CurrMotion = MotionNode
                {
                    MOTION_MON_STAND,
                    0,
                    m_CurrMotion.Direction,
                    rstAction.X,
                    rstAction.Y,
                };

                AddAttachMagic(DBCOM_MAGICID(u8"瞬息移动"), 0, EGS_DONE);
                break;
            }
        case ACTION_ATTACK:
            {
                if(auto pCreature = m_ProcessRun->RetrieveUID(rstAction.AimUID)){
                    auto nX   = pCreature->X();
                    auto nY   = pCreature->Y();
                    auto nDir = PathFind::GetDirection(rstAction.X, rstAction.Y, nX, nY);

                    if(nDir > DIR_NONE && nDir < DIR_MAX){
                        m_MotionQueue.emplace_back(MOTION_MON_ATTACK0, 0, nDir, rstAction.X, rstAction.Y);
                    }
                }else{
                    return false;
                }
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

bool Monster::Location(int *pX, int *pY)
{
    if(MotionValid(m_CurrMotion)){
        switch(m_CurrMotion.Motion){
            case MOTION_MON_WALK:
                {
                    auto nX0        = m_CurrMotion.X;
                    auto nY0        = m_CurrMotion.Y;
                    auto nX1        = m_CurrMotion.EndX;
                    auto nY1        = m_CurrMotion.EndY;
                    auto nFrame     = m_CurrMotion.Frame;
                    auto nDirection = m_CurrMotion.Direction;

                    switch(auto nFrameCount = MotionFrameCount(MOTION_MON_WALK, m_CurrMotion.Direction)){
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

int Monster::GfxID(int nMotion, int nDirection) const
{
    // see Monster::Draw() for format of nGfxID
    // monster GfxID consists of (LookID, Motion, Direction)
    static_assert(sizeof(int) > 2, "Monster::GfxID() overflows because of sizeof(int) <= 2");

    if(MonsterID()){
        auto nLookID      = LookID();
        auto nGfxMotionID = GfxMotionID(nMotion);

        if(true
                && nLookID >= LID_MIN
                && nLookID <  LID_MAX

                && nDirection > DIR_NONE
                && nDirection < DIR_MAX

                && nGfxMotionID >= 0){

            // if passed listed simple test
            // we need to check the huge table for it

            return (((nLookID - LID_MIN) & 0X07FF) << 7) + ((nGfxMotionID & 0X000F) << 3) + ((nDirection - (DIR_NONE + 1)) & 0X0007);
        }
    }
    return -1;
}

bool Monster::MotionValid(const MotionNode &rstMotion) const
{
    if(true
            && rstMotion.Motion > MOTION_MON_NONE
            && rstMotion.Motion < MOTION_MON_MAX

            && rstMotion.Direction > DIR_NONE
            && rstMotion.Direction < DIR_MAX 

            && m_ProcessRun
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.X,    rstMotion.Y)
            && m_ProcessRun->OnMap(m_ProcessRun->MapID(), rstMotion.EndX, rstMotion.EndY)

            && rstMotion.Speed >= SYS_MINSPEED
            && rstMotion.Speed <= SYS_MAXSPEED

            && rstMotion.Frame >= 0
            && rstMotion.Frame <  MotionFrameCount(rstMotion.Motion, rstMotion.Direction)){

        auto nLDistance2 = MathFunc::LDistance2(rstMotion.X, rstMotion.Y, rstMotion.EndX, rstMotion.EndY);
        switch(rstMotion.Motion){
            case MOTION_MON_STAND:
                {
                    return nLDistance2 == 0;
                }
            case MOTION_MON_WALK:
                {
                    return false
                        || nLDistance2 == 1
                        || nLDistance2 == 2
                        || nLDistance2 == 1 * MaxStep() * MaxStep()
                        || nLDistance2 == 2 * MaxStep() * MaxStep();
                }
            case MOTION_MON_ATTACK0:
            case MOTION_MON_HITTED:
            case MOTION_MON_DIE:
                {
                    return nLDistance2 == 0;
                }
            default:
                {
                    break;
                }
        }
    }

    return false;
}

bool Monster::canFocus(int nPointX, int nPointY)
{
    switch(m_CurrMotion.Motion){
        case MOTION_MON_DIE:
            {
                return false;
            }
        default:
            {
                break;
            }
    }

    auto nGfxID = GfxID(m_CurrMotion.Motion, m_CurrMotion.Direction);
    if(nGfxID >= 0){

        // we only check the body frame
        // can focus or not is decided by the graphics size

        uint32_t nKey0 = ((uint32_t)(nGfxID & 0X03FFFF) << 5) + m_CurrMotion.Frame;

        int nDX0 = 0;
        int nDY0 = 0;

        auto pFrame0 = g_MonsterDB->Retrieve(nKey0, &nDX0, &nDY0);

        int nShiftX = 0;
        int nShiftY = 0;
        GetShift(&nShiftX, &nShiftY);

        int nStartX = X() * SYS_MAPGRIDXP + nDX0 + nShiftX;
        int nStartY = Y() * SYS_MAPGRIDYP + nDY0 + nShiftY;

        int nW = 0;
        int nH = 0;
        SDL_QueryTexture(pFrame0, nullptr, nullptr, &nW, &nH);

        int nMaxTargetW = SYS_MAPGRIDXP + SYS_TARGETRGN_GAPX;
        int nMaxTargetH = SYS_MAPGRIDYP + SYS_TARGETRGN_GAPY;

        return ((nW >= nMaxTargetW) ? MathFunc::PointInSegment(nPointX, (nStartX + (nW - nMaxTargetW) / 2), nMaxTargetW) : MathFunc::PointInSegment(nPointX, nStartX, nW))
            && ((nH >= nMaxTargetH) ? MathFunc::PointInSegment(nPointY, (nStartY + (nH - nMaxTargetH) / 2), nMaxTargetH) : MathFunc::PointInSegment(nPointY, nStartY, nH));
    }

    return false;
}

Monster *Monster::CreateMonster(uint64_t nUID, ProcessRun *pRun, const ActionNode &rstAction)
{
    if(UIDFunc::GetUIDType(nUID) != UID_MON){
        throw fflerror("Invalid UID provided for monster type: UIDName = %s", UIDFunc::GetUIDString(nUID).c_str());
    }

    Monster *pMonster = nullptr;
    try{
        pMonster = new Monster(nUID, pRun);
    }
    catch(...){
        throw fflerror("Create monster failed: UIDName = %s", UIDFunc::GetUIDString(nUID).c_str());
    }

    // setup the initial motion
    // this motion may never be present since we immediately call ParseAction()
    switch(pMonster->MonsterID()){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                pMonster->m_CurrMotion = {MOTION_MON_STAND, 0, DIR_DOWNLEFT, rstAction.X, rstAction.Y};
                break;
            }
        default:
            {
                pMonster->m_CurrMotion = {MOTION_MON_STAND, 0, DIR_UP, rstAction.X, rstAction.Y};
                break;
            }
    }

    if(pMonster->ParseAction(rstAction)){
        return pMonster;
    }

    delete pMonster;
    return nullptr;
}

MotionNode Monster::MakeMotionWalk(int nX0, int nY0, int nX1, int nY1, int nSpeed) const
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

        auto nLDistance2 = MathFunc::LDistance2(nX0, nY0, nX1, nY1);
        if(false
                || nLDistance2 == 1
                || nLDistance2 == 2
                || nLDistance2 == 1 * MaxStep() * MaxStep()
                || nLDistance2 == 2 * MaxStep() * MaxStep()){

            return {MOTION_MON_WALK, 0, nDirV[nSDY][nSDX], nSpeed, nX0, nY0, nX1, nY1};
        }
    }
    return {};
}

int Monster::MaxStep() const
{
    return 1;
}

int Monster::CurrStep() const
{
    return 1;
}

int Monster::LookID() const
{
    if(auto &rstMR = DBCOM_MONSTERRECORD(MonsterID())){
        return rstMR.LookID;
    }
    return -1;
}

int Monster::GfxMotionID(int nMotion) const
{
    return ((nMotion > MOTION_MON_NONE) && (nMotion < MOTION_MON_MAX)) ? (nMotion - (MOTION_MON_NONE + 1)) : -1;
}
