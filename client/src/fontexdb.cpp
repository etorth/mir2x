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

    const auto [range, index] = decodeRange(textEncode);

    const auto useMiniToken = (range == 1);
    const auto useLongText  = (range == 3);

    const auto fnReturnValue = [useLongText, textEncode, this](SDL_Texture *texture) -> std::optional<std::tuple<FontexElement, size_t>>
    {
        // long-text is part of the resource
        // shall not return nullopt if the load has allocated a long-text

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

    auto ttf = findTTF(ttfIndex);
    if(!ttf){
        return fnReturnValue(nullptr);
    }

    TTF_SetFontKerning(ttf, useMiniToken ? 0 : 1);
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

        TTF_SetFontStyle(ttf, sdlTTFStyle);
    }

    std::string strBuf;
    const char *utf8String;

    switch(range){
        case 1:
            {
                strBuf = utf8f::code2str(index);
                utf8String = strBuf.data();
                break;
            }
        case 2:
            {
                strBuf.assign(reinterpret_cast<const char *>(&index), 4);
                utf8String = strBuf.data();
                break;
            }
        default:
            {
                utf8String = m_encode2LongText.at(textEncode);
                break;
            }
    }

    SDL_Surface *surf = nullptr;
    // if(useMiniToken){
    //
    // }
    // else
    {
        if(fontStyle & FONTSTYLE_SOLID){
            // create an texture that only has two colors: RGBA: (0, 0, 0, 0) and (255, 255, 255, 0)
            // cannot be used for SDL_BLENDMODE_BLEND because alpha channel is always 0
            surf = TTF_RenderUTF8_Solid(ttf, utf8String, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF));
        }
        else if(fontStyle & FONTSTYLE_SHADED){
            // create an texture that has color: (x, x, x, 0), x = 0~255
            // cannot be used for SDL_BLENDMODE_BLEND because alpha channel is always 0
            surf = TTF_RenderUTF8_Shaded(ttf, utf8String, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF), colorf::RGBA2SDLColor(0X00, 0X00, 0X00, 0X00));
        }
        else{
            // create an texture that has color: (255, 255, 255, x), x = 0~255
            // cannot be used for SDL_BLENDMODE_NONE, otherwise will get a white opaque block
            surf = TTF_RenderUTF8_Blended(ttf, utf8String, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF));
        }

        // put same # of transparent pixels at left side and right side
        // TTF_RenderUTF8_Blended() does not guarantee that the first visible pixel starts at x=0.
        //
        // the internal logic of SDL2_ttf is as follows:
        //
        //   int minx = 0, maxx = 0;
        //   ...
        //   minx = SDL_min(minx, glyph_left);
        //   maxx = SDL_max(maxx, glyph_right);
        //   ...
        //   xstart = (minx < 0) ? -minx : 0;
        //
        // therefore, it only handles one specific case: if a glyph extends into x < 0,
        // the entire surface is shifted right to prevent the left side from being clipped. However,
        // if the first glyph has a positive left bearing (i.e., minx > 0), SDL2_ttf will NOT crop it to 0, the leading transparent pixels are preserved.
        //
        // as a result:
        //   - First glyph minx < 0  -> SDL_ttf shifts right, the leftmost visible pixel usually aligns to x=0.
        //   - First glyph minx = 0  -> Usually starts exactly at x=0.
        //   - First glyph minx > 0  -> Leading transparent pixels will be present on the left.
        //   - First char is a space -> The left side will be entirely transparent, accounting only for the advance.
        //
        // but SDL2_ttf extends the total surface width, using the advance width of the last character:
        //
        //   maxx = SDL_max(maxx, FT_FLOOR(x + prev_advance));
        //
        // in conclusion, the returned surface represents a layout texture rather than an alpha-tight cropped texture,
        // transparent pixels may exist on both the left and right margins, though trailing advance padding on the right is more common.

        // put same # of transparent pixels at left side and right side
        // skip if leading or tailing glyph is transparent

        if(surf){
            const auto firstCodePoint = utf8f::str2code(utf8f::peekFirst(utf8String));
            const auto  lastCodePoint = utf8f::str2code(utf8f::peekLast (utf8String));

            if(!isTransparant(ttf, firstCodePoint) && !isTransparant(ttf, lastCodePoint)){
                const auto [leftMinX,         _, _, _,            _] = getGlyphMetrics(ttf, firstCodePoint);
                const auto [       _, rightMaxX, _, _, rightAdvance] = getGlyphMetrics(ttf, lastCodePoint );

                const int padLeft  = std::max<int>(0,                 leftMinX);
                const int padRight = std::max<int>(0, rightAdvance - rightMaxX);

                const int addLeft  = std::max<int>(0, padRight - padLeft );
                const int addRight = std::max<int>(0, padLeft  - padRight);

                if(addLeft || addRight){
                    if(auto padded = SDL_CreateRGBSurfaceWithFormat(0, surf->w + addLeft + addRight, surf->h, 32, surf->format->format)){
                        SDL_FillRect(padded, nullptr, SDL_MapRGBA(padded->format, 0, 0, 0, 0));

                        SDL_Rect dst{addLeft, 0, surf->w, surf->h};
                        SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);
                        SDL_BlitSurface(surf, nullptr, padded, &dst);

                        SDL_FreeSurface(surf);
                        surf = padded;
                    }
                }
            }
        }
    }

    if(!surf){
        return fnReturnValue(nullptr);
    }

    auto tex = g_sdlDevice->createTextureFromSurface(surf);
    SDL_FreeSurface(surf);

    return fnReturnValue(tex); // tex can be nullptr
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

uint32_t FontexDB::hasGlphy(TTF_Font *font, uint32_t codePoint)
{
    return TTF_GlyphIsProvided32(font, codePoint);
}

bool FontexDB::isTransparant(TTF_Font *font, uint32_t codePoint)
{
    int minx = 0;
    int maxx = 0;
    int miny = 0;
    int maxy = 0;
    int advance = 0;

    if(TTF_GlyphMetrics32(font, codePoint, &minx, &maxx, &miny, &maxy, &advance)){
        throw fflpanic("failed to query glphy metrics: {}", codePoint);
    }

    return minx == 0
        && maxx == 0
        && miny == 0
        && maxy == 0; // TBD: should I check advance > 0 ?
}

std::tuple<int, int, int, int, int> FontexDB::getGlyphMetrics(TTF_Font *font, uint32_t codePoint)
{
    int minx;
    int maxx;
    int miny;
    int maxy;
    int advance;

    if(TTF_GlyphMetrics32(font, codePoint, &minx, &maxx, &miny, &maxy, &advance)){
        throw fflpanic("failed to get glyph metrics: {}", codePoint);
    }

    return {minx, maxx, miny, maxy, advance};
}
