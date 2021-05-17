/*
 * =====================================================================================
 *
 *       Filename: fixedlocmagic.cpp
 *        Created: 08/07/2017 21:31:24
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

#include "pathf.hpp"
#include "sdldevice.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "fixedlocmagic.hpp"

extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;

void FixedLocMagic::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    if(done()){
        return;
    }

    if(m_gfxEntry.gfxID == SYS_TEXNIL){
        return;
    }

    int offX = 0;
    int offY = 0;

    const int gfxDirOff = ((m_gfxDirIndex >= 0) ? m_gfxDirIndex : 0) * m_gfxEntry.gfxIDCount;
    if(auto texPtr = g_magicDB->Retrieve(m_gfxEntry.gfxID + gfxDirOff + frame(), &offX, &offY)){
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, modColor);
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_sdlDevice->drawTexture(texPtr, m_x * SYS_MAPGRIDXP - viewX + offX, m_y * SYS_MAPGRIDYP - viewY + offY);
    }
}

void FireAshEffect_RUN::drawGroundAsh(int viewX, int viewY, uint32_t modColor) const
{
    if(auto [texPtr, offX, offY] = g_magicDB->Retrieve(0X000009C4); texPtr){
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(getPlainModColor(), modColor));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_sdlDevice->drawTexture(texPtr, x() * SYS_MAPGRIDXP - viewX + offX, y() * SYS_MAPGRIDYP - viewY + offY);
    }
}

void FireAshEffect_RUN::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    // won't draw ground ash in this
    // ash should be draw as ground object
    FixedLocMagic::drawViewOff(viewX, viewY, colorf::modRGBA(getPlainModColor(), modColor));
}

void FireWall_RUN::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    FixedLocMagic::drawViewOff(viewX, viewY, colorf::modRGBA(getPlainModColor(), modColor));
}

void HellFire_RUN::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    constexpr int repeat = 2;
    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, m_fireDir, 1);

    constexpr int dx = SYS_MAPGRIDXP / repeat;
    constexpr int dy = SYS_MAPGRIDYP / repeat;

    for(int i = 0; i < repeat; ++i){
        FixedLocMagic::drawViewOff(viewX + sgnDX * dx * i, viewY + sgnDY * dy * i, modColor);
    }
}
