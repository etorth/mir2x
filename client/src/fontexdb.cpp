/*
 * =====================================================================================
 *
 *       Filename: fontexdb.cpp
 *        Created: 02/24/2016 17:51:16
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

#include "fontexdb.hpp"
#include "sdldevice.hpp"

extern SDLDevice *g_sdlDevice;
TTF_Font *FontexDB::findTTF(uint16_t ttfIndex)
{
    if(auto p = m_ttfCache.find(ttfIndex); p != m_ttfCache.end()){
        return p->second;
    }

    return m_ttfCache[ttfIndex] = [this, ttfIndex]() -> TTF_Font *
    {
        const uint8_t fontIndex = to_u8((ttfIndex & 0XFF00) >> 8);
        const uint8_t fontSize  = to_u8((ttfIndex & 0X00FF) >> 0);

        if(auto &fontDataBuf = findFontData(fontIndex); !fontDataBuf.empty()){
            return g_sdlDevice->createTTF(fontDataBuf.data(), fontDataBuf.size(), fontSize);
        }
        return nullptr;
    }();
}

std::optional<std::tuple<FontexElement, size_t>> FontexDB::loadResource(uint64_t key)
{
    const auto ttfIndex  = to_u16((key & 0X00FFFF0000000000) >> 40);
    const auto fontStyle = to_u8 ((key & 0X000000FF00000000) >> 32);
    const auto utf8Code  = to_u32((key & 0X00000000FFFFFFFF) >>  0);

    auto ttfPtr = findTTF(ttfIndex);
    if(!ttfPtr){
        return {};
    }

    TTF_SetFontKerning(ttfPtr, 0);
    if(fontStyle){
        int sdlTTFStyle = 0;
        if(fontStyle & FONTSTYLE_BOLD){
            sdlTTFStyle &= TTF_STYLE_BOLD;
        }

        if(fontStyle & FONTSTYLE_ITALIC){
            sdlTTFStyle &= TTF_STYLE_ITALIC;
        }

        if(fontStyle & FONTSTYLE_UNDERLINE){
            sdlTTFStyle &= TTF_STYLE_UNDERLINE;
        }

        if(fontStyle & FONTSTYLE_STRIKETHROUGH){
            sdlTTFStyle &= TTF_STYLE_STRIKETHROUGH;
        }

        TTF_SetFontStyle(ttfPtr, sdlTTFStyle);
    }

    char utf8String[8];
    std::memset(utf8String, 0, sizeof(utf8String));
    std::memcpy(utf8String, &utf8Code, sizeof(utf8Code));

    SDL_Surface *surfPtr = nullptr;
    if(fontStyle & FONTSTYLE_SOLID){
        surfPtr = TTF_RenderUTF8_Solid(ttfPtr, utf8String, {0XFF, 0XFF, 0XFF, 0XFF});
    }
    else if(fontStyle & FONTSTYLE_SHADED){
        surfPtr = TTF_RenderUTF8_Shaded(ttfPtr, utf8String, {0XFF, 0XFF, 0XFF, 0XFF}, {0X00, 0X00, 0X00, 0X00});
    }
    else{
        surfPtr = TTF_RenderUTF8_Blended(ttfPtr, utf8String, {0XFF, 0XFF, 0XFF, 0XFF});
    }

    if(!surfPtr){
        return {};
    }

    auto texPtr = g_sdlDevice->createTextureFromSurface(surfPtr);
    SDL_FreeSurface(surfPtr);

    if(texPtr){
        return std::make_tuple(FontexElement
        {
            .texture = texPtr,
        }, 1);
    }
    return {};
}

void FontexDB::freeResource(FontexElement &element)
{
    if(element.texture){
        SDL_DestroyTexture(element.texture);
        element.texture = nullptr;
    }
}
