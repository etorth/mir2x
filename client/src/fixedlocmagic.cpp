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

void FireAshEffect_RUN::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    const auto fnDrawTexture = [viewX, viewY, modColor, this](float ratio)
    {
        if(auto [texPtr, offX, offY] = g_magicDB->Retrieve(0X000009C4); texPtr){
            const auto groundModColor = colorf::WHITE + colorf::round255(ratio * 150.0);
            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(groundModColor, modColor));
            SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
            g_sdlDevice->drawTexture(texPtr, x() * SYS_MAPGRIDXP - viewX + offX, y() * SYS_MAPGRIDYP - viewY + offY);
        }

        const auto  plainModColor = colorf::WHITE + colorf::round255(ratio * 255.0);
        FixedLocMagic::drawViewOff(viewX, viewY, colorf::modRGBA(plainModColor, modColor));
    };

    if(m_accuTime < m_alphaTime[0]){
        fnDrawTexture(to_f(m_accuTime) / m_alphaTime[0]);
    }
    else if(m_accuTime < m_alphaTime[0] + m_alphaTime[1]){
        fnDrawTexture(1.0);
    }
    else if(m_accuTime < m_alphaTime[0] + m_alphaTime[1] + m_alphaTime[2]){
        fnDrawTexture(1.0 - to_f(m_accuTime - m_alphaTime[0] - m_alphaTime[1]) / m_alphaTime[2]);
    }
    else{
        // won't draw anything
    }
}

void FireWall_RUN::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    FixedLocMagic::drawViewOff(viewX, viewY, [modColor, this]() -> uint32_t
    {
        if(hasFadeOut()){
            const float r = 1.0f - (m_accuTime - m_fadeStartTime) / to_f(m_fadeDuration);
            const uint32_t plainModColor = colorf::WHITE + colorf::round255(r * 255.0);
            return colorf::modRGBA(plainModColor, modColor);
        }
        else{
            return modColor;
        }
    }());
}
