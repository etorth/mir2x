#include <SDL2/SDL.h>
#include <algorithm>
#include "log.hpp"
#include "pathf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"
#include "uidf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"
#include "clientpathfinder.hpp"
#include "creaturemovable.hpp"
#include "clienttaodog.hpp"
#include "clientsandcactus.hpp"
#include "clienttaoskeleton.hpp"
#include "clienttaoskeletonext.hpp"
#include "clientbugbatmaggot.hpp"
#include "clientcannibalplant.hpp"
#include "clientdualaxeskeleton.hpp"
#include "clientguard.hpp"
#include "clientsandghost.hpp"
#include "clientsandstoneman.hpp"
#include "clientwedgemoth.hpp"
#include "clientlightboltzombie.hpp"
#include "clientmonkzombie.hpp"
#include "clientrebornzombie.hpp"
#include "clientanthealer.hpp"
#include "clientnumawizard.hpp"
#include "clientredclothwizard.hpp"
#include "clientcavemaggot.hpp"
#include "clientwoomaflamingwarrior.hpp"
#include "clientwoomataurus.hpp"
#include "clientdarkwarrior.hpp"
#include "clientdung.hpp"
#include "clientevilcentipede.hpp"
#include "clientzumaarcher.hpp"
#include "clientzumamonster.hpp"
#include "clientzumataurus.hpp"
#include "clientgasant.hpp"
#include "clientsandevilfan.hpp"
#include "clientbombspider.hpp"
#include "clientshipwrecklord.hpp"
#include "clientminotaurguardian.hpp"
#include "clienttree.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_monsterDB;
extern ClientArgParser *g_clientArgParser;

std::optional<uint32_t> MonsterFrameGfxSeq::gfxID(const ClientMonster *monPtr, std::optional<int> frameOpt) const
{
    fflassert(monPtr);
    if(*this){
        // monster graphics retrieving key structure
        //
        //   3322 2222 2222 1111 1111 1100 0000 0000
        //   1098 7654 3210 9876 5432 1098 7654 3210
        //   ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^
        //   |||| |||| |||| |||| |||| |||| |||| ||||
        //             |||| |||| |||| |||| |||+-++++-----------     frame : max =   32
        //             |||| |||| |||| |||| +++----------------- direction : max =    8 -+
        //             |||| |||| |||| ++++---------------------    motion : max =   16 -+
        //             |+++-++++-++++--------------------------      look : max = 2048 -+------> gfxBaseID
        //             +---------------------------------------    shadow : max =    2
        //

        const auto frame = frameOpt.value_or(monPtr->currMotion()->frame);
        fflassert(frame >= 0);
        fflassert(frame < count);

        const auto          gfxFrame = to_u32(begin + frame * (reverse ? -1 : 1));
        const auto      lookGfxIndex = to_u32(gfxLookID.value_or(monPtr->getMR().lookID) - LID_BEGIN);
        const auto    motionGfxIndex = to_u32(gfxMotionID.value_or(monPtr->currMotion()->type) - MOTION_MON_BEGIN);
        const auto directionGfxIndex = to_u32(gfxDirectionID.value_or(monPtr->currMotion()->direction) - DIR_BEGIN);

        return 0
            + ((     lookGfxIndex & 0X07FF) << 12)
            + ((   motionGfxIndex & 0X000F) <<  8)
            + ((directionGfxIndex & 0X0007) <<  5)
            + ((         gfxFrame & 0X001F) <<  0);
    }
    return {};
}

ClientMonster::ClientMonster(uint64_t uid, ProcessRun *proc)
    : CreatureMovable(uid, proc)
{
    fflassert(uidf::getUIDType(uid) == UID_MON, uidf::getUIDString(uid));
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
    const CallOnExitHelper motionOnUpdate([lastSeqFrameID = m_currMotion->getSeqFrameID(), this]()
    {
        m_currMotion->runTrigger();
        if(lastSeqFrameID == m_currMotion->getSeqFrameID()){
            return;
        }

        switch(m_currMotion->type){
            case MOTION_MON_SPAWN:
                {
                    if(m_currMotion->frame == 0){
                        playSoundEffect(getSeffID(MONSEFF_SPAWN));
                    }
                    break;
                }
            case MOTION_MON_STAND: // shall only play when show up
                {
                    break;
                }
            case MOTION_MON_ATTACK0:
            case MOTION_MON_ATTACK1:
                {
                    if(m_currMotion->frame == 0){
                        playSoundEffect(getSeffID(MONSEFF_ATTACK));
                    }
                    break;
                }
            case MOTION_MON_HITTED:
                {
                    if(m_currMotion->frame == 0){
                        playSoundEffect(getSeffID(MONSEFF_HITTED));
                        switch(const auto fromUID = m_currMotion->extParam.hitted.fromUID; uidf::getUIDType(fromUID)){
                            case UID_MON:
                                {
                                    playSoundEffect(0X01010000 + 61);
                                    break;
                                }
                            case UID_PLY:
                                {
                                    playSoundEffect([fromUID, this]() -> uint32_t
                                    {
                                        if(const auto plyPtr = m_processRun->findUID(fromUID)){
                                            if(const auto itemID = dynamic_cast<const Hero *>(plyPtr)->getWLItem(WLG_WEAPON).itemID){
                                                const auto &ir = DBCOM_ITEMRECORD(itemID);
                                                fflassert(ir);

                                                if     (ir.equip.weapon.category == u8"匕首") return 0X01010000 + 60;
                                                else if(ir.equip.weapon.category == u8"木剑") return 0X01010000 + 61;
                                                else if(ir.equip.weapon.category == u8"剑"  ) return 0X01010000 + 62;
                                                else if(ir.equip.weapon.category == u8"刀"  ) return 0X01010000 + 63;
                                                else if(ir.equip.weapon.category == u8"斧"  ) return 0X01010000 + 64;
                                                else if(ir.equip.weapon.category == u8"锏"  ) return 0X01010000 + 65;
                                            }
                                        }
                                        return 0X01010000 + 61; // use 木剑 sound effect as default
                                    }());
                                    break;
                                }
                            default:
                                {
                                    break;
                                }
                        }
                    }
                    break;
                }
            case MOTION_MON_DIE:
                {
                    if(m_currMotion->frame == 0){
                        playSoundEffect(getSeffID(MONSEFF_DIE));
                    }
                    break;
                }
            default:
                {
                    break;
                }
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
        case MOTION_MON_STAND:
            {
                if(stayIdle()){
                    return advanceMotionFrame();
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
                const auto frameCount = getFrameCount(m_currMotion.get());
                if(frameCount <= 0){
                    return false;
                }

                if(m_currMotion->frame + 1 < frameCount){
                    return advanceMotionFrame();
                }

                switch(m_currMotion->extParam.die.fadeOut){
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
                            nextFadeOut = (std::max<int>)(1, m_currMotion->extParam.die.fadeOut + 10);
                            nextFadeOut = (std::min<int>)(nextFadeOut, 255);

                            m_currMotion->extParam.die.fadeOut = nextFadeOut;
                            break;
                        }
                }
                return true;
            }
        default:
            {
                return updateMotion(true);
            }
    }
}

void ClientMonster::drawFrame(int viewX, int viewY, int focusMask, int frame, bool frameOnly)
{
    const auto gfxBodyIDOpt = getFrameGfxSeq(m_currMotion->type, m_currMotion->direction).gfxID(this, frame);
    if(!gfxBodyIDOpt.has_value()){
        return;
    }

    const uint32_t   bodyKey = (to_u32(0) << 23) + gfxBodyIDOpt.value(); // body
    const uint32_t shadowKey = (to_u32(1) << 23) + gfxBodyIDOpt.value(); // shadow

    const auto [  bodyFrame,   bodyDX,   bodyDY] = g_monsterDB->retrieve(  bodyKey);
    const auto [shadowFrame, shadowDX, shadowDY] = g_monsterDB->retrieve(shadowKey);
    const auto [shiftX, shiftY] = getShift(frame);

    // always reset the alpha mode for each texture because texture is shared
    // one texture to draw can be configured with different alpha mode for other creatures

    if(bodyFrame){
        SDL_SetTextureAlphaMod(bodyFrame, 255);
    }

    if(shadowFrame){
        SDL_SetTextureAlphaMod(shadowFrame, 128);
    }

    if(true
            && (m_currMotion->type  == MOTION_MON_DIE)
            && (m_currMotion->extParam.die.fadeOut  > 0)){
        // FadeOut :    0 : normal
        //         : 1-255: fadeOut
        if(bodyFrame){
            SDL_SetTextureAlphaMod(bodyFrame, (255 - m_currMotion->extParam.die.fadeOut) / 1);
        }

        if(shadowFrame){
            SDL_SetTextureAlphaMod(shadowFrame, (255 - m_currMotion->extParam.die.fadeOut) / 2);
        }
    }

    const auto fnBlendFrame = [](SDL_Texture *pTexture, int nFocusChan, int nX, int nY)
    {
        if(true
                && pTexture
                && nFocusChan >= 0
                && nFocusChan <  FOCUS_END){

            // if provided channel as 0
            // just blend it using the original color

            const auto stColor = focusColor(nFocusChan);
            if(!SDL_SetTextureColorMod(pTexture, stColor.r, stColor.g, stColor.b)){
                g_sdlDevice->drawTexture(pTexture, nX, nY);
            }
        }
    };

    const int startX = currMotion()->x * SYS_MAPGRIDXP + shiftX - viewX;
    const int startY = currMotion()->y * SYS_MAPGRIDYP + shiftY - viewY;

    if(getMR().shadow){
        fnBlendFrame(shadowFrame, 0, startX + shadowDX, startY + shadowDY);
    }
    fnBlendFrame(bodyFrame, 0, startX + bodyDX, startY + bodyDY);

    if(!frameOnly){
        if(g_clientArgParser->drawTextureAlignLine){
            g_sdlDevice->drawLine (colorf::RED  + colorf::A_SHF(128), startX, startY, startX + bodyDX, startY + bodyDY);
            g_sdlDevice->drawCross(colorf::BLUE + colorf::A_SHF(128), startX, startY, 5);

            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(bodyFrame);
            g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(128), startX + bodyDX, startY + bodyDY, texW, texH);
        }

        if(g_clientArgParser->drawTargetBox){
            if(const auto box = getTargetBox()){
                g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(128), box.x - viewX, box.y - viewY, box.w, box.h);
            }
        }
    }

    for(int nFocusChan = 1; nFocusChan < FOCUS_END; ++nFocusChan){
        if(focusMask & (1 << nFocusChan)){
            fnBlendFrame(bodyFrame, nFocusChan, startX + bodyDX, startY + bodyDY);
        }
    }

    if(!frameOnly){
        for(auto &p: m_attachMagicList){
            p->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF));
        }

        if(m_currMotion->effect && !m_currMotion->effect->done()){
            m_currMotion->effect->drawShift(startX, startY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XF0));
        }

        if(m_currMotion->type != MOTION_MON_DIE && g_clientArgParser->drawHPBar){
            auto pBar0 = g_progUseDB->retrieve(0X00000014);
            auto pBar1 = g_progUseDB->retrieve(0X00000015);

            const auto [nBarW, nBarH] = SDLDeviceHelper::getTextureSize(pBar1);
            const int drawBarXP = startX +  7;
            const int drawBarYP = startY - 53;
            const int drawBarWidth = to_d(std::lround(nBarW * getHealthRatio().at(0)));

            g_sdlDevice->drawTexture(pBar1, drawBarXP, drawBarYP, 0, 0, drawBarWidth, nBarH);
            g_sdlDevice->drawTexture(pBar0, drawBarXP, drawBarYP);

            constexpr int buffIconDrawW = 10;
            constexpr int buffIconDrawH = 10;

            const int buffIconStartX = drawBarXP + 1;
            const int buffIconStartY = drawBarYP - buffIconDrawH;

            if(getSDBuffIDList().has_value()){
                for(int drawIconCount = 0; const auto id: getSDBuffIDList().value().idList){
                    const auto &br = DBCOM_BUFFRECORD(id);
                    fflassert(br);

                    if(br.icon.gfxID != SYS_U32NIL){
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

            if(g_clientArgParser->alwaysDrawName || (focusMask & (1 << FOCUS_MOUSE))){
                const int nLW = m_nameBoard.w();
                const int nLH = m_nameBoard.h();
                const int nDrawNameXP = drawBarXP + nBarW / 2 - nLW / 2;
                const int nDrawNameYP = drawBarYP + 20;
                m_nameBoard.drawEx(nDrawNameXP, nDrawNameYP, 0, 0, nLW, nLH);
            }
        }
    }
}

static uint32_t monsterSeffBaseIDHelper(std::u8string_view monName, int offset)
{
    fflassert(!monName.empty());
    fflassert(offset >= 0, offset);
    fflassert(offset < to_d(SYS_SEFFSIZE), offset, SYS_SEFFSIZE);

    const auto &mr = DBCOM_MONSTERRECORD(monName.data());
    fflassert(mr);

    if(mr.seff.ref.empty()){
        if(mr.seff.list.at(offset).has_value()){
            if(const auto &[subname, suboff] = mr.seff.list.at(offset).value(); subname.empty()){
                return suboff;
            }
            else{
                return monsterSeffBaseIDHelper(subname, offset);
            }
        }
        else{
            return SYS_MONSEFFBASE(mr.lookID) + offset;
        }
    }
    else{
        return monsterSeffBaseIDHelper(mr.seff.ref, offset);
    }
}

uint32_t ClientMonster::getSeffID(int offset) const
{
    return monsterSeffBaseIDHelper(getMR().name, offset);
}

bool ClientMonster::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();
    for(const auto &m: m_forcedMotionQueue){
        if(m->type == MOTION_MON_DIE){
            return true;
        }
    }

    for(const auto &m: m_motionQueue){
        if(m->type == MOTION_MON_DIE){
            throw fflerror("Found MOTION_MON_DIE in pending motion queue");
        }
    }

    m_motionQueue.clear();
    switch(action.type){
        case ACTION_DIE      : return onActionDie      (action) && motionQueueValid();
        case ACTION_STAND    : return onActionStand    (action) && motionQueueValid();
        case ACTION_HITTED   : return onActionHitted   (action) && motionQueueValid();
        case ACTION_JUMP     : return onActionJump     (action) && motionQueueValid();
        case ACTION_MOVE     : return onActionMove     (action) && motionQueueValid();
        case ACTION_ATTACK   : return onActionAttack   (action) && motionQueueValid();
        case ACTION_SPAWN    : return onActionSpawn    (action) && motionQueueValid();
        case ACTION_TRANSF   : return onActionTransf   (action) && motionQueueValid();
        case ACTION_SPACEMOVE: return onActionSpaceMove(action) && motionQueueValid();
        default              : return false;
    }
}

bool ClientMonster::onActionDie(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    for(auto &node: makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED)){
        if(!(node && motionValid(node))){
            throw fflerror("current motion node is invalid");
        }
        m_forcedMotionQueue.push_back(std::move(node));
    }

    const auto [dieX, dieY, dieDir] = motionEndGLoc().at(1);
    m_forcedMotionQueue.emplace_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_DIE,
        .direction = pathf::dirValid(dieDir) ? to_d(dieDir) : DIR_UP,
        .x = dieX,
        .y = dieY,
    }));

    if(const auto deathEffectName = str_printf(u8"%s_死亡特效", to_cstr(monsterName())); DBCOM_MAGICID(to_u8cstr(deathEffectName))){
        m_forcedMotionQueue.back()->addTrigger(true, [deathEffectName, this](MotionNode *) -> bool
        {
            addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(to_u8cstr(deathEffectName), u8"运行")));
            return true;
        });
    }

    // set motion fadeOut as 0
    // server later will issue fadeOut on dead body
    return true;
}

bool ClientMonster::onActionStand(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

bool ClientMonster::onActionHitted(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    m_motionQueue.emplace_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_HITTED,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
        .extParam
        {
            .hitted
            {
                .fromUID = action.fromUID,
            },
        },
    }));
    return true;
}

bool ClientMonster::onActionTransf(const ActionNode &)
{
    throw fflerror("unexpected ACTION_TRANSF to uid: %s", uidf::getUIDString(UID()).c_str());
}

bool ClientMonster::onActionSpaceMove(const ActionNode &action)
{
    flushForcedMotion();
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = m_currMotion->direction,
        .x = action.aimX,
        .y = action.aimY,
    });

    m_processRun->addFixedLocMagic(std::unique_ptr<FixedLocMagic>(new FixedLocMagic(u8"瞬息移动", u8"运行", action.x, action.y)));
    addAttachMagic(std::unique_ptr<AttachMagic>(new AttachMagic(u8"瞬息移动", u8"裂解")));
    return true;
}

bool ClientMonster::onActionJump(const ActionNode &action)
{
    flushForcedMotion();
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
    });
    return true;
}

bool ClientMonster::onActionMove(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto moveNode = makeWalkMotion(action.x, action.y, action.aimX, action.aimY, action.speed); motionValid(moveNode)){
        m_motionQueue.push_back(std::move(moveNode));
        return true;
    }
    return false;
}

bool ClientMonster::onActionSpawn(const ActionNode &action)
{
    if(!m_forcedMotionQueue.empty()){
        throw fflerror("found motion before spawn: %s", uidf::getUIDString(UID()).c_str());
    }

    m_currMotion = std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = [&action]() -> int
        {
            if(pathf::dirValid(action.direction)){
                return action.direction;
            }
            return DIR_UP;
        }(),

        .x = action.x,
        .y = action.y,
    });
    return true;
}

bool ClientMonster::onActionAttack(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.aimUID)){
        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = [&action, endDir, coPtr]() -> int
            {
                const auto nX = coPtr->x();
                const auto nY = coPtr->y();
                if(mathf::LDistance2<int>(nX, nY, action.x, action.y) == 0){
                    return endDir;
                }
                return pathf::getOffDir(action.x, action.y, nX, nY);
            }(),
            .x = action.x,
            .y = action.y,
        }));
        return true;
    }
    return false;
}

bool ClientMonster::motionValid(const std::unique_ptr<MotionNode> &motionPtr) const
{
    if(true
            && motionPtr
            && motionPtr->type >= MOTION_MON_BEGIN
            && motionPtr->type <  MOTION_MON_END

            && motionPtr->direction >= DIR_BEGIN
            && motionPtr->direction <  DIR_END

            && m_processRun
            && m_processRun->onMap(m_processRun->mapID(), motionPtr->x,    motionPtr->y)
            && m_processRun->onMap(m_processRun->mapID(), motionPtr->endX, motionPtr->endY)

            && motionPtr->speed >= SYS_MINSPEED
            && motionPtr->speed <= SYS_MAXSPEED

            && motionPtr->frame >= 0
            && motionPtr->frame <  getFrameCount(motionPtr.get())){

        const auto nLDistance2 = mathf::LDistance2(motionPtr->x, motionPtr->y, motionPtr->endX, motionPtr->endY);
        switch(motionPtr->type){
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
            case MOTION_MON_ATTACK1:
            case MOTION_MON_SPELL0:
            case MOTION_MON_SPELL1:
            case MOTION_MON_HITTED:
            case MOTION_MON_DIE:
            case MOTION_MON_SPAWN:
                {
                    return nLDistance2 == 0;
                }
            case MOTION_MON_SPECIAL:
                {
                    return true;
                }
            default:
                {
                    break;
                }
        }
    }
    return false;
}

MonsterFrameGfxSeq ClientMonster::getFrameGfxSeq(int motion, int) const
{
    switch(motion){
        case MOTION_MON_STAND  : return {.count =  4};
        case MOTION_MON_WALK   : return {.count =  6};
        case MOTION_MON_ATTACK0: return {.count =  6};
        case MOTION_MON_HITTED : return {.count =  2};
        case MOTION_MON_DIE    : return {.count = 10};
        case MOTION_MON_ATTACK1: return {.count =  6};
        case MOTION_MON_SPELL0 :
        case MOTION_MON_SPELL1 : return {.count = 10};
        case MOTION_MON_SPAWN  : return {.count = 10};
        case MOTION_MON_SPECIAL: return {.count =  6};
        default                : return {};
    }
}

std::unique_ptr<MotionNode> ClientMonster::makeWalkMotion(int nX0, int nY0, int nX1, int nY1, int nSpeed) const
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

        auto nLDistance2 = mathf::LDistance2(nX0, nY0, nX1, nY1);
        if(false
                || nLDistance2 == 1
                || nLDistance2 == 2
                || nLDistance2 == 1 * maxStep() * maxStep()
                || nLDistance2 == 2 * maxStep() * maxStep()){

            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_WALK,
                .direction = nDirV[nSDY][nSDX],
                .speed = nSpeed,
                .x = nX0,
                .y = nY0,
                .endX = nX1,
                .endY = nY1,
            });
        }
    }
    return {};
}

ClientCreature::TargetBox ClientMonster::getTargetBox() const
{
    switch(m_currMotion->type){
        case MOTION_MON_DIE:
            {
                return {};
            }
        default:
            {
                break;
            }
    }

    const auto texBodyID = getFrameGfxSeq(m_currMotion->type, m_currMotion->direction).gfxID(this);
    if(!texBodyID.has_value()){
        return {};
    }

    int dx = 0;
    int dy = 0;
    auto bodyFrameTexPtr = g_monsterDB->retrieve(texBodyID.value(), &dx, &dy);

    if(!bodyFrameTexPtr){
        return {};
    }

    const auto [bodyFrameW, bodyFrameH] = SDLDeviceHelper::getTextureSize(bodyFrameTexPtr);

    const auto [shiftX, shiftY] = getShift(m_currMotion->frame);
    const int startX = m_currMotion->x * SYS_MAPGRIDXP + shiftX + dx;
    const int startY = m_currMotion->y * SYS_MAPGRIDYP + shiftY + dy;

    return getTargetBoxHelper(startX, startY, bodyFrameW, bodyFrameH);
}

bool ClientMonster::deadFadeOut()
{
    switch(m_currMotion->type){
        case MOTION_MON_DIE:
            {
                if(getMR().deadFadeOut){
                    if(!m_currMotion->extParam.die.fadeOut){
                        m_currMotion->extParam.die.fadeOut = 1;
                    }
                }
                return true;
            }
        default:
            {
                return false; // TODO push an ActionDie here
            }
    }
}

int ClientMonster::maxStep() const
{
    return 1;
}

int ClientMonster::currStep() const
{
    fflassert(motionValid(m_currMotion));
    switch(m_currMotion->type){
        case MOTION_MON_WALK:
            {
                return 1;
            }
        default:
            {
                return 0;
            }
    }
}

ClientMonster *ClientMonster::create(uint64_t uid, ProcessRun *proc, const ActionNode &action)
{
    switch(const auto monID = uidf::getMonsterID(uid)){
        case DBCOM_MONSTERID(u8"蚂蚁道士"):
            {
                return new ClientAntHealer(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"诺玛法老"):
        case DBCOM_MONSTERID(u8"诺玛大法老"):
            {
                return new ClientNumaWizard(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"红衣法师"):
            {
                return new ClientRedClothWizard(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"洞蛆"):
            {
                return new ClientCaveMaggot(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"火焰沃玛"):
            {
                return new ClientWoomaFlamingWarrior(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沃玛教主"):
            {
                return new ClientWoomaTaurus(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"暗黑战士"):
            {
                return new ClientDarkWarrior(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"粪虫"):
            {
                return new ClientDung(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"触龙神"):
            {
                return new ClientEvilCentipede(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"祖玛弓箭手"):
            {
                return new ClientZumaArcher(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"祖玛雕像"):
        case DBCOM_MONSTERID(u8"祖玛卫士"):
            {
                return new ClientZumaMonster(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"祖玛教主"):
            {
                return new ClientZumaTaurus(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"爆毒蚂蚁"):
            {
                return new ClientGasAnt(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙漠风魔"):
            {
                return new ClientSandEvilFan(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"爆裂蜘蛛"):
            {
                return new ClientBombSpider(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"霸王教主"):
            {
                return new ClientShipwreckLord(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"潘夜左护卫"):
        case DBCOM_MONSTERID(u8"潘夜右护卫"):
            {
                return new ClientMinotaurGuardian(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                return new ClientTaoSkeleton(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"超强骷髅"):
            {
                return new ClientTaoSkeletonExt(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"神兽"):
            {
                return new ClientTaoDog(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"食人花"):
            {
                return new ClientCannibalPlant(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"角蝇"):
            {
                return new ClientBugbatMaggot(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙漠树魔"):
            {
                return new ClientSandCactus(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"掷斧骷髅"):
            {
                return new ClientDualAxeSkeleton(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"楔蛾"):
            {
                return new ClientWedgeMoth(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙鬼"):
            {
                return new ClientSandGhost(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙漠石人"):
            {
                return new ClientSandStoneMan(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"雷电僵尸"):
            {
                return new ClientLightBoltZombie(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"僧侣僵尸"):
            {
                return new ClientMonkZombie(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"僵尸_1"):
        case DBCOM_MONSTERID(u8"僵尸_2"):
        case DBCOM_MONSTERID(u8"腐僵"):
            {
                return new ClientRebornZombie(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"栗子树"):
        case DBCOM_MONSTERID(u8"圣诞树"):
        case DBCOM_MONSTERID(u8"圣诞树1"):
            {
                return new ClientTree(uid, proc, action);
            }
        default:
            {
                if(DBCOM_MONSTERRECORD(monID).behaveMode == BM_GUARD){
                    return new ClientGuard(uid, proc, action);
                }
                else{
                    return new ClientMonster(uid, proc, action);
                }
            }
    }
}
