/*
 * =====================================================================================
 *
 *       Filename: pngtexoffdb.cpp
 *        Created: 06/18/21 18:55:42
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
#include "pngtexoffdb.hpp"

extern SDLDevice *g_sdlDevice;
std::optional<std::tuple<PNGTexOffElement, size_t>> PNGTexOffDB::loadResource(uint32_t key)
{
    char keyString[16];
    std::vector<uint8_t> dataBuf;

    if(const auto fontFileName = m_zsdbPtr->decomp(hexstr::to_string<uint32_t, 4>(key, keyString, true), 8, &dataBuf); fontFileName && (std::strlen(fontFileName) >= 18)){
        //
        // [0 ~ 7] [8] [9] [10 ~ 13] [14 ~ 17]
        //  <KEY>  <S> <S>   <+DX>     <+DY>
        //    4    1/2 1/2     2         2
        //
        //   KEY: 3 bytes
        //   S  : sign of DX, take 1 char, 1/2 byte, + for 1, - for 0
        //   S  : sign of DY, take 1 char, 1/2 byte
        //   +DX: abs(DX) take 4 chars, 2 bytes
        //   +DY: abs(DY) take 4 chars, 2 bytes

        if(auto texPtr = g_sdlDevice->createTexture(dataBuf.data(), dataBuf.size())){
            return std::make_tuple(PNGTexOffElement
            {
                .dx = to_d((fontFileName[8] != '0') ? 1 : (-1)) * to_d(hexstr::to_hex<uint32_t, 2>(fontFileName + 10)),
                .dy = to_d((fontFileName[9] != '0') ? 1 : (-1)) * to_d(hexstr::to_hex<uint32_t, 2>(fontFileName + 14)),

                .texture = texPtr,
            }, 1);
        }
    }
    return {};
}

void PNGTexOffDB::freeResource(PNGTexOffElement &element)
{
    if(element.texture){
        SDL_DestroyTexture(element.texture);
        element.texture = nullptr;
    }
}
