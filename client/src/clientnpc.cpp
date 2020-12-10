/*
 * =====================================================================================
 *
 *       Filename: clientnpc.cpp
 *        Created: 04/12/2020 12:53:00
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

#include "uidf.hpp"
#include "mathf.hpp"
#include "clientnpc.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"

extern PNGTexOffDB *g_standNPCDB;
extern ClientArgParser *g_clientArgParser;

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

int ClientNPC::motionFrameCount(int motion, int direction) const
{
    // NPC direction is not the real direction
    // it's like a seq id for the first/second/third valid direction

    switch(lookID()){
        case 56: // 六面神石
            {
                return 12;
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
                                        return 4;
                                    }
                                default:
                                    {
                                        return 4; // TODO
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
                                        return 4;
                                    }
                                default:
                                    {
                                        return 4; // TODO
                                    }
                            }
                        }
                    default:
                        {
                            return -1;
                        }
                }
            }
    }
}

int32_t ClientNPC::gfxShadowID(int32_t gfxId) const
{
    if(gfxId < 0){
        return -1;
    }

    constexpr int32_t shadowBit = (int32_t)(1) << 23;
    if(gfxId & shadowBit){
        throw fflerror("gfxID shadow bit mask is set");
    }
    return gfxId | shadowBit;
}

void ClientNPC::draw(int viewX, int viewY, int focusMask)
{
    const auto bodyKey = gfxID();
    if(bodyKey < 0){
        return;
    }

    const auto shadowKey = gfxShadowID(bodyKey);
    if(shadowKey < 0){
        return;
    }

    int bodyDX = 0;
    int bodyDY = 0;
    auto bodyFrame = g_standNPCDB->Retrieve(bodyKey + m_currMotion.frame, &bodyDX, &bodyDY);

    int shadowDX = 0;
    int shadowDY = 0;
    auto shadowFrame = g_standNPCDB->Retrieve(shadowKey + m_currMotion.frame, &shadowDX, &shadowDY);

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

        if(!(focusChan >= 0 && focusChan < FOCUS_MAX)){
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

    for(int focusChan = 1; focusChan < FOCUS_MAX; ++focusChan){
        if(focusMask & (1 << focusChan)){
            fnBlendFrame(bodyFrame, focusChan, bodyDrawX, bodyDrawY, 255);
        }
    }

    if(g_clientArgParser->drawTextureAlignLine){
        g_sdlDevice->drawLine(colorf::RED + 128, bodyDrawX, bodyDrawY, bodyDrawX + bodyDX, bodyDrawY + bodyDY);
        g_sdlDevice->drawLine(colorf::BLUE + 128, bodyDrawX - 5, bodyDrawY, bodyDrawX + 5, bodyDrawY);
        g_sdlDevice->drawLine(colorf::BLUE + 128, bodyDrawX, bodyDrawY - 5, bodyDrawX, bodyDrawY + 5);
    }

    if(g_clientArgParser->drawTargetBox){
        if(const auto box = getTargetBox()){
            g_sdlDevice->drawRectangle(colorf::BLUE + 128, box.x - viewX, box.y - viewY, box.w, box.h);
        }
    }

    for(auto &p: m_attachMagicList){
        p->drawShift(x() * SYS_MAPGRIDXP - viewX, y() * SYS_MAPGRIDYP - viewY, false);
    }
}

int32_t ClientNPC::gfxID() const
{
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

    if(motionFrameCount(m_currMotion.type, m_currMotion.direction) <= 0){
        return -1;
    }
    return ((int32_t)(lookID()) << 12) | ((int32_t)(m_currMotion.type & 0X0F) << 8) | ((int32_t)(m_currMotion.direction) << 5);
}

bool ClientNPC::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();
    const int motion = [&action]() -> int
    {
        switch(action.Action){
            case ACTION_SPAWN:
                {
                    return MOTION_NPC_STAND;
                }
            case ACTION_STAND:
                {
                    switch(action.ActionParam){
                        case 1 : return MOTION_NPC_ACT;
                        case 2 : return MOTION_NPC_ACTEXT;
                        default: return MOTION_NPC_STAND;
                    }
                }
            default:
                {
                    throw fflerror("invalid action node: type = %d", action.Action);
                }
        }
    }();

    m_currMotion = MotionNode
    {
        .type = motion,
        .direction = action.Direction,
        .x = action.X,
        .y = action.Y,
    };

    return motionValid(m_currMotion);
}

bool ClientNPC::update(double ms)
{
    updateAttachMagic(ms);

    if(SDL_GetTicks() * 1.0f < currMotionDelay() + m_lastUpdateTime){
        return true;
    }

    motionValidEx(m_currMotion);
    m_lastUpdateTime = SDL_GetTicks() * 1.0f;

    const CallOnExitHelper motionOnUpdate([this]()
    {
        if(m_currMotion.onUpdate){
            m_currMotion.onUpdate(&m_currMotion);
        }
    });

    const bool doneCurrMotion = [this]()
    {
        return m_currMotion.frame + 1 == motionFrameCount(m_currMotion.type, m_currMotion.direction);
    }();

    switch(m_currMotion.type){
        case MOTION_NPC_STAND:
            {
                return advanceMotionFrame(1);
            }
        case MOTION_NPC_ACT:
        case MOTION_NPC_ACTEXT:
            {
                if(doneCurrMotion){
                    return moveNextMotion();
                }
                return advanceMotionFrame(1);
            }
        default:
            {
                throw fflerror("invalid motion: %d", m_currMotion.type);
            }
    }
}

bool ClientNPC::motionValid(const MotionNode &) const
{
    return true;
}

int ClientNPC::gfxMotionID(int) const
{
    return -1;
}

ClientCreature::TargetBox ClientNPC::getTargetBox() const
{
    const auto texBaseID = gfxID();
    if(texBaseID < 0){
        return {};
    }

    int dx = 0;
    int dy = 0;
    auto bodyFrameTexPtr = g_standNPCDB->Retrieve(texBaseID + m_currMotion.frame, &dx, &dy);

    if(!bodyFrameTexPtr){
        return {};
    }

    const auto [bodyFrameW, bodyFrameH] = SDLDevice::getTextureSize(bodyFrameTexPtr);

    const int startX = x() * SYS_MAPGRIDXP + dx;
    const int startY = y() * SYS_MAPGRIDYP + dy;

    return getTargetBoxHelper(startX, startY, bodyFrameW, bodyFrameH);
}
