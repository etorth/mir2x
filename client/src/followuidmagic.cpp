// Magic following UID has two offsets
// Location a is the <m_x, m_y>, offset (b - a) is where to draw frame gfxs
// Location c is the magic gfx arrow head (or fireball head) in the frame animation, which is used as target point
//
//     g_magicDB->retrieve(seqID, frame): ( offX,  offY) is (b - a)
//     g_magicDB->head(seqID)           : (txOff, tyOff) is (c - a), FollowUIDMagic::targetOff()
//
//        b--------+
//       /|     /  |
//      / |    /   |
//     /  |   /    |
//    /   |  L c   |
//   a    |        |
//        +--------+
//
//  the original gfx library doesn't have the head, it's stored in the magic APNG file name
//  only follow uid magic needs it, mir2 also gives this hard-coded offset in src: Mir3EIClient/GameProcess/Magic.cpp

#include <cmath>
#include "pathf.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "attachmagic.hpp"
#include "magicdb.hpp"
#include "followuidmagic.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern MagicDB *g_magicDB;
extern ClientArgParser *g_clientArgParser;

FollowUIDMagic::FollowUIDMagic(
        const char8_t *magicName,
        const char8_t *magicStage,

        int x,
        int y,
        int gfxDirIndex,
        int flyDirIndex,
        int moveSpeed,

        uint64_t uid,
        ProcessRun *runPtr)
    : BaseMagic(magicName, magicStage, gfxDirIndex)
    , m_startX(x)
    , m_startY(y)
    , m_flyDirIndex(flyDirIndex)
    , m_x(x)
    , m_y(y)
    , m_moveSpeed(moveSpeed)
    , m_uid(uid)
    , m_process(runPtr)
{
    fflassert(m_process);
    fflassert(flyDirIndex >= 0 && flyDirIndex < 16);
    fflassert(m_x >= 0 && m_y >= 0 && m_process->onMap(m_x / SYS_MAPGRIDXP, m_y / SYS_MAPGRIDYP));
}

std::tuple<int, int> FollowUIDMagic::targetOff() const
{
    if(const auto seqID = gfxSeqID(); seqID != SYS_U32NIL){
        return g_magicDB->head(seqID);
    }
    return {0, 0};
}

bool FollowUIDMagic::update(double ms)
{
    const auto [dx, dy] = [this]() -> std::tuple<int, int>
    {
        if(const auto coPtr = m_process->findUID(m_uid)){
            if(const auto box = coPtr->getTargetBox()){
                const auto [srcTargetX, srcTargetY] = targetPLoc();
                const auto [dstTargetX, dstTargetY] = box.targetPLoc();

                const int xdiff = dstTargetX - srcTargetX;
                const int ydiff = dstTargetY - srcTargetY;

                m_lastLDistance2 = mathf::LDistance2(dstTargetX, dstTargetY, srcTargetX, srcTargetY);
                if(xdiff == 0 && ydiff == 0){
                    return {0, 0};
                }
                else{
                    return pathf::getDirOff(xdiff, ydiff, m_moveSpeed);
                }
            }
        }

        m_lastLDistance2 = INT_MAX;
        return m_lastFlyOff.value_or(pathf::getDir16Off(m_flyDirIndex, m_moveSpeed));
    }();

    m_x += dx;
    m_y += dy;

    m_lastFlyOff = {dx, dy};
    return BaseMagic::update(ms);
}

void FollowUIDMagic::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    if(const auto seqID = gfxSeqID(); seqID != SYS_U32NIL){
        if(auto [texPtr, offX, offY] = g_magicDB->retrieve(seqID, frame()); texPtr){
            const auto gfxEntryModColor = m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor;
            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(gfxEntryModColor, modColor));
            SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);

            const int drawPX = (m_x + offX) - viewX;
            const int drawPY = (m_y + offY) - viewY;
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

            g_sdlDevice->drawTexture(texPtr, drawPX, drawPY);
            if(g_clientArgParser->drawMagicGrid){
                g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(200), drawPX, drawPY, texW, texH);
                g_sdlDevice->drawLine(colorf::RED + colorf::A_SHF(200), drawPX, drawPY, m_x - viewX, m_y - viewY);

                const auto [srcTargetX, srcTargetY] = targetPLoc();
                g_sdlDevice->drawLine(colorf::GREEN + colorf::A_SHF(200), m_x - viewX, m_y - viewY, srcTargetX - viewX, srcTargetY - viewY);

                if(const auto coPtr = m_process->findUID(m_uid)){
                    const auto [srcTargetX, srcTargetY] = targetPLoc();
                    const auto [dstTargetX, dstTargetY] = coPtr->getTargetBox().targetPLoc();
                    g_sdlDevice->drawLine(colorf::MAGENTA + colorf::A_SHF(200), srcTargetX - viewX, srcTargetY - viewY, dstTargetX - viewX, dstTargetY - viewY);
                }
            }
        }
    }
}

bool FollowUIDMagic::done() const
{
    const auto startGX = m_startX / SYS_MAPGRIDXP;
    const auto startGY = m_startY / SYS_MAPGRIDYP;

    const auto currGX = m_x / SYS_MAPGRIDXP;
    const auto currGY = m_y / SYS_MAPGRIDYP;

    if(mathf::LDistance(startGX, startGY, currGX, currGY) > 255){
        return true;
    }

    if(const auto coPtr = m_process->findUID(m_uid)){
        const auto [srcTargetX, srcTargetY] = targetPLoc();
        const auto [dstTargetX, dstTargetY] = coPtr->getTargetBox().targetPLoc();
        return (std::abs(srcTargetX - dstTargetX) < 24 && std::abs(srcTargetY - dstTargetY) < 16) || (mathf::LDistance2<int>(srcTargetX, srcTargetY, dstTargetX, dstTargetY) > m_lastLDistance2);
    }
    return false;
}

std::tuple<int, int> FollowUIDMagic::getSoundEffectPosition() const
{
    const auto selfGX = m_x / SYS_MAPGRIDXP;
    const auto selfGY = m_y / SYS_MAPGRIDYP;

    fflassert(m_process->getMyHero());
    const auto [heroGX, heroGY] = m_process->getMyHero()->location();

    const auto dGX = selfGX - heroGX;
    const auto dGY = selfGY - heroGY;

    if(dGX == 0 && dGY == 0){
        return {0, 0};
    }

    return
    {
        std::lround(mathf::LDistance<double>(dGX, dGY, 0, 0)),
        std::lround(90.0 - 180.0 * std::atan2(-dGY, dGX) / mathf::pi),
    };
}
