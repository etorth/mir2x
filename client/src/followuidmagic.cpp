/*
 * =====================================================================================
 *
 *       Filename: followuidmagic.cpp
 *        Created: 12/07/2020 21:19:44
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

// Magic following UID has two offsets
// Location a is the <m_x, m_y>, offset (b - a) is where to draw frame gfxs
// Location c is the magic gfx arrow head (or fireball head) in the frame animation, which is used as target point
//
//     g_magicDB->Retrieve(texID, offX, offY): ( offX,  offY) is (b - a)
//     FollowUIDMagic::targetOff()           : (txOff, tyOff) is (c - a)
//
//        b--------+
//       /|     /  |
//      / |    /   |
//     /  |   /    |
//    /   |  L c   |
//   a    |        |
//        +--------+
//
//  unfortunately the gfx library doesn't have this information
//  fortunately only follow uid magic needs this, mir2 also gives this hard-coded offset in src: Mir3EIClient/GameProcess/Magic.cpp

#include <cmath>
#include "pathf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdb.hpp"
#include "followuidmagic.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;
extern ClientArgParser *g_clientArgParser;

FollowUIDMagic::FollowUIDMagic(
        const char8_t *magicName,
        const char8_t *magicStage,

        int x,
        int y,
        int gfxDirIndex,
        int moveSpeed,

        uint64_t uid,
        ProcessRun *runPtr)
    : MagicBase(magicName, magicStage, gfxDirIndex)
    , m_x(x)
    , m_y(y)
    , m_moveSpeed(moveSpeed)
    , m_uid(uid)
    , m_process(runPtr)
{
    if(!m_process){
        throw fflerror("invalid process pointer: nullptr");
    }

    if(!(m_x >= 0 && m_y >= 0 && m_process->onMap(m_x / SYS_MAPGRIDXP, m_y / SYS_MAPGRIDYP))){
        throw fflerror("invalid pixel on map: x = %d, y = %d", m_x, m_y);
    }
}

uint32_t FollowUIDMagic::frameTexID() const
{
    if(m_gfxEntry.gfxID == SYS_TEXNIL){
        return SYS_TEXNIL;
    }

    switch(m_gfxEntry.gfxDirType){
        case  1: return m_gfxEntry.gfxID + frame();
        case  4:
        case  8:
        case 16:
        default: return m_gfxEntry.gfxID + frame() + m_gfxDirIndex * m_gfxEntry.gfxIDCount;
    }
}

std::tuple<int, int> FollowUIDMagic::targetOff() const
{
    switch(magicID()){
        case DBCOM_MAGICID(u8"灵魂火符"):
            {
                return {0, 0};
            }
        case DBCOM_MAGICID(u8"火球术"):
        case DBCOM_MAGICID(u8"大火球"):
        case DBCOM_MAGICID(u8"冰月神掌"):
            {
                switch(gfxDirIndex()){
                    case 0 : return { 24, -33};
                    case 1 : return { 34, -33};
                    case 2 : return { 39, -30};
                    case 3 : return { 43, -28};
                    case 4 : return { 51, -15};
                    case 5 : return { 48, -7 };
                    case 6 : return { 40, -4 };
                    case 7 : return { 34,  1 };
                    case 8 : return { 23,  1 };
                    case 9 : return { 13,  1 };
                    case 10: return { 10, -1 };
                    case 11: return { 5 , -3 };
                    case 12: return {-2 , -14};
                    case 13: return { 5 , -28};
                    case 14: return { 9 , -30};
                    case 15: return { 13, -32};
                    default: throw bad_reach();
                }
            }
        default:
            {
                return {0, 0};
            }
    }
}

bool FollowUIDMagic::update(double ms)
{
    const auto [dx, dy] = [this]() -> std::tuple<int, int>
    {
        if(const auto coPtr = m_process->findUID(m_uid)){
            const auto [targetX, targetY] = targetPLoc();
            const auto [x, y] = coPtr->getTargetBox().center();

            const int xdiff = x - targetX;
            const int ydiff = y - targetY;

            m_lastLDistance2 = mathf::LDistance2(x, y, targetX, targetY);
            if(xdiff == 0 && ydiff == 0){
                return {0, 0};
            }
            else{
                return pathf::getDirOff(xdiff, ydiff, m_moveSpeed);
            }
        }
        else{
            m_lastLDistance2 = INT_MAX;
            switch(m_gfxEntry.gfxDirType){
                case 16: return pathf::getDir16Off(m_gfxDirIndex, m_moveSpeed);
                case  8: return pathf::getDir8Off (m_gfxDirIndex, m_moveSpeed);
                case  4: return pathf::getDir4Off (m_gfxDirIndex, m_moveSpeed);
                case  1: return pathf::getDir16Off(m_gfxDirIndex, m_moveSpeed); // TODO if magic gfx has no direction I use 16-dir
                default: throw fflerror("invalid gfxDirType: %d", m_gfxEntry.gfxDirType);
            }
        }
    }();

    m_x += dx;
    m_y += dy;
    return MagicBase::update(ms);
}

void FollowUIDMagic::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    if(const auto texID = frameTexID(); texID != SYS_TEXNIL){
        if(auto [texPtr, offX, offY] = g_magicDB->Retrieve(texID); texPtr){
            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, modColor);
            SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);

            const int drawPX = (m_x + offX) - viewX;
            const int drawPY = (m_y + offY) - viewY;
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

            g_sdlDevice->drawTexture(texPtr, drawPX, drawPY);
            if(g_clientArgParser->drawMagicGrid){
                g_sdlDevice->drawRectangle(colorf::BLUE + 200, drawPX, drawPY, texW, texH);
                g_sdlDevice->drawLine(colorf::RED + 200, drawPX, drawPY, m_x - viewX, m_y - viewY);

                const auto [targetX, targetY] = targetPLoc();
                g_sdlDevice->drawLine(colorf::GREEN + 200, m_x - viewX, m_y - viewY, targetX - viewX, targetY - viewY);
            }
        }
    }
}

bool FollowUIDMagic::done() const
{
    if(const auto coPtr = m_process->findUID(m_uid)){
        const auto [targetX, targetY] = targetPLoc();
        const auto [centerX, centerY] = coPtr->getTargetBox().center();
        return (std::abs(targetX - centerX) < 24 && std::abs(targetY - centerY) < 16) || (mathf::LDistance2<int>(targetX, targetY, centerX, centerY) > m_lastLDistance2);
    }
    return false;
}
