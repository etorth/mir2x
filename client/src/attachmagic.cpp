#include "colorf.hpp"
#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "attachmagic.hpp"
#include "magicdb.hpp"

extern SDLDevice *g_sdlDevice;
extern MagicDB *g_magicDB;

void AttachMagic::drawShift(int shiftX, int shiftY, uint32_t modColor) const
{
    if(m_gfxEntry->gfxID == SYS_U32NIL){
        return;
    }

    if(auto [texPtr, offX, offY] = g_magicDB->retrieve(gfxSeqID(), frame()); texPtr){
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor, modColor));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_sdlDevice->drawTexture(texPtr, shiftX + offX, shiftY + offY);
    }
}

void Thunderbolt::drawShift(int shiftX, int shiftY, uint32_t modColor) const
{
    if(m_gfxEntry->gfxID == SYS_U32NIL){
        return;
    }

    if(auto [texPtr, offX, offY] = g_magicDB->retrieve(gfxSeqID(), frame()); texPtr){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::modRGBA(m_gfxEntryRef ? m_gfxEntryRef->modColor : m_gfxEntry->modColor, modColor));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);

        // thunder bolt has 5 frames
        // frame 0 ~ 3 are long, last frame is short
        g_sdlDevice->drawTexture(texPtr, shiftX + offX, shiftY + offY);
        if(frame() <= 3){
            g_sdlDevice->drawTextureEx(texPtr, 0, 0, texW, texH, shiftX + offX, shiftY + offY - texH, texW, texH, 0, 0, 0, SDL_FLIP_VERTICAL);
        }
    }
}

void TaoYellowBlueRing::drawShift(int shiftX, int shiftY, uint32_t modColor) const
{
    const auto r = std::fabs(std::cos(m_accuTime / 800.0));
    const auto timedModColor = colorf::RGBA(0XFF, 0XFF, 0XFF, std::max<uint8_t>(colorf::round255(r * 255), 32));
    AttachMagic::drawShift(shiftX, shiftY, colorf::modRGBA(timedModColor, modColor));
}

void AntHealing::drawShift(int shiftX, int shiftY, uint32_t modColor) const
{
    AttachMagic::drawShift(shiftX, shiftY - frame() * 3, modColor);
}
