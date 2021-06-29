/*
 * =====================================================================================
 *
 *       Filename: pngtexdb.cpp
 *        Created: 02/26/2021 21:48:43
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

#include "pngtexdb.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
std::optional<std::tuple<PNGTexElement, size_t>> PNGTexDB::loadResource(uint32_t key)
{
    char keyString[16];
    if(std::vector<uint8_t> dataBuf; m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf)){
        if(auto texPtr = g_sdlDevice->loadPNGTexture(dataBuf.data(), dataBuf.size())){
            return std::make_tuple(PNGTexElement
            {
                .texture = texPtr,
            }, 1);
        }
    }
    return {};
}

void PNGTexDB::freeResource(PNGTexElement &element)
{
    if(element.texture){
        SDL_DestroyTexture(element.texture);
        element.texture = nullptr;
    }
}
