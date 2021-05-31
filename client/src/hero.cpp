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
#include "dbcomrecord.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_heroDB;
extern PNGTexOffDB *g_hairDB;
extern PNGTexOffDB *g_weaponDB;
extern PNGTexOffDB *g_helmetDB;
extern ClientArgParser *g_clientArgParser;

Hero::Hero(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : CreatureMovable(uid, proc)
    , m_horse(0)
    , m_onHorse(false)
{
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_STAND,
        .direction = DIR_DOWN,
        .x = action.x,
        .y = action.y,
    });

    if(!parseAction(action)){
        throw fflerror("failed to parse action");
    }
}

void Hero::drawFrame(int viewX, int viewY, int, int frame, bool)
{
    const auto [shiftX, shiftY] = getShift(frame);
    const auto startX = m_currMotion->x * SYS_MAPGRIDXP + shiftX - viewX;
    const auto startY = m_currMotion->y * SYS_MAPGRIDYP + shiftY - viewY;

    const auto fnDrawWeapon = [startX, startY, frame, this](bool shadow)
    {
        // 04 - 00 :     frame : max =  32
        // 07 - 05 : direction : max =  08 : +
        // 13 - 08 :    motion : max =  64 : +
        // 21 - 14 :    weapon : max = 256 : +----> GfxWeaponID
        //      22 :    gender :
        //      23 :    shadow :
        const auto nGfxWeaponID = GfxWeaponID(DBCOM_ITEMRECORD(getWLItem(WLG_WEAPON).itemID).shape, m_currMotion->type, m_currMotion->direction);
        if(nGfxWeaponID < 0){
            return;
        }

        const uint32_t weaponKey = ((to_u32(shadow ? 1 : 0)) << 23) + (to_u32(gender()) << 22) + ((nGfxWeaponID & 0X01FFFF) << 5) + frame;
        const auto [weaponFrame, weaponDX, weaponDY] = g_weaponDB->Retrieve(weaponKey);

        if(weaponFrame && shadow){
            SDL_SetTextureAlphaMod(weaponFrame, 128);
        }
        g_sdlDevice->drawTexture(weaponFrame, startX + weaponDX, startY + weaponDY);
    };

    fnDrawWeapon(true);

    const auto nDress     = DBCOM_ITEMRECORD(getWLItem(WLG_DRESS).itemID).shape;
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
    const uint32_t   bodyKey = (to_u32(0) << 23) + (to_u32(gender()) << 22) + ((to_u32(nGfxDressID & 0X01FFFF)) << 5) + frame;
    const uint32_t shadowKey = (to_u32(1) << 23) + (to_u32(gender()) << 22) + ((to_u32(nGfxDressID & 0X01FFFF)) << 5) + frame;

    const auto [  bodyFrame,   bodyDX,   bodyDY] = g_heroDB->Retrieve(bodyKey);
    const auto [shadowFrame, shadowDX, shadowDY] = g_heroDB->Retrieve(shadowKey);

    if(shadowFrame){
        SDL_SetTextureAlphaMod(shadowFrame, 128);
    }
    g_sdlDevice->drawTexture(shadowFrame, startX + shadowDX, startY + shadowDY);

    if(true
            && getWLItem(WLG_WEAPON)
            && WeaponOrder(m_currMotion->type, m_currMotion->direction, frame) == 1){
        fnDrawWeapon(false);
    }

    for(auto &p: m_attachMagicList){
        switch(p->magicID()){
            case DBCOM_MAGICID(u8"魔法盾"):
                {
                    p->drawShift(startX, startY, true);
                    break;
                }
            default:
                {
                    p->drawShift(startX, startY, false);
                    break;
                }
        }
    }

    g_sdlDevice->drawTexture(bodyFrame, startX + bodyDX, startY + bodyDY);
    if(getWLItem(WLG_HELMET)){
        if(const auto nHelmetGfxID = gfxHelmetID(DBCOM_ITEMRECORD(getWLItem(WLG_HELMET).itemID).shape, nMotion, nDirection); nHelmetGfxID >= 0){
            const uint32_t nHelmetKey = (to_u32(gender()) << 22) + ((to_u32(nHelmetGfxID & 0X01FFFF)) << 5) + frame;
            if(auto [texPtr, dx, dy] = g_helmetDB->Retrieve(nHelmetKey); texPtr){
                g_sdlDevice->drawTexture(texPtr, startX + dx, startY + dy);
            }
        }
    }
    else{
        if(const auto nHairGfxID = GfxHairID(m_sdWLDesp.hair, nMotion, nDirection); nHairGfxID >= 0){
            const uint32_t nHairKey = (to_u32(gender()) << 22) + ((to_u32(nHairGfxID & 0X01FFFF)) << 5) + frame;
            if(auto [texPtr, dx, dy] = g_hairDB->Retrieve(nHairKey); texPtr){
                SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, m_sdWLDesp.hairColor);
                g_sdlDevice->drawTexture(texPtr, startX + dx, startY + dy);
            }
        }
    }

    if(true
            && getWLItem(WLG_WEAPON)
            && WeaponOrder(m_currMotion->type, m_currMotion->direction, frame) == 0){
        fnDrawWeapon(false);
    }

    if(g_clientArgParser->drawTextureAlignLine){
        g_sdlDevice->drawLine (colorf::RED  + 128, startX, startY, startX + bodyDX, startY + bodyDY);
        g_sdlDevice->drawCross(colorf::BLUE + 128, startX, startY, 5);

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(bodyFrame);
        g_sdlDevice->drawRectangle(colorf::RED + 128, startX + bodyDX, startY + bodyDY, texW, texH);
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
                                p->drawShift(startX, startY, true);
                                break;
                            }
                    }
                    break;
                }
            default:
                {
                    p->drawShift(startX, startY, true);
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
                if(m_currMotion->extParam.swing.magicID){
                    //
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
    if(m_currMotion->type != MOTION_DIE && g_clientArgParser->drawHPBar){
        auto bar0Ptr = g_progUseDB->Retrieve(0X00000014);
        auto bar1Ptr = g_progUseDB->Retrieve(0X00000015);

        const auto [bar1TexW, bar1TexH] = SDLDeviceHelper::getTextureSize(bar1Ptr);

        const int drawHPX = startX +  7;
        const int drawHPY = startY - 53;
        const int drawHPW = to_d(std::lround(bar1TexW * (m_maxHP ? (std::min<double>)(1.0, (1.0 * m_HP) / m_maxHP) : 1.0)));

        g_sdlDevice->drawTexture(bar1Ptr, drawHPX, drawHPY, 0, 0, drawHPW, bar1TexH);
        g_sdlDevice->drawTexture(bar0Ptr, drawHPX, drawHPY);
    }
}

bool Hero::update(double ms)
{
    updateAttachMagic(ms);
    m_currMotion->updateSpellEffect(ms);

    if(!checkUpdate(ms)){
        return true;
    }

    const CallOnExitHelper motionOnUpdate([this]()
    {
        m_currMotion->update();
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

                const int motionEndFrame = motionFrameCountEx(m_currMotion->type, m_currMotion->direction) - 1;
                const int effectEndFrame = m_currMotion->extParam.spell.effect->frameCount() - 1;
                const int motionSyncFrameCount = [this]() -> int
                {
                    if(m_currMotion->type == MOTION_SPELL0){
                        return 3;
                    }
                    return 1;
                }();
                const int effectSyncFrameCount = [motionSyncFrameCount, this]() -> int
                {
                    return std::lround(m_currMotion->extParam.spell.effect->speed() * motionSyncFrameCount / m_currMotion->speed);
                }();

                if( m_currMotion->frame >= motionEndFrame - motionSyncFrameCount && m_currMotion->extParam.spell.effect->frame() < effectEndFrame - effectSyncFrameCount){
                    m_currMotion->frame  = motionEndFrame - motionSyncFrameCount;
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
                if(m_currMotion->extParam.swing.magicID){
                    //
                }
                return updateMotion();
            }
        default:
            {
                return updateMotion();
            }
    }
}

bool Hero::motionValid(const std::unique_ptr<MotionNode> &motionPtr) const
{
    if(true
            && motionPtr
            && motionPtr->type >= MOTION_BEGIN
            && motionPtr->type <  MOTION_END

            && motionPtr->direction >= DIR_BEGIN
            && motionPtr->direction <  DIR_END

            && m_processRun
            && m_processRun->onMap(m_processRun->mapID(), motionPtr->x,    motionPtr->y)
            && m_processRun->onMap(m_processRun->mapID(), motionPtr->endX, motionPtr->endY)

            && motionPtr->speed >= SYS_MINSPEED
            && motionPtr->speed <= SYS_MAXSPEED

            && motionPtr->frame >= 0
            && motionPtr->frame <  motionFrameCount(motionPtr->type, motionPtr->direction)){

        const auto nLDistance2 = mathf::LDistance2(motionPtr->x, motionPtr->y, motionPtr->endX, motionPtr->endY);
        switch(motionPtr->type){
            case MOTION_STAND:
                {
                    return !onHorse() && (nLDistance2 == 0);
                }
            case MOTION_SPELL0:
            case MOTION_SPELL1:
                {
                    return !onHorse() && (nLDistance2 == 0);
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
                    return !onHorse() && (nLDistance2 == 0);
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
                    return !onHorse() && (nLDistance2 == 0);
                }
            case MOTION_BACKDROPKICK:
                {
                    return false;
                }
            case MOTION_DIE:
                {
                    return !onHorse() && (nLDistance2 == 0);
                }
            case MOTION_ONHORSEDIE:
                {
                    return  onHorse() && (nLDistance2 == 0);
                }
            case MOTION_WALK:
                {
                    return !onHorse() && (nLDistance2 == 1 || nLDistance2 == 2);
                }
            case MOTION_RUN:
                {
                    return !onHorse() && (nLDistance2 == 4 || nLDistance2 == 8);
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
                    return onHorse() && (nLDistance2 == 0);
                }
            case MOTION_ONHORSEWALK:
                {
                    return onHorse() && (nLDistance2 == 1 || nLDistance2 == 2);
                }
            case MOTION_ONHORSERUN:
                {
                    return onHorse() && (nLDistance2 == 9 || nLDistance2 == 18);
                }
            case MOTION_ONHORSEHITTED:
                {
                    return onHorse() && (nLDistance2 == 0);
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

    const int endX   = m_forcedMotionQueue.empty() ? m_currMotion->endX      : m_forcedMotionQueue.back()->endX;
    const int endY   = m_forcedMotionQueue.empty() ? m_currMotion->endY      : m_forcedMotionQueue.back()->endY;
    const int endDir = m_forcedMotionQueue.empty() ? m_currMotion->direction : m_forcedMotionQueue.back()->direction;

    // 1. prepare before parsing action
    //    additional movement added if necessary but in rush
    switch(action.type){
        case ACTION_DIE:
            {
                // take this as cirtical
                // server use ReportStand() to do location sync
                for(auto &motion: makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED)){
                    m_forcedMotionQueue.insert(m_forcedMotionQueue.end(), std::move(motion));
                }
                break;
            }
        case ACTION_MOVE:
        case ACTION_STAND:
        case ACTION_SPELL:
        case ACTION_ATTACK:
            {
                m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
                break;
            }
        case ACTION_SPACEMOVE2:
            {
                break;
            }
        default:
            {
                break;
            }
    }

    // 2. parse the action
    //    now motion list is at the right grid
    switch(action.type){
        case ACTION_STAND:
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = [this]()
                    {
                        if(onHorse()){
                            return MOTION_ONHORSESTAND;
                        }
                        return MOTION_STAND;
                    }(),

                    .direction = action.direction,
                    .x = action.x,
                    .y = action.y,
                }));
                break;
            }
        case ACTION_MOVE:
            {
                if(auto moveNode = makeWalkMotion(action.x, action.y, action.aimX, action.aimY, action.speed)){
                    m_motionQueue.push_back(std::move(moveNode));
                    if(action.extParam.move.pickUp){
                        if(UID() != m_processRun->getMyHeroUID()){
                            throw fflerror("invalid UID to trigger pickUp action: uid = %llu", to_llu(UID()));
                        }

                        m_motionQueue.back()->addUpdate(false, [this](MotionNode *motionPtr) -> bool
                        {
                            if(motionPtr->frame < 4){
                                return false;
                            }

                            m_processRun->requestPickUp();
                            return true;
                        });
                    }
                }
                else{
                    return false;
                }
                break;
            }
        case ACTION_SPACEMOVE2:
            {
                flushForcedMotion();
                jumpLoc(action.x, action.y, m_currMotion->direction);
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"结束")));
                break;
            }
        case ACTION_SPELL:
            {
                const auto magicID = action.extParam.spell.magicID;
                if(auto &mr = DBCOM_MAGICRECORD(magicID)){
                    if(auto &gfxEntry = mr.getGfxEntry(u8"启动")){
                        const auto motionSpell = [&gfxEntry]() -> int
                        {
                            switch(gfxEntry.motion){
                                case 0  : return MOTION_SPELL0;
                                case 1  : return MOTION_SPELL1;
                                default : throw bad_reach();
                            }
                        }();

                        const auto fnGetSpellDir = [this](int nX0, int nY0, int nX1, int nY1) -> int
                        {
                            switch(mathf::LDistance2(nX0, nY0, nX1, nY1)){
                                case 0:
                                    {
                                        return m_currMotion->direction;
                                    }
                                default:
                                    {
                                        return pathf::getDir8(nX1 - nX0, nY1 - nY0) + DIR_BEGIN;
                                    }
                            }
                        };

                        const auto standDir = [&fnGetSpellDir, &action, this]() -> int
                        {
                            if(action.aimUID){
                                if(auto coPtr = m_processRun->findUID(action.aimUID); coPtr && coPtr->getTargetBox()){
                                    if(const auto dir = m_processRun->getAimDirection(action, DIR_NONE); dir != DIR_NONE){
                                        return dir;
                                    }
                                }
                                return m_currMotion->direction;
                            }
                            else{
                                return fnGetSpellDir(action.x, action.y, action.aimX, action.aimY);
                            }
                        }();

                        fflassert(directionValid(standDir));
                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                        {
                            .type = motionSpell,
                            .direction = standDir,
                            .x = action.x,
                            .y = action.y,
                            .extParam
                            {
                                .spell
                                {
                                    .magicID = magicID,
                                },
                            },
                        }));

                        m_motionQueue.back()->extParam.spell.effect.reset(new CastMagicMotionEffect(m_motionQueue.back().get()));
                        if(UID() == m_processRun->getMyHeroUID()){
                            m_processRun->getMyHero()->setMagicCastTime(magicID);
                        }

                        switch(magicID){
                            case DBCOM_MAGICID(u8"冰沙掌"):
                            case DBCOM_MAGICID(u8"地狱火"):
                                {
                                    m_motionQueue.back()->addUpdate(true, [standDir, magicID, this](MotionNode *motionPtr) -> bool
                                    {
                                        // usually when reaches this cb the motionPtr is just currMotion()
                                        // but not true if in flushForcedMotion()

                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        const auto castX = motionPtr->endX;
                                        const auto castY = motionPtr->endY;

                                        for(const auto distance: {1, 2, 3, 4, 5, 6, 7, 8}){
                                            m_processRun->addDelay(distance * 100, [standDir, magicID, castX, castY, distance, castMapID = m_processRun->mapID(), this]()
                                            {
                                                if(m_processRun->mapID() != castMapID){
                                                    return;
                                                }

                                                const auto [aimX, aimY] = pathf::getFrontGLoc(castX, castY, standDir, distance);
                                                if(!m_processRun->groundValid(aimX, aimY)){
                                                    return;
                                                }

                                                if(magicID == DBCOM_MAGICID(u8"冰沙掌")){
                                                    m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new IceThrust_RUN(aimX, aimY, standDir)));
                                                }
                                                else if(magicID == DBCOM_MAGICID(u8"地狱火")){
                                                    m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new HellFire_RUN
                                                    {
                                                        aimX,
                                                        aimY,
                                                        standDir,
                                                    }))->addOnUpdate([aimX, aimY, this](MagicBase *magicPtr)
                                                    {
                                                        if(magicPtr->frame() < 10){
                                                            return false;
                                                        }

                                                        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FireAshEffect_RUN
                                                        {
                                                            aimX,
                                                            aimY,
                                                            1000,
                                                        }));
                                                        return true;
                                                    });
                                                }
                                                else{
                                                    throw bad_reach();
                                                }
                                            });
                                        }
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"疾光电影"):
                                {
                                    m_motionQueue.back()->addUpdate(true, [standDir, this](MotionNode *motionPtr) -> bool
                                    {
                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                                        {
                                            u8"疾光电影",
                                            u8"运行",

                                            motionPtr->endX,
                                            motionPtr->endY,
                                            standDir - DIR_BEGIN,
                                        }));
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"火球术"):
                            case DBCOM_MAGICID(u8"大火球"):
                            case DBCOM_MAGICID(u8"灵魂火符"):
                            case DBCOM_MAGICID(u8"冰月神掌"):
                            case DBCOM_MAGICID(u8"冰月震天"):
                                {
                                    m_motionQueue.back()->addUpdate(true, [targetUID = action.aimUID, magicID, this](MotionNode *motionPtr) -> bool
                                    {
                                        // usually when reaches this cb the current motion is motionPtr
                                        // but not true if in flushForcedMotion()

                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        const auto fromX = motionPtr->x * SYS_MAPGRIDXP;
                                        const auto fromY = motionPtr->y * SYS_MAPGRIDYP;
                                        const auto flyDir16Index = [fromX, fromY, targetUID, this]() -> int
                                        {
                                            if(const auto coPtr = m_processRun->findUID(targetUID)){
                                                if(const auto targetBox = coPtr->getTargetBox()){
                                                    const auto [targetPX, targetPY] = targetBox.targetPLoc();
                                                    return pathf::getDir16((targetPX - fromX) * SYS_MAPGRIDYP, (targetPY - fromY) * SYS_MAPGRIDXP);
                                                }
                                            }
                                            return (m_currMotion->direction - DIR_BEGIN) * 2;
                                        }();

                                        const auto gfxDirIndex = [magicID, flyDir16Index]()
                                        {
                                            switch(magicID){
                                                case DBCOM_MAGICID(u8"冰月震天"): return 0;
                                                default                         : return flyDir16Index;
                                            }
                                        }();

                                        m_processRun->addFollowUIDMagic(std::unique_ptr<FollowUIDMagic>(new FollowUIDMagic
                                        {
                                            DBCOM_MAGICRECORD(magicID).name,
                                            u8"运行",

                                            fromX,
                                            fromY,

                                            gfxDirIndex,
                                            flyDir16Index,
                                            20,

                                            targetUID,
                                            m_processRun,

                                        }))->addOnDone([targetUID, magicID, this](MagicBase *)
                                        {
                                            if(auto coPtr = m_processRun->findUID(targetUID)){
                                                coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(DBCOM_MAGICRECORD(magicID).name, u8"结束")));
                                            }
                                        });
                                        return true;
                                    });
                                    break;
                                }
                            default:
                                {
                                    break;
                                }
                        }

                        if(motionSpell == MOTION_SPELL0){
                            for(int i = 0; i < 2; ++i){
                                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                                {
                                    .type = MOTION_ATTACKMODE,
                                    .direction = standDir,
                                    .x = action.x,
                                    .y = action.y,
                                }));
                            }
                        }
                    }
                }
                break;
            }
        case ACTION_ATTACK:
            {
                if(auto coPtr = m_processRun->findUID(action.aimUID)){
                    if(const auto attackDir = PathFind::GetDirection(action.x, action.y, coPtr->x(), coPtr->y()); attackDir >= DIR_BEGIN && attackDir < DIR_END){
                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                        {
                            .type = MOTION_ONEVSWING,
                            .direction = attackDir,
                            .x = action.x,
                            .y = action.y,
                        }));

                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                        {
                            .type = MOTION_ATTACKMODE,
                            .direction = attackDir,
                            .x = action.x,
                            .y = action.y,
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
                        if(onHorse()){
                            return MOTION_ONHORSEHITTED;
                        }
                        return MOTION_HITTED;
                    }(),

                    .direction = endDir,
                    .x = endX,
                    .y = endY,
                }));

                m_motionQueue.front()->addUpdate(true, [this](MotionNode *) -> bool
                {
                    for(auto &p: m_attachMagicList){
                        if(p->magicID() == DBCOM_MAGICID(u8"魔法盾")){
                            // here we replace the old 魔法盾 with a new one
                            // it's possible the old one haven't trigger its onDone callback yet, but doesn't matter
                            p.reset(new AttachMagic(u8"魔法盾", u8"挨打"));
                            p->addOnDone([this](MagicBase *)
                            {
                                // don't use p, the m_attachMagicList may re-allocate
                                // don't replace the ptr holding by p, just add a new magic, since the callback is by ptr holding by p
                                for(auto &p: m_attachMagicList){
                                    if(p->magicID() == DBCOM_MAGICID(u8"魔法盾")){
                                        addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"魔法盾", u8"运行")));
                                        break;
                                    }
                                }
                            });
                            break;
                        }
                    }
                    return true;
                });
                break;
            }
        case ACTION_DIE:
            {
                const auto [dieX, dieY, dieDir] = motionEndGLoc(END_FORCED);
                m_forcedMotionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = [this]() -> int
                    {
                        if(onHorse()){
                            return MOTION_ONHORSEDIE;
                        }
                        return MOTION_DIE;
                    }(),

                    .direction = dieDir,
                    .x = dieX,
                    .y = dieY,
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

FrameSeq Hero::motionFrameSeq(int motion, int direction) const
{
    if(!(motion >= MOTION_BEGIN && motion < MOTION_END)){
        return {};
    }

    if(!(direction >= DIR_BEGIN && direction < DIR_END)){
        return {};
    }

    switch(motion){
        case MOTION_STAND		    : return {.count =  4};
        case MOTION_ARROWATTACK		: return {.count =  6};
        case MOTION_SPELL0		    : return {.count =  5};
        case MOTION_SPELL1		    : return {.count =  5};
        case MOTION_HOLD		    : return {.count =  1};
        case MOTION_PUSHBACK		: return {.count =  1};
        case MOTION_PUSHBACKFLY		: return {.count =  1};
        case MOTION_ATTACKMODE		: return {.count =  3};
        case MOTION_CUT		        : return {.count =  2};
        case MOTION_ONEVSWING		: return {.count =  6};
        case MOTION_TWOVSWING		: return {.count =  6};
        case MOTION_ONEHSWING		: return {.count =  6};
        case MOTION_TWOHSWING		: return {.count =  6};
        case MOTION_SPEARVSWING     : return {.count =  6};
        case MOTION_SPEARHSWING     : return {.count =  6};
        case MOTION_HITTED          : return {.count =  3};
        case MOTION_WHEELWIND       : return {.count = 10};
        case MOTION_RANDSWING       : return {.count = 10};
        case MOTION_BACKDROPKICK    : return {.count = 10};
        case MOTION_DIE             : return {.count = 10};
        case MOTION_ONHORSEDIE      : return {.count = 10};
        case MOTION_WALK            : return {.count =  6};
        case MOTION_RUN             : return {.count =  6};
        case MOTION_MOODEPO         : return {.count =  6};
        case MOTION_ROLL            : return {.count = 10};
        case MOTION_FISHSTAND       : return {.count =  4};
        case MOTION_FISHHAND        : return {.count =  3};
        case MOTION_FISHTHROW       : return {.count =  8};
        case MOTION_FISHPULL        : return {.count =  8};
        case MOTION_ONHORSESTAND    : return {.count =  4};
        case MOTION_ONHORSEWALK     : return {.count =  6};
        case MOTION_ONHORSERUN      : return {.count =  6};
        case MOTION_ONHORSEHITTED   : return {.count =  3};
        default                     : return {};
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
                    nMotion = (onHorse() ? MOTION_ONHORSEWALK : MOTION_WALK);
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
            && (nWeapon    >= WEAPON_BEGIN && nWeapon    < WEAPON_END)
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >= DIR_BEGIN    && nDirection < DIR_END   )){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID >= 0){
            return ((nWeapon - WEAPON_BEGIN) << 9) + (nGfxMotionID << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return -1;
}

int Hero::GfxHairID(int nHair, int nMotion, int nDirection) const
{
    static_assert(sizeof(int) > 2, "GfxHairID() overflows because of sizeof(int) too small");
    if(true
            && (nHair      >= HAIR_BEGIN   && nHair      < HAIR_END  )
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >= DIR_BEGIN    && nDirection < DIR_END   )){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID >= 0){
            return ((nHair - HAIR_BEGIN /* hair gfx id start from 0 */) << 9) + (nGfxMotionID << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return -1;
}

int Hero::GfxDressID(int nDress, int nMotion, int nDirection) const
{
    static_assert(sizeof(int) > 2, "GfxDressID() overflows because of sizeof(int) too small");
    if(true
            && (nDress     >= DRESS_NONE   && nDress     < DRESS_END )  // support DRESS_NONE as naked
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >= DIR_BEGIN    && nDirection < DIR_END   )){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID >= 0){
            return ((nDress - DRESS_NONE) << 9) + (nGfxMotionID << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return -1;
}

int Hero::gfxHelmetID(int nHelmet, int nMotion, int nDirection) const
{
    static_assert(sizeof(int) > 2, "gfxHelmetID() overflows because of sizeof(int) too small");
    if(true
            && (nHelmet    >= HELMET_BEGIN && nHelmet    < HELMET_END)
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >= DIR_BEGIN    && nDirection < DIR_END   )){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID >= 0){
            return ((nHelmet - HELMET_BEGIN) << 9) + (nGfxMotionID << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return -1;
}

int Hero::currStep() const
{
    fflassert(motionValid(m_currMotion));
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

    const auto nDress     = getWLItem(WLG_DRESS).itemID;
    const auto nGender    = gender();
    const auto nMotion    = currMotion()->type;
    const auto nDirection = currMotion()->direction;

    const auto texBaseID = GfxDressID(nDress, nMotion, nDirection);
    if(texBaseID < 0){
        return {};
    }

    const uint32_t texID = ((to_u32(nGender ? 1 : 0)) << 22) + ((to_u32(texBaseID & 0X01FFFF)) << 5) + currMotion()->frame;

    int dx = 0;
    int dy = 0;
    auto bodyFrameTexPtr = g_heroDB->Retrieve(texID, &dx, &dy);

    if(!bodyFrameTexPtr){
        return {};
    }

    const auto [bodyFrameW, bodyFrameH] = SDLDeviceHelper::getTextureSize(bodyFrameTexPtr);

    const auto [shiftX, shiftY] = getShift(m_currMotion->frame);
    const int startX = m_currMotion->x * SYS_MAPGRIDXP + shiftX + dx;
    const int startY = m_currMotion->y * SYS_MAPGRIDYP + shiftY + dy;

    return getTargetBoxHelper(startX, startY, bodyFrameW, bodyFrameH);
}

const SDItem &Hero::getWLItem(int wltype) const
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wltype: %d", wltype);
    }
    return m_sdWLDesp.wear.getWLItem(wltype);
}

bool Hero::setWLItem(int wltype, SDItem item)
{
    if(!(wltype >= WLG_BEGIN && wltype < WLG_END)){
        throw fflerror("invalid wear/look type: %d", wltype);
    }

    if(item.itemID == 0){
        m_sdWLDesp.wear.setWLItem(wltype, {});
        return true;
    }

    if(!item){
        throw fflerror("invalid itemID: %llu", to_llu(item.itemID));
    }

    const auto &ir = DBCOM_ITEMRECORD(item.itemID);
    if(!ir){
        throw fflerror("invalid itemID: %llu", to_llu(item.itemID));
    }

    switch(wltype){
        case WLG_DRESS:
            {
                if((to_u8sv(ir.type) != u8"衣服") || (getClothGender(item.itemID) != gender())){
                    return false;
                }
                break;
            }
        default:
            {
                if(!ir.wearable(wltype)){
                    return false;
                }
                break;
            }
    }

    m_sdWLDesp.wear.setWLItem(wltype, std::move(item));
    return true;
}

void Hero::jumpLoc(int x, int y, int direction)
{
    m_currMotion.reset(new MotionNode
    {
        .type = [this]()
        {
            if(onHorse()){
                return MOTION_ONHORSESTAND;
            }
            return MOTION_STAND;
        }(),

        .direction = direction,
        .x = x,
        .y = y,
    });
}

bool Hero::deadFadeOut()
{
    switch(m_currMotion->type){
        case MOTION_DIE:
            {
                if(!m_currMotion->extParam.die.fadeOut){
                    m_currMotion->extParam.die.fadeOut = 1;
                }
                return true;
            }
        default:
            {
                return false; // TODO push an ActionDie here
            }
    }
}

void Hero::setBuff(int type, int state)
{
    switch(type){
        case BFT_SHIELD:
            {
                switch(state){
                    case BFS_OFF:
                        {
                            for(auto p = m_attachMagicList.begin(); p != m_attachMagicList.end();){
                                if(p->get()->magicID() == DBCOM_MAGICID(u8"魔法盾")){
                                    p = m_attachMagicList.erase(p);
                                }
                                else{
                                    p++;
                                }
                            }
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
            }
        default:
            {
                break;
            }
    }
}
