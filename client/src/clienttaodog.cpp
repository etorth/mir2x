/*
 * =====================================================================================
 *
 *       Filename: clienttaodog.cpp
 *        Created: 08/31/2015 08:26:19
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

#include <SDL2/SDL.h>
#include "sdldevice.hpp"
#include "pngtexoffdb.hpp"
#include "clienttaodog.hpp"

extern SDLDevice *g_SDLDevice;
extern PNGTexOffDB *g_magicDB;

void ClientTaoDog::DogFire::Draw(int drawOffX, int drawOffY)
{
    if(RefreshCache()){
        if(m_cacheEntry->gfxID >= 0){
            int offX = 0;
            int offY = 0;
            if(auto texPtr = g_magicDB->Retrieve(m_cacheEntry->gfxID + Frame() + (m_direction - DIR_BEGIN) * m_cacheEntry->gfxIDCount, &offX, &offY)){
                SDL_SetTextureBlendMode(texPtr, SDL_BLENDMODE_BLEND);
                g_SDLDevice->drawTexture(texPtr, drawOffX + offX + m_dirOff.at(m_direction).at(0), drawOffY + offY + m_dirOff.at(m_direction).at(1));
            }
        }
    }
}
