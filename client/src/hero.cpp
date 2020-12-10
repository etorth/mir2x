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
#include "pathf.hpp"
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
#include "processrun.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_heroDB;
extern PNGTexOffDB *g_weaponDB;
extern ClientArgParser *g_clientArgParser;

Hero::Hero(uint64_t uid, uint32_t dbid, bool gender, uint32_t nDress, ProcessRun *proc, const ActionNode &action)
    : CreatureMovable(uid, proc)
    , m_DBID(dbid)
    , m_gender(gender)
    , m_horse(0)
    , m_weapon(5)
    , m_hair(0)
    , m_hairColor(0)
    , m_dress(nDress - nDress + 6)
    , m_dressColor(0)
    , m_onHorse(false)
{
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_STAND,
        .direction = DIR_DOWN,
        .x = action.X,
        .y = action.Y,
    });

    if(!parseAction(action)){
        throw fflerror("failed to parse action");
    }
}

void Hero::draw(int viewX, int viewY, int)
{
    const auto [shiftX, shiftY] = getShift();
    const auto startX = m_currMotion->x * SYS_MAPGRIDXP + shiftX - viewX;
    const auto startY = m_currMotion->y * SYS_MAPGRIDYP + shiftY - viewY;

    const auto fnDrawWeapon = [startX, startY, this](bool shadow)
    {
        // 04 - 00 :     frame : max =  32
        // 07 - 05 : direction : max =  08 : +
        // 13 - 08 :    motion : max =  64 : +
        // 21 - 14 :    weapon : max = 256 : +----> GfxWeaponID
        //      22 :    gender :
        //      23 :    shadow :
        const auto nGfxWeaponID = GfxWeaponID(m_weapon, m_currMotion->type, m_currMotion->direction);
        if(nGfxWeaponID < 0){
            return;
        }

        const uint32_t weaponKey = (((uint32_t)(shadow ? 1 : 0)) << 23) + (((uint32_t)(m_gender ? 1 : 0)) << 22) + ((nGfxWeaponID & 0X01FFFF) << 5) + m_currMotion->frame;

        int weaponDX = 0;
        int weaponDY = 0;

        auto weaponFrame = g_weaponDB->Retrieve(weaponKey, &weaponDX, &weaponDY);

        if(weaponFrame && shadow){
            SDL_SetTextureAlphaMod(weaponFrame, 128);
        }
        g_sdlDevice->drawTexture(weaponFrame, startX + weaponDX, startY + weaponDY);
    };

    fnDrawWeapon(true);

    const auto nDress     = m_dress;
    const auto nMotion    = m_currMotion->type;
    const auto nDirection = m_currMotion->direction;

    const auto nGfxDressID = GfxDressID(nDress, nMotion, nDirection);
    if(nGfxDressID < 0){
        m_currMotion->print();
        return;
    }

    // human gfx dress id indexing
    // 04 - 00 :     frame : max =  32
    // 07 - 05 : direction : max =  08 : +
    // 13 - 08 :    motion : max =  64 : +
    // 21 - 14 :     dress : max = 256 : +----> GfxDressID
    //      22 :       sex :
    //      23 :    shadow :
    const uint32_t nKey0 = ((uint32_t)(0) << 23) + (((uint32_t)(m_gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + m_currMotion->frame;
    const uint32_t nKey1 = ((uint32_t)(1) << 23) + (((uint32_t)(m_gender ? 1 : 0)) << 22) + (((uint32_t)(nGfxDressID & 0X01FFFF)) << 5) + m_currMotion->frame;

    int nDX0 = 0;
    int nDY0 = 0;
    int nDX1 = 0;
    int nDY1 = 0;

    auto pFrame0 = g_heroDB->Retrieve(nKey0, &nDX0, &nDY0);
    auto pFrame1 = g_heroDB->Retrieve(nKey1, &nDX1, &nDY1);

    if(pFrame1){
        SDL_SetTextureAlphaMod(pFrame1, 128);
    }
    g_sdlDevice->drawTexture(pFrame1, startX + nDX1, startY + nDY1);

    if(true
            && m_weapon
            && WeaponOrder(m_currMotion->type, m_currMotion->direction, m_currMotion->frame) == 1){
        fnDrawWeapon(false);
    }

    for(auto &p: m_attachMagicList){
        p->drawShift(startX, startY, true);
    }

    g_sdlDevice->drawTexture(pFrame0, startX + nDX0, startY + nDY0);

    if(true
            && m_weapon
            && WeaponOrder(m_currMotion->type, m_currMotion->direction, m_currMotion->frame) == 0){
        fnDrawWeapon(false);
    }

    if(g_clientArgParser->drawTextureAlignLine){
        g_sdlDevice->drawLine(colorf::RED + 128, startX, startY, startX + nDX0, startY + nDY0);
        g_sdlDevice->drawLine(colorf::BLUE + 128, startX - 5, startY, startX + 5, startY);
        g_sdlDevice->drawLine(colorf::BLUE + 128, startX, startY - 5, startX, startY + 5);
    }

    if(g_clientArgParser->drawTargetBox){
        if(const auto box = getTargetBox()){
            g_sdlDevice->drawRectangle(colorf::BLUE + 128, box.x - viewX, box.y - viewY, box.w, box.h);
        }
    }

    // draw attached magic for the second time
    // for some direction magic should be on top of body

    for(auto &p: m_attachMagicList){
        switch(p->magicID()){
            case DBCOM_MAGICID(u8"灵魂火符"):
                {
                    switch(m_currMotion->direction){
                        case DIR_UP:
                        case DIR_UPRIGHT:
                        case DIR_RIGHT:
                        case DIR_DOWNRIGHT:
                            {
                                break;
                            }
                        default:
                            {
                                p->drawShift(startX, startY, false);
                                break;
                            }
                    }
                    break;
                }
            default:
                {
                    p->drawShift(startX, startY, false);
                    break;
                }
        }
    }

    switch(m_currMotion->type){
        case MOTION_SPELL0:
        case MOTION_SPELL1:
            {
                if(m_currMotion->extParam.spell.effect){
                    m_currMotion->extParam.spell.effect->drawShift(startX, startY, false);
                }
                break;
            }
        case MOTION_ONEHSWING:
        case MOTION_ONEVSWING:
        case MOTION_TWOHSWING:
        case MOTION_TWOVSWING:
        case MOTION_RANDSWING:
        case MOTION_SPEARHSWING:
        case MOTION_SPEARVSWING:
            {
                if(m_currMotion->extParam.swing.effect){
                    m_currMotion->extParam.swing.effect->drawShift(startX, startY, false);
                }
                break;
            }
        default:
            {
                break;
            }
    }

    // draw HP bar
    // if current m_HPMqx is zero we draw full bar
    switch(m_currMotion->type){
        case MOTION_DIE:
            {
                break;
            }
        default:
            {
                auto pBar0 = g_progUseDB->Retrieve(0X00000014);
                auto pBar1 = g_progUseDB->Retrieve(0X00000015);

                int nW = -1;
                int nH = -1;
                SDL_QueryTexture(pBar1, nullptr, nullptr, &nW, &nH);

                const int drawHPX = startX +  7;
                const int drawHPY = startY - 53;
                const int drawHPW = (int)(std::lround(nW * (m_maxHP ? (std::min<double>)(1.0, (1.0 * m_HP) / m_maxHP) : 1.0)));

                g_sdlDevice->drawTexture(pBar1, drawHPX, drawHPY, 0, 0, drawHPW, nH);
                g_sdlDevice->drawTexture(pBar0, drawHPX, drawHPY);
            }
    }
}

bool Hero::update(double ms)
{
    updateAttachMagic(ms);

    if(SDL_GetTicks() * 1.0f < currMotionDelay() + m_lastUpdateTime){
        return true;
    }

    m_lastUpdateTime = SDL_GetTicks() * 1.0f;
    const CallOnExitHelper motionOnUpdate([this]()
    {
        if(m_currMotion->onUpdate){
            m_currMotion->onUpdate();
        }
    });

    switch(m_currMotion->type){
        case MOTION_STAND:
            {
                if(stayIdle()){
                    return advanceMotionFrame(1);
                }

                // move to next motion will reset frame as 0
                // if current there is no more motion pending
                // it will add a MOTION_STAND
                //
                // we don't want to reset the frame here
                return moveNextMotion();
            }
        case MOTION_HITTED:
            {
                if(stayIdle()){
                    return updateMotion();
                }

                // move to next motion will reset frame as 0
                // if current there is no more motion pending
                // it will add a MOTION_STAND
                //
                // we don't want to reset the frame here
                return moveNextMotion();
            }
        case MOTION_SPELL0:
        case MOTION_SPELL1:
            {
                if(!m_currMotion->extParam.spell.effect){
                    return updateMotion();
                }

                if(m_currMotion->extParam.spell.effect){
                    m_currMotion->extParam.spell.effect->nextFrame();
                }

                const int motionEndFrame = motionFrameCountEx(m_currMotion->type, m_currMotion->direction) - 1;
                const int effectEndFrame = m_currMotion->extParam.spell.effect->frameCount() - 1;
                const int syncFrameCount = [this]() -> int
                {
                    if(m_currMotion->type == MOTION_SPELL0){
                        return 3;
                    }
                    return 1;
                }();

                if( m_currMotion->frame >= motionEndFrame - syncFrameCount && m_currMotion->extParam.spell.effect->frame() < effectEndFrame - syncFrameCount){
                    m_currMotion->frame  = motionEndFrame - syncFrameCount;
                    return true;
                }
                else{
                    return updateMotion();
                }
            }
        case MOTION_ONEHSWING:
        case MOTION_ONEVSWING:
        case MOTION_TWOHSWING:
        case MOTION_TWOVSWING:
        case MOTION_RANDSWING:
        case MOTION_SPEARHSWING:
        case MOTION_SPEARVSWING:
            {
                if(m_currMotion->extParam.swing.effect){
                    m_currMotion->extParam.swing.effect->nextFrame();
                }
                return updateMotion();
            }
        default:
            {
                return updateMotion();
            }
    }
}

bool Hero::motionValid(const MotionNode &motion) const
{
    if(true
            && motion.type >= MOTION_BEGIN
            && motion.type <  MOTION_END

            && motion.direction >= DIR_BEGIN
            && motion.direction <  DIR_END

            && m_processRun
            && m_processRun->onMap(m_processRun->MapID(), motion.x,    motion.y)
            && m_processRun->onMap(m_processRun->MapID(), motion.endX, motion.endY)

            && motion.speed >= SYS_MINSPEED
            && motion.speed <= SYS_MAXSPEED

            && motion.frame >= 0
            && motion.frame <  motionFrameCount(motion.type, motion.direction)){

        const auto nLDistance2 = mathf::LDistance2(motion.x, motion.y, motion.endX, motion.endY);
        switch(motion.type){
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

bool Hero::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();

    m_currMotion->speed = SYS_MAXSPEED;
    m_motionQueue.clear();

    const int endX   = m_forceMotionQueue.empty() ? m_currMotion->endX      : m_forceMotionQueue.back()->endX;
    const int endY   = m_forceMotionQueue.empty() ? m_currMotion->endY      : m_forceMotionQueue.back()->endY;
    const int endDir = m_forceMotionQueue.empty() ? m_currMotion->direction : m_forceMotionQueue.back()->direction;

    // 1. prepare before parsing action
    //    additional movement added if necessary but in rush
    switch(action.Action){
        case ACTION_DIE:
        case ACTION_STAND:
            {
                // take this as cirtical
                // server use ReportStand() to do location sync
                for(auto &motion: makeWalkMotionQueue(endX, endY, action.X, action.Y, SYS_MAXSPEED)){
                    m_forceMotionQueue.insert(m_forceMotionQueue.end(), std::move(motion));
                }
                break;
            }
        case ACTION_MOVE:
        case ACTION_SPELL:
        case ACTION_ATTACK:
            {
                m_motionQueue = makeWalkMotionQueue(endX, endY, action.X, action.Y, SYS_MAXSPEED);
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
    switch(action.Action){
        case ACTION_STAND:
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = [this]()
                    {
                        if(OnHorse()){
                            return MOTION_ONHORSESTAND;
                        }
                        return MOTION_STAND;
                    }(),

                    .direction = action.Direction,
                    .x = action.X,
                    .y = action.Y,
                }));
                break;
            }
        case ACTION_MOVE:
            {
                if(auto moveNode = makeWalkMotion(action.X, action.Y, action.AimX, action.AimY, action.Speed)){
                    m_motionQueue.push_back(std::move(moveNode));
                }
                else{
                    return false;
                }
                break;
            }
        case ACTION_SPACEMOVE2:
            {
                m_currMotion.reset(new MotionNode
                {
                    .type = [this]()
                    {
                        if(OnHorse()){
                            return MOTION_ONHORSESTAND;
                        }
                        return MOTION_STAND;
                    }(),

                    .direction = m_currMotion->direction,
                    .x = action.X,
                    .y = action.Y,
                });

                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"结束", -1)));
                break;
            }
        case ACTION_SPELL:
            {
                if(auto &mr = DBCOM_MAGICRECORD(action.ActionParam)){
                    if(auto &gfxEntry = mr.getGfxEntry(u8"启动")){
                        const auto motionSpell = [&gfxEntry]() -> int
                        {
                            switch(gfxEntry.motion){
                                case 0  : return MOTION_SPELL0;
                                case 1  : return MOTION_SPELL1;
                                default : return MOTION_NONE;
                            }
                        }();

                        if(motionSpell != MOTION_NONE){
                            const auto fnGetSpellDir = [this](int nX0, int nY0, int nX1, int nY1) -> int
                            {
                                switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
                                    case 0:
                                        {
                                            return m_currMotion->direction;
                                        }
                                    default:
                                        {
                                            return PathFind::GetDirection(nX0, nY0, nX1, nY1);
                                        }
                                }
                            };

                            const auto standDir = [&fnGetSpellDir, &action, this]() -> int
                            {
                                if(m_processRun->canMove(true, 0, action.AimX, action.AimY)){
                                    return fnGetSpellDir(action.X, action.Y, action.AimX, action.AimY);
                                }
                                else if(action.AimUID){
                                    if(auto coPtr = m_processRun->findUID(action.AimUID)){
                                        return fnGetSpellDir(action.X, action.Y, coPtr->x(), coPtr->y());
                                    }
                                }
                                return DIR_NONE;
                            }();

                            if(standDir >= DIR_BEGIN && standDir < DIR_END){
                                auto motionPtr = new MotionNode
                                {
                                    .type = motionSpell,
                                    .direction = standDir,
                                    .x = action.X,
                                    .y = action.Y,
                                    .extParam
                                    {
                                        .spell
                                        {
                                            .magicID = (int)(action.ActionParam),
                                        },
                                    },
                                };

                                auto magicPtr = [&action, motionPtr, this]() -> MotionEffectBase *
                                {
                                    switch(action.ActionParam){
                                        case DBCOM_MAGICID(u8"召唤神兽"): return new TaoSumDogEffect    (motionPtr);
                                        case DBCOM_MAGICID(u8"灵魂火符"): return new TaoFireFigureEffect(motionPtr);
                                        default                         : return new MagicSpellEffect   (motionPtr);
                                    }
                                }();

                                motionPtr->extParam.spell.effect = std::unique_ptr<MotionEffectBase>(magicPtr);
                                switch(action.ActionParam){
                                    case DBCOM_MAGICID(u8"灵魂火符"):
                                        {
                                            motionPtr->onUpdate = [lastMotionFrame = (int)(-1), this] () mutable
                                            {
                                                if(lastMotionFrame == m_currMotion->frame){
                                                    return;
                                                }

                                                lastMotionFrame = m_currMotion->frame;
                                                if(m_currMotion->frame != 4){
                                                    return;
                                                }

                                                auto [fromX, fromY] = getTargetBox().center();
                                                PathFind::GetFrontLocation(&fromX, &fromY, fromX, fromY, m_currMotion->direction, 8);

                                                const auto targetUID = m_processRun->FocusUID(FOCUS_MAGIC);
                                                auto magicPtr = new TaoFireFigure_RUN
                                                {
                                                    fromX,
                                                    fromY,

                                                    [fromX, fromY, targetUID, this]() -> int
                                                    {
                                                        const auto [x, y] = [targetUID, this]() -> std::tuple<int, int>
                                                        {
                                                            if(const auto coPtr = m_processRun->findUID(targetUID)){
                                                                return coPtr->getTargetBox().center();
                                                            }

                                                            int mousePX = -1;
                                                            int mousePY = -1;
                                                            SDL_GetMouseState(&mousePX, &mousePY);

                                                            const auto [viewX, viewY] = m_processRun->getViewShift();
                                                            return {mousePX + viewX, mousePY + viewY};
                                                        }();
                                                        return pathf::getDir16(x - fromX, y - fromY);
                                                    }(),

                                                    targetUID,
                                                    m_processRun,
                                                };

                                                magicPtr->addOnDone([targetUID, this]()
                                                {
                                                    m_processRun->addAttachMagic(targetUID, std::unique_ptr<AttachMagic>(new AttachMagic
                                                    {
                                                        u8"灵魂火符",
                                                        u8"结束",
                                                    }));
                                                });
                                                m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(magicPtr));
                                            };
                                            break;
                                        }
                                    default:
                                        {
                                            break;
                                        }
                                }

                                m_motionQueue.push_back(std::unique_ptr<MotionNode>(motionPtr));
                                if(motionSpell == MOTION_SPELL0){
                                    for(int i = 0; i < 2; ++i){
                                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                                        {
                                            .type = MOTION_ATTACKMODE,
                                            .direction = standDir,
                                            .x = action.X,
                                            .y = action.Y,
                                        }));
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
        case ACTION_ATTACK:
            {
                if(auto coPtr = m_processRun->findUID(action.AimUID)){
                    if(const auto attackDir = PathFind::GetDirection(action.X, action.Y, coPtr->x(), coPtr->y()); attackDir >= DIR_BEGIN && attackDir < DIR_END){
                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                        {
                            .type = MOTION_ONEHSWING,
                            .direction = attackDir,
                            .x = action.X,
                            .y = action.Y,
                        }));

                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                        {
                            .type = MOTION_ATTACKMODE,
                            .direction = attackDir,
                            .x = action.X,
                            .y = action.Y,
                        }));
                    }
                }
                else{
                    return false;
                }

                break;
            }
        case ACTION_HITTED:
            {
                m_motionQueue.push_front(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = [this]() -> int
                    {
                        if(OnHorse()){
                            return MOTION_ONHORSEHITTED;
                        }
                        return MOTION_HITTED;
                    }(),

                    .direction = endDir,
                    .x = endX,
                    .y = endY,
                }));
                break;
            }
        case ACTION_PICKUP:
            {
                pickUp();
                break;
            }
        case ACTION_DIE:
            {
                m_forceMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = [this]() -> int
                    {
                        if(OnHorse()){
                            return MOTION_ONHORSEDIE;
                        }
                        return MOTION_DIE;
                    }(),

                    .direction = action.Direction,
                    .x = action.X,
                    .y = action.Y,
                }));
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

std::tuple<int, int> Hero::location() const
{
    if(!motionValid(*m_currMotion)){
        throw fflerror("current motion is invalid");
    }

    switch(m_currMotion->type){
        case MOTION_WALK:
        case MOTION_RUN:
        case MOTION_ONHORSEWALK:
        case MOTION_ONHORSERUN:
            {
                const auto nX0 = m_currMotion->x;
                const auto nY0 = m_currMotion->y;
                const auto nX1 = m_currMotion->endX;
                const auto nY1 = m_currMotion->endY;

                const auto frameCountMoving = motionFrameCount(m_currMotion->type, m_currMotion->direction);
                if(frameCountMoving <= 0){
                    throw fflerror("invalid player moving frame count: %d", frameCountMoving);
                }

                return
                {
                    (m_currMotion->frame < (frameCountMoving / 2)) ? nX0 : nX1,
                    (m_currMotion->frame < (frameCountMoving / 2)) ? nY0 : nY1,
                };
            }
        default:
            {
                return {m_currMotion->x, m_currMotion->y};
            }
    }
}

int Hero::motionFrameCount(int motion, int direction) const
{
    if(!(motion >= MOTION_BEGIN && motion < MOTION_END)){
        return -1;
    }

    if(!(direction >= DIR_BEGIN && direction < DIR_END)){
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
        || m_currMotion->type == MOTION_RUN
        || m_currMotion->type == MOTION_WALK
        || m_currMotion->type == MOTION_ONHORSERUN
        || m_currMotion->type == MOTION_ONHORSEWALK;
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

    constexpr static int s_WeaponOrder[2640]
    {
        #include "weaponorder.inc"
    };

    const auto nGfxMotionID = gfxMotionID(nMotion);
    if(nGfxMotionID < 0){
        return -1;
    }
    return s_WeaponOrder[nGfxMotionID * 80 + (nDirection - DIR_BEGIN) * 10 + nFrame];
}

std::unique_ptr<MotionNode> Hero::makeWalkMotion(int nX0, int nY0, int nX1, int nY1, int nSpeed) const
{
    if(true
            && m_processRun
            && m_processRun->canMove(true, 0, nX0, nY0)
            && m_processRun->canMove(true, 0, nX1, nY1)

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

        return std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = nMotion,
            .direction = nDirV[nSDY][nSDX],
            .speed = nSpeed,
            .x = nX0,
            .y = nY0,
            .endX = nX1,
            .endY = nY1,
        });
    }

    return {};
}

int Hero::GfxWeaponID(int nWeapon, int nMotion, int nDirection) const
{
    static_assert(sizeof(int) > 2, "GfxWeaponID() overflows because of sizeof(int) too small");
    if(true
            && (nWeapon    >  WEAPON_NONE  && nWeapon    < WEAPON_MAX)
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >= DIR_BEGIN    && nDirection < DIR_END   )){
        const auto nGfxMotionID = gfxMotionID(nMotion);
        if(nGfxMotionID >= 0){
            return ((nWeapon - (WEAPON_NONE + 1)) << 9) + (nGfxMotionID << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return -1;
}

int Hero::GfxDressID(int nDress, int nMotion, int nDirection) const
{
    static_assert(sizeof(int) > 2, "GfxDressID() overflows because of sizeof(int) too small");
    if(true
            && (nDress     >  DRESS_NONE   && nDress     < DRESS_MAX )
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >= DIR_BEGIN    && nDirection < DIR_END   )){
        const auto nGfxMotionID = gfxMotionID(nMotion);
        if(nGfxMotionID >= 0){
            return ((nDress - (DRESS_NONE + 1)) << 9) + (nGfxMotionID << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return -1;
}

int Hero::currStep() const
{
    motionValidEx(*m_currMotion);
    switch(m_currMotion->type){
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

ClientCreature::TargetBox Hero::getTargetBox() const
{
    switch(currMotion()->type){
        case MOTION_DIE:
            {
                return {};
            }
        default:
            {
                break;
            }
    }

    const auto nDress     = Dress();
    const auto nGender    = Gender();
    const auto nMotion    = currMotion()->type;
    const auto nDirection = currMotion()->direction;

    const auto texBaseID = GfxDressID(nDress, nMotion, nDirection);
    if(texBaseID < 0){
        return {};
    }

    const uint32_t texID = (((uint32_t)(nGender ? 1 : 0)) << 22) + (((uint32_t)(texBaseID & 0X01FFFF)) << 5) + currMotion()->frame;

    int dx = 0;
    int dy = 0;
    auto bodyFrameTexPtr = g_heroDB->Retrieve(texID, &dx, &dy);

    if(!bodyFrameTexPtr){
        return {};
    }

    const auto [bodyFrameW, bodyFrameH] = SDLDevice::getTextureSize(bodyFrameTexPtr);

    const auto [shiftX, shiftY] = getShift();
    const int startX = m_currMotion->x * SYS_MAPGRIDXP + shiftX + dx;
    const int startY = m_currMotion->y * SYS_MAPGRIDYP + shiftY + dy;

    return getTargetBoxHelper(startX, startY, bodyFrameW, bodyFrameH);
}
