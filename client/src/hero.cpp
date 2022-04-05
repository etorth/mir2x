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

#include <cmath>
#include "log.hpp"
#include "hero.hpp"
#include "pathf.hpp"
#include "dbcomid.hpp"
#include "mathf.hpp"
#include "sysconst.hpp"
#include "soundeffectdb.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "motionnode.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_heroDB;
extern PNGTexOffDB *g_hairDB;
extern PNGTexOffDB *g_weaponDB;
extern PNGTexOffDB *g_helmetDB;
extern SoundEffectDB *g_seffDB;
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
        // 21 - 14 :    weapon : max = 256 : +----> gfxWeaponID
        //      22 :    gender :
        //      23 :    shadow :
        const auto nGfxWeaponID = gfxWeaponID(DBCOM_ITEMRECORD(getWLItem(WLG_WEAPON).itemID).shape, m_currMotion->type, m_currMotion->direction);
        if(!nGfxWeaponID.has_value()){
            return;
        }

        const uint32_t weaponKey = ((to_u32(shadow ? 1 : 0)) << 23) + (to_u32(gender()) << 22) + ((nGfxWeaponID.value() & 0X01FFFF) << 5) + frame;
        const auto [weaponFrame, weaponDX, weaponDY] = g_weaponDB->retrieve(weaponKey);

        if(weaponFrame && shadow){
            SDL_SetTextureAlphaMod(weaponFrame, 128);
        }
        g_sdlDevice->drawTexture(weaponFrame, startX + weaponDX, startY + weaponDY);
    };

    fnDrawWeapon(true);

    const auto [dressGfxIndex, modDressColor] = [this]() -> std::tuple<int, uint32_t>
    {
        if(const auto &dressItem = getWLItem(WLG_DRESS)){
            return {DBCOM_ITEMRECORD(dressItem.itemID).shape, dressItem.getExtAttr<uint32_t>(SDItem::EA_COLOR).value_or(0XFFFFFFFF)};
        }
        return {DRESS_BEGIN, 0XFFFFFFFF}; // naked
    }();

    const auto nGfxDressID = gfxDressID(dressGfxIndex, m_currMotion->type, m_currMotion->direction);
    if(!nGfxDressID.has_value()){
        m_currMotion->print([](const std::string &s)
        {
            g_log->addLog(LOGTYPE_WARNING, "%s", s.c_str());
        });
        return;
    }

    // human gfx dress id indexing
    // 04 - 00 :     frame : max =  32
    // 07 - 05 : direction : max =  08 : +
    // 13 - 08 :    motion : max =  64 : +
    // 21 - 14 :     dress : max = 256 : +----> gfxDressID
    //      22 :       sex :
    //      23 :    shadow :
    //      24 :     layer :
    const uint32_t   bodyKey = (to_u32(0) << 23) + (to_u32(gender()) << 22) + ((to_u32(nGfxDressID.value() & 0X01FFFF)) << 5) + frame;
    const uint32_t shadowKey = (to_u32(1) << 23) + (to_u32(gender()) << 22) + ((to_u32(nGfxDressID.value() & 0X01FFFF)) << 5) + frame;

    const auto [ bodyLayer0,  body0DX,  body0DY] = g_heroDB->retrieve(bodyKey);
    const auto [ bodyLayer1,  body1DX,  body1DY] = g_heroDB->retrieve(bodyKey | (to_u32(1) << 24));
    const auto [shadowFrame, shadowDX, shadowDY] = g_heroDB->retrieve(shadowKey);

    if(shadowFrame){
        SDL_SetTextureAlphaMod(shadowFrame, 128);
    }
    g_sdlDevice->drawTexture(shadowFrame, startX + shadowDX, startY + shadowDY);

    if(true
            && getWLItem(WLG_WEAPON)
            && weaponOrder(m_currMotion->type, m_currMotion->direction, frame).value_or(-1) == 1){
        fnDrawWeapon(false);
    }

    for(auto &p: m_attachMagicList){
        switch(p->magicID()){
            case DBCOM_MAGICID(u8"魔法盾"):
                {
                    p->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XF0));
                    break;
                }
            default:
                {
                    p->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF));
                    break;
                }
        }
    }

    g_sdlDevice->drawTexture(bodyLayer0, startX + body0DX, startY + body0DY);
    if(bodyLayer1){
        SDLDeviceHelper::EnableTextureModColor modColor(bodyLayer1, modDressColor);
        g_sdlDevice->drawTexture(bodyLayer1, startX + body1DX, startY + body1DY);
    }

    if(getWLItem(WLG_HELMET)){
        if(const auto nHelmetGfxID = gfxHelmetID(DBCOM_ITEMRECORD(getWLItem(WLG_HELMET).itemID).shape, m_currMotion->type, m_currMotion->direction); nHelmetGfxID.has_value()){
            const uint32_t nHelmetKey = (to_u32(gender()) << 22) + ((to_u32(nHelmetGfxID.value() & 0X01FFFF)) << 5) + frame;
            if(auto [texPtr, dx, dy] = g_helmetDB->retrieve(nHelmetKey); texPtr){
                g_sdlDevice->drawTexture(texPtr, startX + dx, startY + dy);
            }
        }
    }
    else{
        if(const auto nHairGfxID = gfxHairID(m_sdWLDesp.hair, m_currMotion->type, m_currMotion->direction); nHairGfxID.has_value()){
            const uint32_t nHairKey = (to_u32(gender()) << 22) + ((to_u32(nHairGfxID.value() & 0X01FFFF)) << 5) + frame;
            if(auto [texPtr, dx, dy] = g_hairDB->retrieve(nHairKey); texPtr){
                SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, m_sdWLDesp.hairColor);
                g_sdlDevice->drawTexture(texPtr, startX + dx, startY + dy);
            }
        }
    }

    if(true
            && getWLItem(WLG_WEAPON)
            && weaponOrder(m_currMotion->type, m_currMotion->direction, frame).value_or(-1) == 0){
        fnDrawWeapon(false);
    }

    if(g_clientArgParser->drawTextureAlignLine){
        g_sdlDevice->drawLine (colorf::RED  + colorf::A_SHF(128), startX, startY, startX + body0DX, startY + body0DY);
        g_sdlDevice->drawCross(colorf::BLUE + colorf::A_SHF(128), startX, startY, 5);

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(bodyLayer0);
        g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(128), startX + body0DX, startY + body0DY, texW, texH);
    }

    if(g_clientArgParser->drawTargetBox){
        if(const auto box = getTargetBox()){
            g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(128), box.x - viewX, box.y - viewY, box.w, box.h);
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
                                p->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF));
                                break;
                            }
                    }
                    break;
                }
            default:
                {
                    p->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XF0));
                    break;
                }
        }
    }

    if(m_currMotion->effect && !m_currMotion->effect->done()){
        m_currMotion->effect->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF));
    }

    // draw HP bar
    // if current m_HPMqx is zero we draw full bar
    if(m_currMotion->type != MOTION_DIE && g_clientArgParser->drawHPBar){
        auto bar0Ptr = g_progUseDB->retrieve(0X00000014);
        auto bar1Ptr = g_progUseDB->retrieve(0X00000015);

        const auto [bar1TexW, bar1TexH] = SDLDeviceHelper::getTextureSize(bar1Ptr);
        const int drawHPX = startX +  7;
        const int drawHPY = startY - 53;
        const int drawHPW = to_d(std::lround(bar1TexW * getHealthRatio().at(0)));

        g_sdlDevice->drawTexture(bar1Ptr, drawHPX, drawHPY, 0, 0, drawHPW, bar1TexH);
        g_sdlDevice->drawTexture(bar0Ptr, drawHPX, drawHPY);

        constexpr int buffIconDrawW = 10;
        constexpr int buffIconDrawH = 10;

        const int buffIconStartX = drawHPX + 1;
        const int buffIconStartY = drawHPY - buffIconDrawH;

        if(getSDBuffIDList().has_value()){
            for(int drawIconCount = 0; const auto id: getSDBuffIDList().value().idList){
                const auto &br = DBCOM_BUFFRECORD(id);
                fflassert(br);

                if(br.icon.gfxID != SYS_TEXNIL){
                    if(auto iconTexPtr = g_progUseDB->retrieve(br.icon.gfxID)){
                        const int buffIconOffX = buffIconStartX + (drawIconCount % 3) * buffIconDrawW;
                        const int buffIconOffY = buffIconStartY - (drawIconCount / 3) * buffIconDrawH;

                        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(iconTexPtr);
                        g_sdlDevice->drawTexture(iconTexPtr, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, 0, 0, texW, texH);

                        const auto baseColor = [&br]() -> uint32_t
                        {
                            if(br.favor > 0){
                                return colorf::GREEN;
                            }
                            else if(br.favor == 0){
                                return colorf::YELLOW;
                            }
                            else{
                                return colorf::RED;
                            }
                        }();

                        const auto startColor = baseColor | colorf::A_SHF(255);
                        const auto   endColor = baseColor | colorf::A_SHF( 64);

                        const auto edgeGridCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                        const auto startLoc = std::lround(edgeGridCount * std::fmod(m_accuUpdateTime, 1500.0) / 1500.0);

                        g_sdlDevice->drawBoxFading(startColor, endColor, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, startLoc, buffIconDrawW + buffIconDrawH);
                        drawIconCount++;
                    }
                }
            }
        }
    }
}

bool Hero::update(double ms)
{
    updateAttachMagic(ms);
    const CallOnExitHelper motionOnUpdate([lastSeqFrameID = m_currMotion->getSeqFrameID(), this]()
    {
        m_currMotion->runTrigger();

        // check soundeffectdb.cpp for more detailed comments
        //
        // TODO: 1. need to check ground type
        //       2. need to add positional sound effect

        if(lastSeqFrameID == m_currMotion->getSeqFrameID()){
            return;
        }

        const auto fnPlayStepSound = [this](uint32_t seffBaseID)
        {
            if(m_currMotion->frame == 1){
                g_sdlDevice->playSoundEffect(g_seffDB->retrieve(seffBaseID));
            }
            else if(m_currMotion->frame == 4){
                g_sdlDevice->playSoundEffect(g_seffDB->retrieve(seffBaseID + 1));
            }
        };

        const auto fnPlayAttackSound = [this]()
        {
            if(m_currMotion->frame == 0){
                if(const auto weaponItemID = getWLItem(WLG_WEAPON).itemID){
                    const auto &ir = DBCOM_ITEMRECORD(weaponItemID);
                    fflassert(ir);

                    if     (ir.equip.weapon.category == u8"匕首") g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 50));
                    else if(ir.equip.weapon.category == u8"木剑") g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 51));
                    else if(ir.equip.weapon.category == u8"剑"  ) g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 52));
                    else if(ir.equip.weapon.category == u8"刀"  ) g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 53));
                    else if(ir.equip.weapon.category == u8"斧"  ) g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 54));
                    else if(ir.equip.weapon.category == u8"锏"  ) g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 55));
                    else if(ir.equip.weapon.category == u8"棍"  ) g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 56));
                    else                                          g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 57)); // anything else, currently use bare-hand
                }
                else{
                    g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 57)); // bare-hand
                }
            }
        };

        const auto fnPlayHittedSound = [this]() // TODO: check attacker's weapon category
        {
            if(const auto dressItemID = getWLItem(WLG_DRESS).itemID; DBCOM_ITEMRECORD(dressItemID)){
                g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 80));
            }
            else{
                g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01010000 + 70));
            }
            g_sdlDevice->playSoundEffect(g_seffDB->retrieve(0X01030000 + (gender() ? 138 : 139)));
        };

        switch(m_currMotion->type){
            case MOTION_WALK        : fnPlayStepSound(0X01000000 +  1); break;
            case MOTION_RUN         : fnPlayStepSound(0X01000000 +  3); break;
            case MOTION_ONHORSEWALK : fnPlayStepSound(0X01000000 + 33); break;
            case MOTION_ONHORSERUN  : fnPlayStepSound(0X01000000 + 35); break;
            case MOTION_ONEHSWING   :
            case MOTION_ONEVSWING   :
            case MOTION_TWOHSWING   :
            case MOTION_TWOVSWING   :
            case MOTION_RANDSWING   :
            case MOTION_SPEARHSWING :
            case MOTION_SPEARVSWING : fnPlayAttackSound(); break;
            case MOTION_HITTED      : fnPlayHittedSound(); break;
            default: break;
        }
    });

    if(m_currMotion->effect && !m_currMotion->effect->done()){
        m_currMotion->effect->update(ms);
        return true;
    }

    if(!checkUpdate(ms)){
        return true;
    }

    switch(m_currMotion->type){
        case MOTION_STAND:
            {
                if(stayIdle()){
                    return advanceMotionFrame();
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
                    return updateMotion(true);
                }

                // move to next motion will reset frame as 0
                // if current there is no more motion pending
                // it will add a MOTION_STAND
                //
                // we don't want to reset the frame here
                return moveNextMotion();
            }
        default:
            {
                return updateMotion(true);
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
            && motionPtr->frame <  getFrameCount(motionPtr->type, motionPtr->direction)){

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
            case MOTION_SPINKICK:
                {
                    return !onHorse() && (nLDistance2 == 0);
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
        case ACTION_SPINKICK:
            {
                m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
                break;
            }
        case ACTION_SPACEMOVE:
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
        case ACTION_SPINKICK:
            {
                m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                {
                    .type = MOTION_SPINKICK,
                    .direction = [&action, this]() -> int
                    {
                        if(action.aimUID){
                            if(auto coPtr = m_processRun->findUID(action.aimUID)){
                                if(mathf::CDistance<int>(coPtr->x(), coPtr->y(), x(), y()) == 1){
                                    return PathFind::GetDirection(coPtr->x(), coPtr->y(), x(), y());
                                }
                            }
                        }
                        return m_currMotion->direction;
                    }(),
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

                        m_motionQueue.back()->addTrigger(false, [this](MotionNode *motionPtr) -> bool
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
        case ACTION_SPACEMOVE:
            {
                flushForcedMotion();
                jumpLoc(action.aimX, action.aimY, m_currMotion->direction);
                m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic(u8"瞬息移动", u8"开始", action.x, action.y)));
                addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"结束")));
                break;
            }
        case ACTION_SPELL:
            {
                const auto magicID = action.extParam.spell.magicID;
                if(auto &mr = DBCOM_MAGICRECORD(magicID)){
                    if(auto &gfxEntry = mr.getGfxEntry(u8"启动").first){
                        const auto motionSpell = [&gfxEntry]() -> int
                        {
                            switch(gfxEntry.motion){
                                case MOTION_SPELL0:
                                case MOTION_SPELL1:
                                case MOTION_ATTACKMODE: return gfxEntry.motion;
                                default: throw fflreach();
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

                        const auto standDir = [&gfxEntry, &fnGetSpellDir, &action, this]() -> int
                        {
                            if(gfxEntry.motion == MOTION_ATTACKMODE){
                                return DIR_DOWN;
                            }

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
                        }));

                        m_motionQueue.back()->effect = std::unique_ptr<HeroSpellMagicEffect>(new HeroSpellMagicEffect
                        {
                            to_u8cstr(DBCOM_MAGICRECORD(magicID).name),
                            this,
                            m_motionQueue.back().get(),
                        });

                        if(UID() == m_processRun->getMyHeroUID()){
                            m_processRun->getMyHero()->setMagicCastTime(magicID);
                        }

                        switch(magicID){
                            case DBCOM_MAGICID(u8"冰沙掌"):
                            case DBCOM_MAGICID(u8"地狱火"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [standDir, magicID, this](MotionNode *motionPtr) -> bool
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
                                                    }))->addTrigger([aimX, aimY, this](BaseMagic *magicPtr)
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
                                                    throw fflreach();
                                                }
                                            });
                                        }
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"风震天"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [standDir, magicID, this](MotionNode *motionPtr) -> bool
                                    {
                                        if(motionPtr->frame < 4){
                                            return false;
                                        }

                                        const auto castX = motionPtr->endX;
                                        const auto castY = motionPtr->endY;
                                        const auto [aimX, aimY] = pathf::getFrontGLoc(castX, castY, standDir, 1);
                                        m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                                        {
                                            u8"风震天",
                                            u8"开始",

                                            aimX,
                                            aimY,
                                        }))->addOnDone([standDir, this](BaseMagic *magicPtr)
                                        {
                                            auto fnAddNextStep = +[](BaseMagic *magicPtr, int standDir, ProcessRun *proc, std::shared_ptr<int> magicRanPtr, void *selfFuncPtr)
                                            {
                                                const auto fixedLocMagicPtr = dynamic_cast<FixedLocMagic *>(magicPtr);
                                                const auto [nextX, nextY] = pathf::getFrontGLoc(fixedLocMagicPtr->x(), fixedLocMagicPtr->y(), standDir, 1);

                                                if(*magicRanPtr >= 5){
                                                    proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                                                    {
                                                        u8"风震天",
                                                        u8"结束",

                                                        nextX,
                                                        nextY,
                                                    }));
                                                }
                                                else{
                                                    (*magicRanPtr)++;
                                                    proc->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic
                                                    {
                                                        u8"风震天",
                                                        u8"运行",

                                                        nextX,
                                                        nextY,
                                                    }))->addOnDone([proc, magicRanPtr, standDir, selfFuncPtr](BaseMagic *magicPtr)
                                                    {
                                                        ((void (*)(BaseMagic *, int, ProcessRun *, std::shared_ptr<int>, void *))(selfFuncPtr))(magicPtr, standDir, proc, magicRanPtr, selfFuncPtr);
                                                    });
                                                }
                                            };
                                            fnAddNextStep(magicPtr, standDir, m_processRun, std::make_shared<int>(0), (void *)(fnAddNextStep));
                                        });
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"疾光电影"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [standDir, this](MotionNode *motionPtr) -> bool
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
                            case DBCOM_MAGICID(u8"乾坤大挪移"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [aimUID = action.aimUID, this](MotionNode *motionPtr) -> bool
                                    {
                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        if(auto coPtr = m_processRun->findUID(aimUID)){
                                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"乾坤大挪移", u8"运行")));
                                        }
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"治愈术"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [magicID, action, this](MotionNode *motionPtr) -> bool
                                    {
                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        const auto coPtr = [action, this]() -> ClientCreature *
                                        {
                                            if(auto coPtr = m_processRun->findUID(action.aimUID)){
                                                return coPtr;
                                            }
                                            else{
                                                return this;
                                            }
                                        }();

                                        coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(DBCOM_MAGICRECORD(magicID).name, u8"运行")));
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"圣言术"):
                            case DBCOM_MAGICID(u8"云寂术"):
                            case DBCOM_MAGICID(u8"回生术"):
                            case DBCOM_MAGICID(u8"施毒术"):
                            case DBCOM_MAGICID(u8"诱惑之光"):
                            case DBCOM_MAGICID(u8"移花接玉"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [magicID, action, this](MotionNode *motionPtr) -> bool
                                    {
                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        if(auto coPtr = m_processRun->findUID(action.aimUID)){
                                            coPtr->addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(DBCOM_MAGICRECORD(magicID).name, u8"运行")));
                                        }
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"击风"  ):
                            case DBCOM_MAGICID(u8"冰咆哮"):
                            case DBCOM_MAGICID(u8"龙卷风"):
                            case DBCOM_MAGICID(u8"爆裂火焰"):
                            case DBCOM_MAGICID(u8"地狱雷光"):
                            case DBCOM_MAGICID(u8"怒神霹雳"):
                            case DBCOM_MAGICID(u8"群体治愈术"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [magicID, action, this](MotionNode *motionPtr) -> bool
                                    {
                                        if(motionPtr->frame < 3){
                                            return false;
                                        }

                                        const auto [aimX, aimY] = [action, this]() -> std::tuple<int, int>
                                        {
                                            if(auto coPtr = m_processRun->findUID(action.aimUID)){
                                                return {coPtr->currMotion()->endX, coPtr->currMotion()->endY};
                                            }
                                            else{
                                                return {action.aimX, action.aimY};
                                            }
                                        }();

                                        auto addedMagic = m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic(DBCOM_MAGICRECORD(magicID).name, u8"运行", aimX, aimY)));
                                        if(magicID == DBCOM_MAGICID(u8"击风")){
                                            addedMagic->addOnDone([aimX, aimY, magicID, this](BaseMagic *)
                                            {
                                                m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic(DBCOM_MAGICRECORD(magicID).name, u8"结束", aimX, aimY)));
                                            });
                                        }
                                        return true;
                                    });
                                    break;
                                }
                            case DBCOM_MAGICID(u8"火球术"):
                            case DBCOM_MAGICID(u8"大火球"):
                            case DBCOM_MAGICID(u8"霹雳掌"):
                            case DBCOM_MAGICID(u8"风掌"  ):
                            case DBCOM_MAGICID(u8"月魂断玉"):
                            case DBCOM_MAGICID(u8"月魂灵波"):
                            case DBCOM_MAGICID(u8"灵魂火符"):
                            case DBCOM_MAGICID(u8"冰月神掌"):
                            case DBCOM_MAGICID(u8"冰月震天"):
                            case DBCOM_MAGICID(u8"幽灵盾"):
                            case DBCOM_MAGICID(u8"神圣战甲术"):
                            case DBCOM_MAGICID(u8"强魔震法"):
                            case DBCOM_MAGICID(u8"猛虎强势"):
                            case DBCOM_MAGICID(u8"集体隐身术"):
                                {
                                    m_motionQueue.back()->addTrigger(true, [targetUID = action.aimUID, magicID, this](MotionNode *motionPtr) -> bool
                                    {
                                        // usually when reaches this cb the current motion is motionPtr
                                        // but not true if in flushForcedMotion()

                                        if(motionPtr->frame < 4){
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
                                                case DBCOM_MAGICID(u8"月魂断玉"): return 0;
                                                case DBCOM_MAGICID(u8"月魂灵波"): return 0;
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

                                        }))->addOnDone([targetUID, magicID, proc = m_processRun](BaseMagic *)
                                        {
                                            if(auto coPtr = proc->findUID(targetUID)){
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
                        const auto magicID = action.extParam.attack.magicID;
                        const auto [swingMotion, motionSpeed, magicName, lagFrame] = [magicID, this]() -> std::tuple<int, int, const char8_t *, int>
                        {
                            const auto doubleHanded = [this]() -> bool
                            {
                                if(const auto &weaponItem = getWLItem(WLG_WEAPON); weaponItem){
                                    const auto &ir = DBCOM_ITEMRECORD(weaponItem.itemID);
                                    fflassert(ir);
                                    return ir.equip.weapon.doubleHand;
                                }
                                return false;
                            }();

                            switch(magicID){
                                case DBCOM_MAGICID(u8"烈火剑法"): return {doubleHanded ? MOTION_TWOVSWING : MOTION_ONEVSWING, 100, u8"烈火剑法", 0};
                                case DBCOM_MAGICID(u8"翔空剑法"): return {                                  MOTION_RANDSWING, 100, u8"翔空剑法", 0};
                                case DBCOM_MAGICID(u8"莲月剑法"): return {                                  MOTION_RANDSWING, 100, u8"莲月剑法", 0};
                                case DBCOM_MAGICID(u8"半月弯刀"): return {doubleHanded ? MOTION_TWOHSWING : MOTION_ONEHSWING, 100, u8"半月弯刀", 0};
                                case DBCOM_MAGICID(u8"十方斩"  ): return {                                  MOTION_WHEELWIND, 150, u8"十方斩"  , 3}; // 十方斩 has 6 frames while MOTION_WHEELWIND has 10
                                case DBCOM_MAGICID(u8"攻杀剑术"): return {doubleHanded ? MOTION_TWOVSWING : MOTION_ONEVSWING, 100, u8"攻杀剑术", 0};
                                case DBCOM_MAGICID(u8"刺杀剑术"): return {doubleHanded ? MOTION_TWOVSWING : MOTION_ONEVSWING, 100, u8"刺杀剑术", 0};
                                default                         : return {doubleHanded ? MOTION_TWOVSWING : MOTION_ONEVSWING, 100, u8"物理攻击", 0};
                            }
                        }();

                        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
                        {
                            .type = swingMotion,
                            .direction = attackDir,
                            .speed = motionSpeed,
                            .x = action.x,
                            .y = action.y,
                        }));

                        if(to_u8sv(magicName) != u8"物理攻击"){
                            m_motionQueue.back()->effect.reset(new MotionSyncEffect(magicName, u8"运行", this, m_motionQueue.back().get(), lagFrame));
                            if(UID() == m_processRun->getMyHeroUID()){
                                m_processRun->getMyHero()->setMagicCastTime(magicID);
                            }
                        }

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

                m_motionQueue.front()->addTrigger(true, [this](MotionNode *) -> bool
                {
                    for(auto &p: m_attachMagicList){
                        if(p->magicID() == DBCOM_MAGICID(u8"魔法盾")){
                            // here we replace the old 魔法盾 with a new one
                            // it's possible the old one haven't trigger its onDone callback yet, but doesn't matter
                            p.reset(new AttachMagic(u8"魔法盾", u8"挨打"));
                            p->addOnDone([this](BaseMagic *)
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
                const auto [dieX, dieY, dieDir] = motionEndGLoc().at(1);
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

HeroFrameGfxSeq Hero::getFrameGfxSeq(int motion, int direction) const
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
        case MOTION_SPINKICK        : return {.count = 10};
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

std::optional<int> Hero::weaponOrder(int nMotion, int nDirection, int nFrame)
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

    constexpr static int s_weaponOrder[2640]
    {
        #include "weaponorder.inc"
    };

    if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID.has_value()){
        return s_weaponOrder[nGfxMotionID.value() * 80 + (nDirection - DIR_BEGIN) * 10 + nFrame];
    }
    return {};
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

std::optional<uint32_t> Hero::gfxWeaponID(int nWeapon, int nMotion, int nDirection) const
{
    if(true
            && (nWeapon    >= WEAPON_BEGIN && nWeapon    < WEAPON_END)
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >=    DIR_BEGIN && nDirection <    DIR_END)){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID.has_value()){
            return ((nWeapon - WEAPON_BEGIN) << 9) + (nGfxMotionID.value() << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return {};
}

std::optional<uint32_t> Hero::gfxHairID(int nHair, int nMotion, int nDirection) const
{
    if(true
            && (nHair      >=   HAIR_BEGIN && nHair      <   HAIR_END)
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >=    DIR_BEGIN && nDirection <    DIR_END)){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID.has_value()){
            return ((nHair - HAIR_BEGIN /* hair gfx id start from 0 */) << 9) + (nGfxMotionID.value() << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return {};
}

std::optional<uint32_t> Hero::gfxDressID(int nDress, int nMotion, int nDirection) const
{
    if(true
            && (nDress     >=  DRESS_BEGIN && nDress     <  DRESS_END)  // DRESS_BEGIN is naked
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >=    DIR_BEGIN && nDirection <    DIR_END)){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID.has_value()){
            return ((nDress - DRESS_BEGIN) << 9) + (nGfxMotionID.value() << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return {};
}

std::optional<uint32_t> Hero::gfxHelmetID(int nHelmet, int nMotion, int nDirection) const
{
    if(true
            && (nHelmet    >= HELMET_BEGIN && nHelmet    < HELMET_END)
            && (nMotion    >= MOTION_BEGIN && nMotion    < MOTION_END)
            && (nDirection >=    DIR_BEGIN && nDirection <    DIR_END)){
        if(const auto nGfxMotionID = gfxMotionID(nMotion); nGfxMotionID.has_value()){
            return ((nHelmet - HELMET_BEGIN) << 9) + (nGfxMotionID.value() << 3) + (nDirection - DIR_BEGIN);
        }
    }
    return {};
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

    const auto dressGfxIndex = [this]() -> int
    {
        if(const auto &dressItem = getWLItem(WLG_DRESS)){
            return DBCOM_ITEMRECORD(dressItem.itemID).shape;
        }
        return DRESS_BEGIN; // naked
    }();

    const auto texBaseID = gfxDressID(dressGfxIndex, currMotion()->type, currMotion()->direction);
    if(!texBaseID.has_value()){
        return {};
    }

    const uint32_t texID = ((to_u32(gender() ? 1 : 0)) << 22) + ((to_u32(texBaseID.value() & 0X01FFFF)) << 5) + currMotion()->frame;

    auto [bodyFrameTexPtr, dx, dy] = g_heroDB->retrieve(texID);
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
                if((to_u8sv(ir.type) != u8"衣服") || (ir.clothGender().value() != gender())){
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

bool Hero::hasSwingMagic(uint32_t magicID) const
{
    return m_swingMagicList.count(magicID);
}

void Hero::toggleSwingMagic(uint32_t magicID, std::optional<bool> enable)
{
    if(enable.has_value()){
        if(enable.value()){
            m_swingMagicList.insert(magicID);
        }
        else{
            m_swingMagicList.erase(magicID);
        }
    }
    else{
        if(m_swingMagicList.count(magicID)){
            m_swingMagicList.erase(magicID);
        }
        else{
            m_swingMagicList.insert(magicID);
        }
    }
}
