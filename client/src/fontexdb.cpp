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

    const auto fnReturnValue = [textEncode, this](SDL_Texture *texture) -> std::optional<std::tuple<FontexElement, size_t>>
    {
        // long-text is part of the resource
        // shall not return nullopt if the load has allocated a long-text

        const auto useLongText = (decodeRange(textEncode).first == 3);
        if(useLongText){
            if(auto &refCount = m_longText2Encode.at(m_encode2LongText.at(textEncode)).second; ++refCount == 0){
                throw fflpanic("reference count for textEncode {} overflows", textEncode);
            }
        }

        if(texture){
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texture);
            return std::make_tuple(FontexElement{textEncode, texture}, texW * texH + 50);
        }
        else if(useLongText){
            return std::make_tuple(FontexElement{textEncode, nullptr}, 1);
        }
        else{
            return std::nullopt;
        }
    };

    auto ttfPtr = findTTF(ttfIndex);
    if(!ttfPtr){
        return fnReturnValue(nullptr);
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

    std::string strBuf;
    const char *utf8String;

    switch(const auto [range, val] = decodeRange(textEncode); range){
        case 1:
            {
                strBuf = utf8f::code2str(val);
                utf8String = strBuf.data();
                break;
            }
        case 2:
            {
                strBuf.assign(reinterpret_cast<const char *>(&val), 4);
                utf8String = strBuf.data();
                break;
            }
        default:
            {
                utf8String = m_encode2LongText.at(textEncode);
                break;
            }
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
        return fnReturnValue(nullptr);
    }

    auto texPtr = g_sdlDevice->createTextureFromSurface(surfPtr);
    SDL_FreeSurface(surfPtr);

    return fnReturnValue(texPtr); // texPtr can be nullptr
}

void FontexDB::freeResource(FontexElement &element)
{
    if(element.texture){
        SDL_DestroyTexture(element.texture);
        element.texture = nullptr;
    }

    if(const auto [range, index] = decodeRange(element.textEncode); range == 3){
        auto p = m_encode2LongText.find(element.textEncode); fflassert(p != m_encode2LongText.end(), element.textEncode);
        auto q = m_longText2Encode.find(p->second         ); fflassert(q != m_longText2Encode.end(), *p                );

        if(q->second.second > 1){
            q->second.second--;
        }
        else{
            m_encode2LongText.erase(p);
            m_longText2Encode.erase(q);
            m_longTextIndexList.push_back(index);
        }
    }
}
