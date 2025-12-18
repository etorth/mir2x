#include "uidf.hpp"
#include "mathf.hpp"
#include "sdldevice.hpp"
#include "clientnpc.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_standNPCDB;
extern ClientArgParser *g_clientArgParser;

std::optional<uint32_t> NPCFrameGfxSeq::gfxID(const ClientNPC *npcPtr, std::optional<int> frameOpt) const
{
    fflassert(npcPtr);
    if(*this){
        // stand npc graphics retrieving key structure
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

        // NPC lookID allows zero
        // check mir2x/tools/npcwil2png/src/main.cpp

        const int frame = frameOpt.value_or(npcPtr->currMotion()->frame);
        fflassert(frame >= 0);
        fflassert(frame < count);

        return 0
            + (to_u32((npcPtr->lookID()                           ) & 0X07FF) << 12)
            + (to_u32((npcPtr->currMotion()->type                 ) & 0X000F) <<  8)
            + (to_u32((npcPtr->currMotion()->direction - DIR_BEGIN) & 0X0007) <<  5)
            + (to_u32((frame                                      ) & 0X001F) <<  0);
    }
    return {};
}

ClientNPC::ClientNPC(uint64_t uid, ProcessRun *proc, const ActionNode &action)
    : ClientCreature(uid, proc)
{
    if(type() != UID_NPC){
        throw fflerror("uid type: %s", uidf::getUIDString(UID()).c_str());
    }

    if(!parseAction(action)){
        throw fflerror("invalid NPC action");
    }
}

NPCFrameGfxSeq ClientNPC::getFrameGfxSeq(int motion, int direction) const
{
    // NPC direction is not the real direction
    // it's like a seq id for the first/second/third valid direction

    switch(lookID()){
        case 56: // 六面神石
            {
                return {.count = 12};
            }
        case 59: // 母子石像
        case 64: // 雪人
        case 65: // 雪人带树枝
            {
                return {.count = 1};
            }
        default:
            {
                // TODO many motions are not included here
                // check mir2x/tools/npcwil2png/src/main.cpp
                switch(motion){
                    case MOTION_NPC_STAND:
                        {
                            switch(direction){
                                case DIR_DOWN:
                                case DIR_DOWNLEFT:
                                case DIR_DOWNRIGHT:
                                    {
                                        return {.count = 4};
                                    }
                                default:
                                    {
                                        return {.count = 4}; // TODO
                                    }
                            }
                        }
                    case MOTION_NPC_ACT:
                        {
                            switch(direction){
                                case DIR_DOWN:
                                case DIR_DOWNLEFT:
                                case DIR_DOWNRIGHT:
                                    {
                                        return {.count = 4};
                                    }
                                default:
                                    {
                                        return {.count = 4}; // TODO
                                    }
                            }
                        }
                    default:
                        {
                            return {};
                        }
                }
            }
    }
}

void ClientNPC::drawFrame(int viewX, int viewY, int focusMask, int frame, bool)
{
    const auto bodyKey = getFrameGfxSeq(currMotion()->type, currMotion()->direction).gfxID(this, frame);
    if(!bodyKey.has_value()){
        return;
    }

    const auto [  bodyFrame,   bodyDX,   bodyDY] = g_standNPCDB->retrieve((bodyKey.value()                    ));
    const auto [shadowFrame, shadowDX, shadowDY] = g_standNPCDB->retrieve((bodyKey.value() | (to_u32(1) << 23)));

    if(bodyFrame){
        SDL_SetTextureAlphaMod(bodyFrame, 255);
    }

    if(shadowFrame){
        SDL_SetTextureAlphaMod(shadowFrame, 128);
    }

    auto fnBlendFrame = [](SDL_Texture *texture, int focusChan, int x, int y, Uint8 alpha)
    {
        if(!texture){
            return;
        }

        if(!(focusChan >= 0 && focusChan < FOCUS_END)){
            return;
        }

        const auto color = focusColor(focusChan);
        SDL_SetTextureColorMod(texture, color.r, color.g, color.b);

        SDL_SetTextureAlphaMod(texture, alpha);
        g_sdlDevice->drawTexture(texture, x, y);
    };

    const int   bodyDrawX = x() * SYS_MAPGRIDXP +   bodyDX - viewX;
    const int   bodyDrawY = y() * SYS_MAPGRIDYP +   bodyDY - viewY;
    const int shadowDrawX = x() * SYS_MAPGRIDXP + shadowDX - viewX;
    const int shadowDrawY = y() * SYS_MAPGRIDYP + shadowDY - viewY;

    fnBlendFrame(shadowFrame, FOCUS_NONE, shadowDrawX, shadowDrawY, 128);
    fnBlendFrame(  bodyFrame, FOCUS_NONE,   bodyDrawX,   bodyDrawY, 255);

    for(int focusChan = 1; focusChan < FOCUS_END; ++focusChan){
        if(focusMask & (1 << focusChan)){
            fnBlendFrame(bodyFrame, focusChan, bodyDrawX, bodyDrawY, 255);
        }
    }

    if(g_clientArgParser->drawTextureAlignLine){
        g_sdlDevice->drawLine (colorf::RED  + colorf::A_SHF(128), bodyDrawX - bodyDX, bodyDrawY - bodyDY, bodyDrawX, bodyDrawY);
        g_sdlDevice->drawCross(colorf::BLUE + colorf::A_SHF(128), bodyDrawX - bodyDX, bodyDrawY - bodyDY, 5);

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(bodyFrame);
        g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(128), bodyDrawX, bodyDrawY, texW, texH);
    }

    if(g_clientArgParser->drawTargetBox){
        if(const auto box = getTargetBox()){
            g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(128), box.x - viewX, box.y - viewY, box.w, box.h);
        }
    }

    for(auto &p: m_attachMagicList){
        p->drawShift(x() * SYS_MAPGRIDXP - viewX, y() * SYS_MAPGRIDYP - viewY, colorf::RGBA(0XFF, 0XFF, 0XFF, 0XFF));
    }
}

bool ClientNPC::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();
    const int motion = [&action]() -> int
    {
        switch(action.type){
            case ACTION_SPAWN:
                {
                    return MOTION_NPC_STAND;
                }
            case ACTION_STAND:
                {
                    switch(action.extParam.stand.npc.act){
                        case 1 : return MOTION_NPC_ACT;
                        case 2 : return MOTION_NPC_ACTEXT;
                        default: return MOTION_NPC_STAND;
                    }
                }
            default:
                {
                    throw fflerror("invalid action node: type = %d", action.type);
                }
        }
    }();

    m_currMotion.reset(new MotionNode
    {
        .type = motion,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
    });

    return motionValid(m_currMotion);
}

bool ClientNPC::update(double ms)
{
    updateAttachMagic(ms);
    if(!checkUpdate(ms)){
        return true;
    }

    fflassert(motionValid(m_currMotion));
    const CallOnExitHelper motionOnUpdate([this]()
    {
        m_currMotion->runTrigger();
    });

    const bool doneCurrMotion = [this]()
    {
        return m_currMotion->frame + 1 == getFrameCount(m_currMotion.get());
    }();

    switch(m_currMotion->type){
        case MOTION_NPC_STAND:
            {
                return advanceMotionFrame();
            }
        case MOTION_NPC_ACT:
        case MOTION_NPC_ACTEXT:
            {
                if(doneCurrMotion){
                    return moveNextMotion();
                }
                return advanceMotionFrame();
            }
        default:
            {
                throw fflerror("invalid motion: %d", m_currMotion->type);
            }
    }
}

bool ClientNPC::motionValid(const std::unique_ptr<MotionNode> &motionPtr) const
{
    return motionPtr.get() != nullptr;
}

ClientCreature::TargetBox ClientNPC::getTargetBox() const
{
    const auto texBodyID = getFrameGfxSeq(currMotion()->type, currMotion()->direction).gfxID(this);
    if(!texBodyID.has_value()){
        return {};
    }

    auto [bodyFrameTexPtr, dx, dy] = g_standNPCDB->retrieve(texBodyID.value());
    if(!bodyFrameTexPtr){
        return {};
    }

    const auto [bodyFrameW, bodyFrameH] = SDLDeviceHelper::getTextureSize(bodyFrameTexPtr);

    const int startX = x() * SYS_MAPGRIDXP + dx;
    const int startY = y() * SYS_MAPGRIDYP + dy;

    return getTargetBoxHelper(startX, startY, bodyFrameW, bodyFrameH);
}
