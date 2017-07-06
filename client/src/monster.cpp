/*
 * =====================================================================================
 *
 *       Filename: monster.cpp
 *        Created: 08/31/2015 08:26:57
 *  Last Modified: 07/06/2017 11:25:37
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

        extern PNGTexOffDBN *g_MonsterDBN;
        auto pFrame0 = g_MonsterDBN->Retrieve(nKey0, &nDX0, &nDY0);
        auto pFrame1 = g_MonsterDBN->Retrieve(nKey1, &nDX1, &nDY1);

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

bool Monster::ParseNewAction(const ActionNode &rstAction, bool bRemote)
{
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
        const static auto stMotionTable = []() -> std::array<int, MOTION_MAX>
        {
            std::array<int, MOTION_MAX> stMotionTable;
            stMotionTable.fill(-1);

            stMotionTable[MOTION_STAND      ] = 0;
            stMotionTable[MOTION_WALK       ] = 1;
            stMotionTable[MOTION_ATTACK     ] = 2;
            stMotionTable[MOTION_UNDERATTACK] = 3;
            stMotionTable[MOTION_DIE        ] = 4;

            return stMotionTable;
        }();

        return ((nMotion > MOTION_NONE) && (nMotion < MOTION_MAX)) ? stMotionTable[nMotion] : -1;
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

        int nLookID      = LookID();
        int nMotionGfxID = fnMotionGfxID(m_CurrMotion.Motion);

        if(true
                && nLookID      >= 0
                && nMotionGfxID >= 0){

            return ((nLookID & 0X07FF) << 7) + ((nMotionGfxID & 0X000F) << 3) + ((nDirection - (DIR_NONE + 1)) & 0X0007);

        }
    }
    return -1;
}

bool Monster::ActionValid(const ActionNode &rstAction, bool bRemote)
{
    // action check should be much looser than motion check
    // since we don't intend to make continuous action list for parsing
    auto fnCheckDir = [](int nDirection) -> bool
    {
        return (nDirection > DIR_NONE) && (nDirection < DIR_MAX);
    };
    
    if(true
            && rstAction.Action > ACTION_NONE
            && rstAction.Action < ACTION_MAX

            && rstAction.Speed >= SYS_MINSPEED
            && rstAction.Speed <= SYS_MAXSPEED

            // we may not use the start point info
            // it's used as a reference of current action and should be valid

            && m_ProcessRun
            && m_ProcessRun->OnMap(rstAction.MapID, rstAction.X, rstAction.Y)){

        switch(rstAction.Action){
            case ACTION_MOVE:
                {
                    // for the move action, we allow the move to an invalid location
                    // but locations should be on current map

                    if(bRemote){
                        return m_ProcessRun->CanMove(false, rstAction.AimX, rstAction.AimY);
                    }else{
                        return m_ProcessRun->OnMap(rstAction.MapID, rstAction.AimX, rstAction.AimY);
                    }
                }
            case ACTION_ATTACK:
                {
                    // attack a target on current map
                    // allow to attack an invalid location if it's magic attack

                    // won't check direction here
                    // plain physical attack need direction info but magic attack not

                    return true;
                }
            case ACTION_STAND:
            case ACTION_UNDERATTACK:
            case ACTION_DIE:
            default:
                {
                    return fnCheckDir(rstAction.Direction);
                }
        }
    }

    return false;
}

bool Monster::MotionValid(const MotionNode &rstMotion)
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
            case MOTION_ATTACK:
            case MOTION_UNDERATTACK:
            case MOTION_DIE:
                {
                    return nLDistance2 == 0;
                }
            case MOTION_WALK:
                {
                    auto nMaxStep = MaxStep();
                    return false
                        || nLDistance2 == 1
                        || nLDistance2 == 2
                        || nLDistance2 == nMaxStep * nMaxStep
                        || nLDistance2 == nMaxStep * nMaxStep * 2;
                }
            default:
                {
                    return false;
                }
        }
    }else{ return false; }
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

        extern PNGTexOffDBN *g_MonsterDBN;
        auto pFrame0 = g_MonsterDBN->Retrieve(nKey0, &nDX0, &nDY0);

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
    pNew->m_CurrMotion = {MOTION_STAND, 0, DIR_UP, rstAction.X, rstAction.Y};

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

int Monster::LookID() const
{
    return (int)(MonsterID() & 0X000007FF) - (MID_NONE + 1);
}
