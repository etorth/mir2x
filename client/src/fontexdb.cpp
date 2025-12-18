#include "colorf.hpp"
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
    const auto ttfIndex   = to_u16((key & 0X00FFFF0000000000) >> 40);
    const auto fontStyle  = to_u8 ((key & 0X000000FF00000000) >> 32);
    const auto textEncode = to_u32((key & 0X00000000FFFFFFFF) >>  0);

    auto ttfPtr = findTTF(ttfIndex);
    if(!ttfPtr){
        return std::nullopt;
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

    char textBuf[8];
    const char * utf8String;

    if((textEncode & 0XFF000000) == 0XFF000000){
        if(auto p = m_encode2LongText.find(textEncode); p != m_encode2LongText.end()){
            utf8String = p->second;
        }
        else{
            return std::nullopt;
        }
    }
    else{
        utf8String = textBuf;
        std::memset(textBuf, 0, sizeof(textBuf));
        std::memcpy(textBuf, &textEncode, sizeof(textEncode));
    }

    SDL_Surface *surfPtr = nullptr;
    if(fontStyle & FONTSTYLE_SOLID){
        // create an texture that only has two colors: RGBA: (0, 0, 0, 0) and (255, 255, 255, 0)
        // cannot be used for SDL_BLENDMODE_BLEND because alpha channel is always 0
        surfPtr = TTF_RenderUTF8_Solid(ttfPtr, utf8String, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF));
    }
    else if(fontStyle & FONTSTYLE_SHADED){
        // create an texture that has color: (x, x, x, 0), x = 0~255
        // cannot be used for SDL_BLENDMODE_BLEND because alpha channel is always 0
        surfPtr = TTF_RenderUTF8_Shaded(ttfPtr, utf8String, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF), colorf::RGBA2SDLColor(0X00, 0X00, 0X00, 0X00));
    }
    else{
        // create an texture that has color: (255, 255, 255, x), x = 0~255
        // cannot be used for SDL_BLENDMODE_NONE, otherwise will get a white opaque block
        surfPtr = TTF_RenderUTF8_Blended(ttfPtr, utf8String, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF));
    }

    if(!surfPtr){
        return std::nullopt;
    }

    auto texPtr = g_sdlDevice->createTextureFromSurface(surfPtr);
    SDL_FreeSurface(surfPtr);

    if(texPtr){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        return std::make_tuple(FontexElement
        {
            .textEncode = textEncode,
            .texture = texPtr,
        },

        texW * texH + 50);
    }
    return std::nullopt;
}

void FontexDB::freeResource(FontexElement &element)
{
    if(element.texture){
        SDL_DestroyTexture(element.texture);
        element.texture = nullptr;

        if((element.textEncode & 0XFF000000) == 0XFF000000){
            if(auto p = m_encode2LongText.find(element.textEncode); p != m_encode2LongText.end()){
                m_longTextIndexList.push_back(to_u32(element.textEncode & 0X00FFFFFF));
                m_longText2Encode.erase(p->second);
                m_encode2LongText.erase(p);
            }
        }
    }
}
