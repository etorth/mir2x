/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57
 *  Last Modified: 07/03/2017 00:02:24
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
#include "pngtexoffdbn.hpp"
#include "clientpathfinder.hpp"

Monster::Monster(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun)
    : Creature(nUID, pRun)
    , m_MonsterID(nMonsterID)
{
    assert(nUID);
    assert(nMonsterID);
    assert(pRun);
}

bool Monster::Update()
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
            case MOTION_UNDERATTACK:
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
            case MOTION_DIE:
                {
                    auto nFrameCount = MotionFrameCount(m_CurrMotion.Motion, m_CurrMotion.Direction);
                    if(nFrameCount > 0){
                        if((m_CurrMotion.Frame + 1) == nFrameCount){
                            switch(m_CurrMotion.FadeOut){
                                case 0:
                                    {
                                        break;
                                    }
                                case 255:
                                    {
                                        Deactivate();
                                        break;
                                    }
                                default:
                                    {
                                        auto nNextFadeOut = 0;
                                        nNextFadeOut = std::max<int>(1, m_CurrMotion.FadeOut + 10);
                                        nNextFadeOut = std::min<int>(nNextFadeOut, 255);

                                        m_CurrMotion.MotionParam = nNextFadeOut;
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
                    return UpdateGeneralMotion(false);
                }
        }
    }
    return true;
}

bool Monster::Draw(int nViewX, int nViewY)
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

        extern PNGTexOffDBN *g_PNGTexOffDBN;
        auto pFrame0 = g_PNGTexOffDBN->Retrieve(nKey0, &nDX0, &nDY0);
        auto pFrame1 = g_PNGTexOffDBN->Retrieve(nKey1, &nDX1, &nDY1);

        int nShiftX = 0;
        int nShiftY = 0;
        EstimatePixelShift(&nShiftX, &nShiftY);

        extern SDLDevice *g_SDLDevice;
        if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, 128); }

        if(true
                && (m_CurrMotion.Motion  == MOTION_DIE)
                && (m_CurrMotion.FadeOut  > 0         )){
            // FadeOut :    0 : normal
            //         : 1-255: fadeOut
            if(pFrame0){ SDL_SetTextureAlphaMod(pFrame0, (255 - m_CurrMotion.FadeOut) / 1); }
            if(pFrame1){ SDL_SetTextureAlphaMod(pFrame1, (255 - m_CurrMotion.FadeOut) / 2); }
        }

        // for focus mode
        // we use SDL_BLENDMODE_ADD mode which take sum of dstRGB and srcRGB
        // but we don't need dstRGB, so always blend body frame two times to overwrite dstRGB first
        auto fnBlendFrame = [](SDL_Texture *pTexture, SDL_BlendMode stMode, int nX, int nY) -> void
        {
            if(pTexture){
                // set blend mode is cheap
                // so we always do this setting for blending
                SDL_SetTextureBlendMode(pTexture, stMode);

                extern SDLDevice *g_SDLDevice;
                g_SDLDevice->DrawTexture(pTexture, nX, nY);
            }
        };

        int nBlendX0 = X() * SYS_MAPGRIDXP + nDX0 - nViewX + nShiftX;
        int nBlendY0 = Y() * SYS_MAPGRIDYP + nDY0 - nViewY + nShiftY;
        int nBlendX1 = X() * SYS_MAPGRIDXP + nDX1 - nViewX + nShiftX;
        int nBlendY1 = Y() * SYS_MAPGRIDYP + nDY1 - nViewY + nShiftY;

        if(true   ){ fnBlendFrame(pFrame1, SDL_BLENDMODE_BLEND, nBlendX1, nBlendY1); }
        if(true   ){ fnBlendFrame(pFrame0, SDL_BLENDMODE_BLEND, nBlendX0, nBlendY0); }
        if(Focus()){ fnBlendFrame(pFrame0, SDL_BLENDMODE_ADD,   nBlendX0, nBlendY0); }
        if(Focus()){ fnBlendFrame(pFrame0, SDL_BLENDMODE_ADD,   nBlendX0, nBlendY0); }

        // draw HP bar
        // if current m_HPMqx is zero we draw full bar
        if(m_CurrMotion.Motion != MOTION_DIE){
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
    }
    return false;
}

int Monster::MotionFrameCount(int nMotion, int nDirection) const
{
    auto nGfxID = GfxID(nMotion, nDirection);
    if(nGfxID >= 0){

        switch(MonsterID()){
            default:
                {
                    switch(nMotion){
                        case MOTION_STAND       : return  4;
                        case MOTION_WALK        : return  6;
                        case MOTION_ATTACK      : return  6;
                        case MOTION_UNDERATTACK : return  2;
                        case MOTION_DIE         : return 10;
                    }
                }
        }
    }

    return -1;
}

bool Monster::ParseNewAction(const ActionNode &rstAction, bool)
{
    if(ActionValid(rstAction, true)){

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
                    auto stPathNodeV = ParseMovePath(m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y, true, true);
                    switch(stPathNodeV.size()){
                        case 0:
                        case 1:
                            {
                                // error happens for path finding
                                extern Log *g_Log;
                                g_Log->AddLog(LOGTYPE_WARNING, "Path finding error: (%d, %d) -> (%d, %d)", m_CurrMotion.EndX, m_CurrMotion.EndY, rstAction.X, rstAction.Y);
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

                switch(auto nFrameCount = MotionFrameCount(MOTION_WALK, m_CurrMotion.Direction)){
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

int Monster::GfxID(int nMotion, int nDirection) const
{
    // see Monster::Draw() for format of nGfxID
    // monster GfxID consists of (LookID, Motion, Direction)

    static_assert(sizeof(int) > 2, "Monster::GfxID() overflows because of sizeof(int) <= 2");
    auto fnMotionGfxID = [](int nMotion) -> int
    {
        // general table for motion id
        // when provided motion supported then it should have this id
        static const std::unordered_map<uint8_t, int> stMotionGfxIDRecord
        {
            {(uint8_t)(MOTION_STAND      ), 0},
            {(uint8_t)(MOTION_WALK       ), 1},
            {(uint8_t)(MOTION_ATTACK     ), 2},
            {(uint8_t)(MOTION_UNDERATTACK), 3},
            {(uint8_t)(MOTION_DIE        ), 4}
        };

        if(stMotionGfxIDRecord.find((uint8_t)(nMotion)) == stMotionGfxIDRecord.end()){
            return -1;
        }else{
            return stMotionGfxIDRecord.at((uint8_t)(nMotion));
        }
    };

    auto fnLookID = [](int nMonsterID) -> int
    {
        static const std::unordered_map<int, int> stLookIDRecord
        {
            {MID_DEER, 0X0015},
            {MID_M10 , 0X009F}
        };

        if(stLookIDRecord.find(nMonsterID) == stLookIDRecord.end()){
            return -1;
        }else{
            return stLookIDRecord.at(nMonsterID);
        }
    };

    if(true
            && MonsterID() > MID_NONE
            && MonsterID() < MID_MAX

            && nMotion > MOTION_NONE
            && nMotion < MOTION_MAX

            && nDirection > DIR_NONE
            && nDirection < DIR_MAX){

        // if passed listed simple test
        // we need to check the huge table for it

        int nLookID      = fnLookID(MonsterID());
        int nMotionGfxID = fnMotionGfxID(m_CurrMotion.Motion);

        if(true
                && nLookID      >= 0
                && nMotionGfxID >= 0){

            switch(MonsterID()){
                default:
                    {
                        return ((nLookID & 0X07FF) << 7) + ((nMotionGfxID & 0X000F) << 3) + ((nDirection - DIR_NONE + 1) & 0X0007);
                    }
            }

        }
    }
    return -1;
}

bool Monster::ActionValid(const ActionNode &rstAction, bool)
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

bool Monster::CanFocus(int nPointX, int nPointY)
{
    switch(m_CurrMotion.Motion){
        case MOTION_DIE:
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

        extern PNGTexOffDBN *g_PNGTexOffDBN;
        auto pFrame0 = g_PNGTexOffDBN->Retrieve(nKey0, &nDX0, &nDY0);

        int nShiftX = 0;
        int nShiftY = 0;
        EstimatePixelShift(&nShiftX, &nShiftY);

        int nStartX = X() * SYS_MAPGRIDXP + nDX0 + nShiftX;
        int nStartY = Y() * SYS_MAPGRIDYP + nDY0 + nShiftY;

        int nW = 0;
        int nH = 0;
        SDL_QueryTexture(pFrame0, nullptr, nullptr, &nW, &nH);

        int nMaxTargetW = SYS_MAPGRIDXP + SYS_TARGETRGN_GAPX;
        int nMaxTargetH = SYS_MAPGRIDYP + SYS_TARGETRGN_GAPY;

        return ((nW >= nMaxTargetW) ? PointInSegment(nPointX, (nStartX + (nW - nMaxTargetW) / 2), nMaxTargetW) : PointInSegment(nPointX, nStartX, nW))
            && ((nH >= nMaxTargetH) ? PointInSegment(nPointY, (nStartY + (nH - nMaxTargetH) / 2), nMaxTargetH) : PointInSegment(nPointY, nStartY, nH));
    }

    return false;
}

Monster *Monster::Create(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun, const ActionNode &rstAction)
{
    auto pNew = new Monster(nUID, nMonsterID, pRun);
    pNew->m_CurrMotion.Motion    = MOTION_STAND;
    pNew->m_CurrMotion.Speed     = 0;
    pNew->m_CurrMotion.Direction = DIR_UP;
    pNew->m_CurrMotion.X         = rstAction.X;
    pNew->m_CurrMotion.Y         = rstAction.Y;
    pNew->m_CurrMotion.EndX      = rstAction.EndX;
    pNew->m_CurrMotion.EndY      = rstAction.EndY;

    if(pNew->ParseNewAction(rstAction, true)){
        return pNew;
    }

    delete pNew;
    return nullptr;
}

MotionNode Monster::MakeMotionWalk(int nX0, int nY0, int nX1, int nY1, int nSpeed)
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
                    nMotion = MOTION_WALK;
                    break;
                }
            case 4:
            case 8:
                {
                    nMotion = MOTION_RUN;
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

int Monster::MaxStep() const
{
    return 1;
}
