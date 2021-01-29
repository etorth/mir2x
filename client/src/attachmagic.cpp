/*
 * =====================================================================================
 *
 *       Filename: attachmagic.cpp
 *        Created: 08/10/2017 12:46:45
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

#include "dbcomid.hpp"
#include "sdldevice.hpp"
#include "dbcomrecord.hpp"
#include "attachmagic.hpp"
#include "pngtexoffdb.hpp"

extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_magicDB;

void AttachMagic::drawShift(int shiftX, int shiftY, bool alpha)
{
    if(m_gfxEntry->gfxID == SYS_TEXNIL){
        return;
    }

    const auto texID = [this]() -> uint32_t
    {
        switch(m_gfxEntry->gfxDirType){
            case  1: return m_gfxEntry->gfxID + frame();
            case  4:
            case  8:
            case 16: return m_gfxEntry->gfxID + frame() + m_gfxDirIndex * m_gfxEntry->gfxIDCount;
            default: throw fflerror("invalid gfxDirType: %d", m_gfxEntry->gfxDirType);
        }
    }();

    int offX = 0;
    int offY = 0;
    if(auto texPtr = g_magicDB->Retrieve(texID, &offX, &offY)){
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::RGBA(0XFF, 0XFF, 0XFF, alpha ? 0X40 : 0XC0));
        SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, SDL_BLENDMODE_BLEND);
        g_sdlDevice->drawTexture(texPtr, shiftX + offX, shiftY + offY);
    }
}
