#include "pathf.hpp"
#include "sdldevice.hpp"
#include "pngtexoffdb.hpp"
#include "fixedlocmagic.hpp"

extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;

void FixedLocMagic::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    if(done()){
        return;
    }

    if(m_gfxEntry->gfxID == SYS_U32NIL){
        return;
    }

    const int gfxDirOff = ((m_gfxDirIndex >= 0) ? m_gfxDirIndex : 0) * m_gfxEntry->gfxIDCount;
    if(auto [texPtr, offX, offY] = g_magicDB->retrieve(m_gfxEntry->gfxID + gfxDirOff + frame()); texPtr){
        const auto gfxEntryModColor = m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor;
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(gfxEntryModColor, modColor));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_sdlDevice->drawTexture(texPtr, m_x * SYS_MAPGRIDXP - viewX + offX, m_y * SYS_MAPGRIDYP - viewY + offY);
    }
}

void FireAshEffect_RUN::drawGroundAsh(int viewX, int viewY, uint32_t modColor) const
{
    if(auto [texPtr, offX, offY] = g_magicDB->retrieve(0X0F0000DC); texPtr){
        const auto gfxEntryModColor = m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor;
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(colorf::modRGBA(gfxEntryModColor, getPlainModColor()), modColor));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

        const int useTexW = std::min<int>(102, texW); // crop left up part to rotate
        const int useTexH = std::min<int>( 72, texH); //

        g_sdlDevice->drawTextureEx(texPtr,
                0,
                0,
                useTexW,
                useTexH,

                x() * SYS_MAPGRIDXP - viewX + offX,
                y() * SYS_MAPGRIDYP - viewY + offY,
                useTexW,
                useTexH,

                useTexW / 2,
                useTexH / 2,

                m_rotate);
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
    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, m_fireDir, 1);

    constexpr int dx = SYS_MAPGRIDXP / 2;
    constexpr int dy = SYS_MAPGRIDYP / 2;

    m_fireRun0.drawViewOff(viewX + sgnDX * dx * 0, viewY + sgnDY * dy * 0, modColor);
    m_fireRun1.drawViewOff(viewX + sgnDX * dx * 1, viewY + sgnDY * dy * 1, modColor);
}

void IceThorn_RUN::drawGroundIce(int viewX, int viewY, uint32_t modColor) const
{
    const auto [texPtr, useTexW, useTexH, offX, offY] = [this]() -> std::tuple<SDL_Texture *, int, int, int, int>
    {
        if(m_iceSlagTexSelect){
            if(const auto [texPtr, offX, offY] = g_magicDB->retrieve(0X0F000104); texPtr){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                return {texPtr, std::min<int>(texW, 83), std::min<int>(texH, 53), offX, offY};
            }
            else{
                return {nullptr, -1, -1, -1, -1};
            }
        }
        else{
            if(const auto [texPtr, offX, offY] = g_magicDB->retrieve(0X0F000105); texPtr){
                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
                return {texPtr, std::min<int>(texW, 76), std::min<int>(texH, 43), offX, offY};
            }
            else{
                return {nullptr, -1, -1, -1, -1};
            }
        }
    }();

    if(texPtr){
        const auto gfxEntryModColor = m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor;
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(colorf::modRGBA(gfxEntryModColor, getPlainModColor()), modColor));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_sdlDevice->drawTextureEx(texPtr,
                0,
                0,
                useTexW,
                useTexH,

                x() * SYS_MAPGRIDXP - viewX + offX,
                y() * SYS_MAPGRIDYP - viewY + offY,
                useTexW,
                useTexH,

                useTexW / 2,
                useTexH / 2,

                m_rotate);
    }
}

void IceThrust_RUN::drawGroundIce(int viewX, int viewY, uint32_t modColor) const
{
    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, m_iceDir, 1);

    constexpr int dx = SYS_MAPGRIDXP / 2;
    constexpr int dy = SYS_MAPGRIDYP / 2;

    m_iceRun0.drawGroundIce(viewX + sgnDX * dx * 0, viewY + sgnDY * dy * 0, modColor);
    m_iceRun1.drawGroundIce(viewX + sgnDX * dx * 1, viewY + sgnDY * dy * 1, modColor);
}

void IceThrust_RUN::drawViewOff(int viewX, int viewY, uint32_t modColor) const
{
    const auto [sgnDX, sgnDY] = pathf::getFrontGLoc(0, 0, m_iceDir, 1);

    constexpr int dx = SYS_MAPGRIDXP / 2;
    constexpr int dy = SYS_MAPGRIDYP / 2;

    m_iceRun0.drawViewOff(viewX + sgnDX * dx * 0, viewY + sgnDY * dy * 0, modColor);
    m_iceRun1.drawViewOff(viewX + sgnDX * dx * 1, viewY + sgnDY * dy * 1, modColor);
}
