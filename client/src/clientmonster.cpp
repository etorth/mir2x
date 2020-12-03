/*
 * =====================================================================================
 *
 *       Filename: clientmonster.cpp
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
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"
#include "uidf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "clienttaodog.hpp"
#include "clienttaoskeleton.hpp"
#include "creaturemovable.hpp"
#include "clientargparser.hpp"
#include "clientpathfinder.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_monsterDB;
extern ClientArgParser *g_clientArgParser;

MotionNode ClientMonster::makeInitMotion(uint32_t monsterID, const ActionNode &action)
{
    switch(monsterID){
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                return MotionNode
                {
                    MOTION_MON_STAND,
                    0,

                    [&action]() -> int
                    {
                        if(action.Action == ACTION_SPAWN){
                            return DIR_DOWNLEFT;
                        }
                        else if(action.Direction >= DIR_BEGIN && action.Direction < DIR_END){
                            return action.Direction;
                        }
                        else{
                            return DIR_UP;
                        }
                    }(),

                    action.X,
                    action.Y,
                };
            }
        case DBCOM_MONSTERID(u8"神兽"):
            {
                return MotionNode
                {
                    action.Action == ACTION_SPAWN ? MOTION_MON_APPEAR : MOTION_MON_STAND,
                    0,

                    [&action]() -> int
                    {
                        if(action.Direction >= DIR_BEGIN && action.Direction < DIR_END){
                            return action.Direction;
                        }
                        else{
                            return DIR_UP;
                        }
                    }(),

                    action.X,
                    action.Y,
                };
            }
        default:
            {
                return MotionNode
                {
                    MOTION_MON_STAND,
                    0,

                    [&action]() -> int
                    {
                        if(action.Direction >= DIR_BEGIN && action.Direction < DIR_END){
                            return action.Direction;
                        }
                        else{
                            return DIR_UP;
                        }
                    }(),

                    action.X,
                    action.Y,
                };
            }
    }
}

ClientMonster::ClientMonster(uint64_t uid, ProcessRun *proc)
    : CreatureMovable(uid, proc)
{
    if(uidf::getUIDType(uid) != UID_MON){
        throw fflerror("invalid UID for monster type: UIDName = %s", to_cstr(uidf::getUIDString(uid)));
    }

    if(g_clientArgParser->drawUID){
        m_nameBoard.setText(u8"%s(%llu)", DBCOM_MONSTERRECORD(monsterID()).name, to_llu(UID()));
    }
    else{
        m_nameBoard.setText(u8"%s", DBCOM_MONSTERRECORD(monsterID()).name);
    }
}

bool ClientMonster::update(double ms)
{
    updateAttachMagic(ms);

    if(SDL_GetTicks() * 1.0f < currMotionDelay() + m_lastUpdateTime){
        return true;
    }

    m_lastUpdateTime = SDL_GetTicks() * 1.0f;
    switch(m_currMotion.motion){
        case MOTION_MON_STAND:
            {
                if(stayIdle()){
                    return advanceMotionFrame(1);
                }

                // move to next motion will reset frame as 0
                // if current there is no more motion pending
                // it will add a MOTION_MON_STAND
                //
                // we don't want to reset the frame here
                return moveNextMotion();
            }
        case MOTION_MON_DIE:
            {
                const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction);
                if(frameCount <= 0){
                    return false;
                }

                if(m_currMotion.frame + 1 < frameCount){
                    return advanceMotionFrame(1);
                }

                switch(m_currMotion.fadeOut){
                    case 0:
                        {
                            break;
                        }
                    case 255:
                        {
                            // deactivated if fadeOut reach 255
                            // next update will auotmatically delete it
                            break;
                        }
                    default:
                        {
                            int nextFadeOut = 0;
                            nextFadeOut = (std::max<int>)(1, m_currMotion.fadeOut + 10);
                            nextFadeOut = (std::min<int>)(nextFadeOut, 255);

                            m_currMotion.fadeOut = nextFadeOut;
                            break;
                        }
                }
                return true;
            }
        default:
            {
                return updateMotion(false);
            }
    }
}

bool ClientMonster::draw(int viewX, int viewY, int focusMask)
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
    //             |+++-++++-++++--------------------------      look : max = 2048 -+------> gfxID
    //             +---------------------------------------    shadow : max =    2
    //

    const auto nGfxID = gfxID(m_currMotion.motion, m_currMotion.direction);
    if(nGfxID < 0){
        return false;
    }

    const uint32_t nKey0 = ((uint32_t)(0) << 23) + ((uint32_t)(nGfxID & 0X03FFFF) << 5) + m_currMotion.frame; // body
    const uint32_t nKey1 = ((uint32_t)(1) << 23) + ((uint32_t)(nGfxID & 0X03FFFF) << 5) + m_currMotion.frame; // shadow

    int nDX0 = 0;
    int nDY0 = 0;
    int nDX1 = 0;
    int nDY1 = 0;

    auto pFrame0 = g_monsterDB->Retrieve(nKey0, &nDX0, &nDY0);
    auto pFrame1 = g_monsterDB->Retrieve(nKey1, &nDX1, &nDY1);
    const auto [shiftX, shiftY] = getShift();

    // always reset the alpha mode for each texture because texture is shared
    // one texture to draw can be configured with different alpha mode for other creatures

    if(pFrame0){
        SDL_SetTextureAlphaMod(pFrame0, 255);
    }

    if(pFrame1){
        SDL_SetTextureAlphaMod(pFrame1, 128);
    }

    if(true
            && (m_currMotion.motion  == MOTION_MON_DIE)
            && (m_currMotion.fadeOut  > 0             )){
        // FadeOut :    0 : normal
        //         : 1-255: fadeOut
        if(pFrame0){
            SDL_SetTextureAlphaMod(pFrame0, (255 - m_currMotion.fadeOut) / 1);
        }

        if(pFrame1){
            SDL_SetTextureAlphaMod(pFrame1, (255 - m_currMotion.fadeOut) / 2);
        }
    }

    const auto fnBlendFrame = [](SDL_Texture *pTexture, int nFocusChan, int nX, int nY)
    {
        if(true
                && pTexture
                && nFocusChan >= 0
                && nFocusChan <  FOCUS_MAX){

            // if provided channel as 0
            // just blend it using the original color

            const auto stColor = focusColor(nFocusChan);
            if(!SDL_SetTextureColorMod(pTexture, stColor.r, stColor.g, stColor.b)){
                g_SDLDevice->drawTexture(pTexture, nX, nY);
            }
        }
    };

    const int startX = currMotion().x * SYS_MAPGRIDXP + shiftX - viewX;
    const int startY = currMotion().y * SYS_MAPGRIDYP + shiftY - viewY;

    fnBlendFrame(pFrame1, 0, startX + nDX1, startY + nDY1);
    fnBlendFrame(pFrame0, 0, startX + nDX0, startY + nDY0);

    for(int nFocusChan = 1; nFocusChan < FOCUS_MAX; ++nFocusChan){
        if(focusMask & (1 << nFocusChan)){
            fnBlendFrame(pFrame0, nFocusChan, startX + nDX0, startY + nDY0);
        }
    }

    for(auto &p: m_attachMagicList){
        p->Draw(startX, startY);
    }

    if(m_currMotion.motion != MOTION_MON_DIE){
        auto pBar0 = g_progUseDB->Retrieve(0X00000014);
        auto pBar1 = g_progUseDB->Retrieve(0X00000015);

        int nBarW = -1;
        int nBarH = -1;
        SDL_QueryTexture(pBar1, nullptr, nullptr, &nBarW, &nBarH);

        const int drawBarXP = startX +  7;
        const int drawBarYP = startY - 53;
        const int drawBarWidth = (int)(std::lround(nBarW * (m_maxHP ? (std::min<double>)(1.0, (1.0 * m_HP) / m_maxHP) : 1.0)));

        g_SDLDevice->drawTexture(pBar1, drawBarXP, drawBarYP, 0, 0, drawBarWidth, nBarH);
        g_SDLDevice->drawTexture(pBar0, drawBarXP, drawBarYP);

        if(g_clientArgParser->alwaysDrawName || (focusMask & (1 << FOCUS_MOUSE))){
            const int nLW = m_nameBoard.w();
            const int nLH = m_nameBoard.h();
            const int nDrawNameXP = drawBarXP + nBarW / 2 - nLW / 2;
            const int nDrawNameYP = drawBarYP + 20;
            m_nameBoard.drawEx(nDrawNameXP, nDrawNameYP, 0, 0, nLW, nLH);
        }
    }
    return true;
}

std::tuple<int, int> ClientMonster::location() const
{
    if(!motionValid(m_currMotion)){
        throw fflerror("invalid current motion");
    }

    switch(currMotion().motion){
        case MOTION_MON_WALK:
            {
                const auto nX0 = m_currMotion.x;
                const auto nY0 = m_currMotion.y;
                const auto nX1 = m_currMotion.endX;
                const auto nY1 = m_currMotion.endY;

                const auto frameCountMoving = motionFrameCount(MOTION_MON_WALK, m_currMotion.direction);
                if(frameCountMoving <= 0){
                    throw fflerror("invalid monster moving frame count: %d", frameCountMoving);
                }

                return
                {
                    (m_currMotion.frame < (frameCountMoving / 2)) ? nX0 : nX1,
                    (m_currMotion.frame < (frameCountMoving / 2)) ? nY0 : nY1,
                };
            }
        default:
            {
                return {m_currMotion.x, m_currMotion.y};
            }
    }
}

bool ClientMonster::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();

    // try find pending motion die
    // ignore all the following actions till the MOTION_MON_DIE done

    for(const auto &m: m_forceMotionQueue){
        if(m.motion == MOTION_MON_DIE){
            return true;
        }
    }

    for(const auto &m: m_motionQueue){
        if(m.motion == MOTION_MON_DIE){
            throw fflerror("Found MOTION_MON_DIE in pending motion queue");
        }
    }

    // make current motion super-fast
    // can presents those ignored actions, helpful for debug
    m_currMotion.speed = SYS_MAXSPEED;
    m_motionQueue.clear();

    const auto [endX, endY] = [&action, this]() -> std::array<int, 2>
    {
        if(!m_forceMotionQueue.empty()){
            return {m_forceMotionQueue.back().endX, m_forceMotionQueue.back().endY};
        }
        return {m_currMotion.endX, m_currMotion.endY};
    }();

    // 1. prepare before parsing action
    //    additional movement added if necessary but in rush
    switch(action.Action){
        case ACTION_STAND:
        case ACTION_MOVE:
        case ACTION_ATTACK:
        case ACTION_HITTED:
            {
                m_motionQueue = makeWalkMotionQueue(endX, endY, action.X, action.Y, SYS_MAXSPEED);
                break;
            }
        case ACTION_DIE:
            {
                const auto motionQueue = makeWalkMotionQueue(endX, endY, action.X, action.Y, SYS_MAXSPEED);
                m_forceMotionQueue.insert(m_forceMotionQueue.end(), motionQueue.begin(), motionQueue.end());
                break;
            }
        default:
            {
                break;
            }
    }

    // 2. parse the action
    switch(action.Action){
        case ACTION_DIE:
            {
                m_forceMotionQueue.emplace_back(MOTION_MON_DIE, 0, action.Direction, action.X, action.Y);
                m_forceMotionQueue.back().fadeOut = action.ActionParam;
                break;
            }
        case ACTION_STAND:
            {
                m_motionQueue.emplace_back(MOTION_MON_STAND, 0, action.Direction, action.X, action.Y);
                break;
            }
        case ACTION_HITTED:
            {
                m_motionQueue.emplace_back(MOTION_MON_HITTED, 0, action.Direction, action.X, action.Y);
                break;
            }
        case ACTION_MOVE:
            {
                if(auto stMotionNode = makeWalkMotion(action.X, action.Y, action.AimX, action.AimY, action.Speed)){
                    m_motionQueue.push_back(stMotionNode);
                }
                break;
            }
        case ACTION_SPACEMOVE2:
            {
                m_currMotion = MotionNode
                {
                    MOTION_MON_STAND,
                    0,
                    m_currMotion.direction,
                    action.X,
                    action.Y,
                };

                addAttachMagic(DBCOM_MAGICID(u8"瞬息移动"), 0, EGS_DONE);
                break;
            }
        case ACTION_ATTACK:
            {
                if(auto pCreature = m_processRun->findUID(action.AimUID)){
                    auto nX   = pCreature->x();
                    auto nY   = pCreature->y();
                    auto nDir = PathFind::GetDirection(action.X, action.Y, nX, nY);

                    if(nDir >= DIR_BEGIN && nDir < DIR_END){
                        m_motionQueue.emplace_back(MOTION_MON_ATTACK0, 0, nDir, action.X, action.Y);
                    }
                }
                else{
                    return false;
                }
                break;
            }
        case ACTION_SPAWN:
            {
                onActionSpawn(action);
                break;
            }
        case ACTION_TRANSF:
            {
                onActionTransf(action);
                break;
            }
        default:
            {
                return false;
            }
    }

    // 3. after action parse
    //    verify the whole motion queue
    return motionQueueValid();
}

bool ClientMonster::motionValid(const MotionNode &motion) const
{
    if(true
            && motion.motion >= MOTION_MON_BEGIN
            && motion.motion <  MOTION_MON_END

            && motion.direction >= DIR_BEGIN
            && motion.direction <  DIR_END

            && m_processRun
            && m_processRun->onMap(m_processRun->MapID(), motion.x,    motion.y)
            && m_processRun->onMap(m_processRun->MapID(), motion.endX, motion.endY)

            && motion.speed >= SYS_MINSPEED
            && motion.speed <= SYS_MAXSPEED

            && motion.frame >= 0
            && motion.frame <  motionFrameCount(motion.motion, motion.direction)){

        const auto nLDistance2 = mathf::LDistance2(motion.x, motion.y, motion.endX, motion.endY);
        switch(motion.motion){
            case MOTION_MON_STAND:
                {
                    return nLDistance2 == 0;
                }
            case MOTION_MON_WALK:
                {
                    return false
                        || nLDistance2 == 1
                        || nLDistance2 == 2
                        || nLDistance2 == 1 * maxStep() * maxStep()
                        || nLDistance2 == 2 * maxStep() * maxStep();
                }
            case MOTION_MON_ATTACK0:
            case MOTION_MON_HITTED:
            case MOTION_MON_DIE:
            case MOTION_MON_APPEAR:
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

bool ClientMonster::canFocus(int pointX, int pointY) const
{
    switch(m_currMotion.motion){
        case MOTION_MON_DIE:
            {
                return false;
            }
        default:
            {
                break;
            }
    }

    const auto nGfxID = gfxID(m_currMotion.motion, m_currMotion.direction);
    if(nGfxID < 0){
        return false;
    }

    // we only check the body frame
    // can focus or not is decided by the graphics size

    const uint32_t nKey0 = ((uint32_t)(nGfxID & 0X03FFFF) << 5) + m_currMotion.frame;

    int nDX0 = 0;
    int nDY0 = 0;

    auto pFrame0 = g_monsterDB->Retrieve(nKey0, &nDX0, &nDY0);
    const auto [shiftX, shiftY] = getShift();

    const int startX = m_currMotion.x * SYS_MAPGRIDXP + shiftX + nDX0;
    const int startY = m_currMotion.y * SYS_MAPGRIDYP + shiftY + nDY0;

    int nW = 0;
    int nH = 0;
    SDL_QueryTexture(pFrame0, nullptr, nullptr, &nW, &nH);

    const int maxTargetW = SYS_MAPGRIDXP + SYS_TARGETRGN_GAPX;
    const int maxTargetH = SYS_MAPGRIDYP + SYS_TARGETRGN_GAPY;

    return ((nW >= maxTargetW) ? mathf::pointInSegment(pointX, (startX + (nW - maxTargetW) / 2), maxTargetW) : mathf::pointInSegment(pointX, startX, nW))
        && ((nH >= maxTargetH) ? mathf::pointInSegment(pointY, (startY + (nH - maxTargetH) / 2), maxTargetH) : mathf::pointInSegment(pointY, startY, nH));
}

int ClientMonster::gfxID(int nMotion, int nDirection) const
{
    // see ClientMonster::Draw() for format of nGfxID
    // monster GfxID consists of (LookID, Motion, Direction)
    static_assert(sizeof(int) > 2, "ClientMonster::GfxID() overflows because of sizeof(int) <= 2");

    if(!monsterID()){
        return -1;
    }

    const auto nLookID      = lookID();
    const auto nGfxMotionID = gfxMotionID(nMotion);

    if(true
            && nLookID >= LID_BEGIN
            && nLookID <  LID_END

            && nDirection >= DIR_BEGIN
            && nDirection <  DIR_END

            && nGfxMotionID >= 0){

        // if passed listed simple test
        // we need to check the huge table for it

        return (((nLookID - LID_BEGIN) & 0X07FF) << 7) + ((nGfxMotionID & 0X000F) << 3) + ((nDirection - DIR_BEGIN) & 0X0007);
    }
    return -1;
}

int ClientMonster::motionFrameCount(int motion, int direction) const
{
    const auto nGfxID = gfxID(motion, direction);
    if(nGfxID < 0){
        return -1;
    }

    switch(motion){
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
                switch(lookID()){
                    default:
                        {
                            return 2;
                        }
                }
            }
        case MOTION_MON_DIE:
            {
                switch(lookID()){
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
                switch(lookID()){
                    default:
                        {
                            return 10;
                        }
                }
            }
        case MOTION_MON_SPECIAL:
            {
                switch(lookID()){
                    default:
                        {
                            return 6;
                        }
                }
            }
        default:
            {
                return -1;
            }
    }
}

MotionNode ClientMonster::makeWalkMotion(int nX0, int nY0, int nX1, int nY1, int nSpeed) const
{
    if(true
            && m_processRun
            && m_processRun->CanMove(true, 0, nX0, nY0)
            && m_processRun->CanMove(true, 0, nX1, nY1)

            && nSpeed >= SYS_MINSPEED
            && nSpeed <= SYS_MAXSPEED){

        static const int nDirV[][3] = {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

        int nSDX = 1 + (nX1 > nX0) - (nX1 < nX0);
        int nSDY = 1 + (nY1 > nY0) - (nY1 < nY0);

        auto nLDistance2 = mathf::LDistance2(nX0, nY0, nX1, nY1);
        if(false
                || nLDistance2 == 1
                || nLDistance2 == 2
                || nLDistance2 == 1 * maxStep() * maxStep()
                || nLDistance2 == 2 * maxStep() * maxStep()){

            return {MOTION_MON_WALK, 0, nDirV[nSDY][nSDX], nSpeed, nX0, nY0, nX1, nY1};
        }
    }
    return {};
}
