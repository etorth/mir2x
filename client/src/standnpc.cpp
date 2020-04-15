/*
 * =====================================================================================
 *
 *       Filename: standnpc.cpp
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
#include "standnpc.hpp"
#include "pngtexoffdb.hpp"

extern PNGTexOffDB *g_standNPCDB;

StandNPC::StandNPC(uint64_t uid, ProcessRun *proc, const ActionNode &)
    : Creature(uid, proc)
{
    if(uidf::getUIDType(UID()) != UID_NPC){
        throw fflerror("uid type: %s", uidf::getUIDString(UID()));
    }
}

int StandNPC::motionFrameCount(int motion, int direction) const
{

    switch(lookID()){
        case 0:
            {
                return -1;
            }
        default:
            {
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
                                        return -1;
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
                                        return -1;
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

int32_t StandNPC::gfxShadowID(int32_t gfxId) const
{
    if(gfxId <= 0){
        return -1;
    }

    constexpr int32_t shadowBit = (int32_t)(1) << 23;
    if(gfxId & shadowBit){
        throw fflerror("gfxID shadow bit mask is set");
    }
    return gfxId | shadowBit;
}

bool StandNPC::canFocus(int pointX, int pointY) const
{
    if(gfxID() < 0){
        return false;
    }

    int bodyDX = 0;
    int bodyDY = 0;
    const uint32_t bodyKey = ((uint32_t)(gfxID() & 0X03FFFF) << 5) + m_currMotion.frame;
    auto bodyFrame = g_standNPCDB->Retrieve(bodyKey, &bodyDX, &bodyDY);

    if(!bodyFrame){
        return false;
    }

    int bodyFrameW = 0;
    int bodyFrameH = 0;
    SDL_QueryTexture(bodyFrame, nullptr, nullptr, &bodyFrameW, &bodyFrameH);

    const int startX = X() * SYS_MAPGRIDXP + bodyDX;
    const int startY = Y() * SYS_MAPGRIDYP + bodyDY;

    const int maxTargetW = SYS_MAPGRIDXP + SYS_TARGETRGN_GAPX;
    const int maxTargetH = SYS_MAPGRIDYP + SYS_TARGETRGN_GAPY;

    return ((bodyFrameW >= maxTargetW) ? mathf::pointInSegment(pointX, (startX + (bodyFrameW - maxTargetW) / 2), maxTargetW) : mathf::pointInSegment(pointX, startX, bodyFrameW))
        && ((bodyFrameH >= maxTargetH) ? mathf::pointInSegment(pointY, (startY + (bodyFrameH - maxTargetH) / 2), maxTargetH) : mathf::pointInSegment(pointY, startY, bodyFrameH));
}

bool StandNPC::draw(int viewX, int viewY, int focusMask)
{
    const auto bodyKey = gfxID();
    if(bodyKey < 0){
        return false;
    }

    const auto shadowKey = gfxShadowID(bodyKey);
    if(shadowKey < 0){
        return false;
    }

    int bodyDX = 0;
    int bodyDY = 0;
    auto bodyFrame = g_standNPCDB->Retrieve(bodyKey, &bodyDX, &bodyDY);

    int shadowDX = 0;
    int shadowDY = 0;
    auto shadowFrame = g_standNPCDB->Retrieve(shadowKey, &shadowDX, &shadowDY);

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
        g_SDLDevice->DrawTexture(texture, x, y);
    };

    const int   bodyDrawX = X() * SYS_MAPGRIDXP +   bodyDX - viewX;
    const int   bodyDrawY = Y() * SYS_MAPGRIDYP +   bodyDY - viewY;
    const int shadowDrawX = X() * SYS_MAPGRIDXP + shadowDX - viewX;
    const int shadowDrawY = Y() * SYS_MAPGRIDYP + shadowDY - viewY;

    fnBlendFrame(shadowFrame, FOCUS_NONE, shadowDrawX, shadowDrawY, 128);
    fnBlendFrame(  bodyFrame, FOCUS_NONE,   bodyDrawX,   bodyDrawY, 255);

    for(int focusChan = 1; focusChan < FOCUS_MAX; ++focusChan){
        if(focusMask & (1 << focusChan)){
            fnBlendFrame(bodyFrame, focusChan, bodyDrawX, bodyDrawY, 255);
        }
    }

    for(auto p: m_attachMagicList){
        p->Draw(X() * SYS_MAPGRIDXP - viewX, Y() * SYS_MAPGRIDYP - viewY);
    }
    return true;
}

int32_t StandNPC::gfxID() const
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

    if(lookID() <= 0){
        return -1;
    }

    if(motionFrameCount(m_currMotion.motion, m_currMotion.direction) <= 0){
        return ((int32_t)(uidf::getLookID(UID()) & 0X07FF) << 12) | ((int32_t)(m_currMotion.motion & 0X0F) << 8) | ((int32_t)(m_currMotion.direction) << 5);
    }
    return -1;
}
