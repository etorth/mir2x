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

TTF_Font *FontexDB::findTTF(uint8_t ttfIndex, uint8_t ttfSize)
{
    return findTTF(utf8f::buildTTFIndex(ttfIndex, ttfSize));
}

std::optional<std::tuple<FontexElement, size_t>> FontexDB::loadResource(uint64_t key)
{
    const auto [fontIndex, fontSize, fontStyle, textEncode] = utf8f::extractU64Key(key);
    const auto ttfIndex = utf8f::buildTTFIndex(fontIndex, fontSize);
    const auto [range, index] = decodeRange(textEncode);

    const auto useMiniToken = (range == 1);
    const auto useLongText  = (range == 3);

    FontexElement result {.textEncode = textEncode};
    const auto fnReturnValue = [useLongText, &result, this] -> std::optional<std::tuple<FontexElement, size_t>>
    {
        // long-text is part of the resource
        // shall not return nullopt if the load has allocated a long-text

        if(useLongText){
            if(auto &refCount = m_longText2Encode.at(m_encode2LongText.at(result.textEncode)).second; ++refCount == 0){
                throw fflpanic("reference count for textEncode {} overflows", result.textEncode);
            }
        }

        if(result.texture){
            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(result.texture);
            return std::make_tuple(result, texW * texH + 50);
        }
        else if(useLongText){
            return std::make_tuple(result, 1);
        }
        else{
            return std::nullopt;
        }
    };

    auto ttf = findTTF(ttfIndex);
    if(!ttf){
        return fnReturnValue();
    }

    TTF_SetFontKerning(ttf, useMiniToken ? 0 : 1);
    {
        int sdlTTFStyle = 0;
        if(fontStyle & FONTSTYLE_BOLD){
            sdlTTFStyle |= TTF_STYLE_BOLD;
        }

        if(fontStyle & FONTSTYLE_ITALIC){
            sdlTTFStyle |= TTF_STYLE_ITALIC;
        }

        if(fontStyle & FONTSTYLE_UNDERLINE){
            sdlTTFStyle |= TTF_STYLE_UNDERLINE;
        }

        if(fontStyle & FONTSTYLE_STRIKETHROUGH){
            sdlTTFStyle |= TTF_STYLE_STRIKETHROUGH;
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
    if(useMiniToken){
        if(hasGlphy(ttf, index)){
            const auto metrics = getGlyphMetrics(ttf, index);

            const auto minx    = std::get<0>(metrics);
            const auto maxy    = std::get<3>(metrics);
            const auto advance = std::get<4>(metrics);

            const auto padding = getGlyphPadding  (metrics);
            const auto pixSize = getGlyphPixelSize(metrics);

            if(pixSize.first <= 0 || pixSize.second <= 0){
                fflassert(isTransparant(ttf, index), index);
                const auto emptyPixSize = getGlyphPixelSize(ttf, utf8f::str2code("a"));

                fflassert(emptyPixSize.first  > 0, emptyPixSize);
                fflassert(emptyPixSize.second > 0, emptyPixSize);

                if((surf = SDL_CreateRGBSurfaceWithFormat(0, advance, emptyPixSize.second, 32, SDL_PIXELFORMAT_ARGB8888))){
                    if(fontStyle & FONTSTYLE_SOLID){
                        SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
                    }
                    else if(fontStyle & FONTSTYLE_SHADED){
                        SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
                    }
                    else{
                        SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 255, 255, 255, 0));
                    }

                    result.left   = 0;
                    result.right  = 0;
                    result.ascent = surf->h;
                }
            }
            else{
                if(fontStyle & FONTSTYLE_SOLID){
                    // create an texture that only has two colors: RGBA: (0, 0, 0, 0) and (255, 255, 255, 0)
                    // cannot be used for SDL_BLENDMODE_BLEND because alpha channel is always 0
                    surf = TTF_RenderGlyph32_Solid(ttf, index, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF));
                }
                else if(fontStyle & FONTSTYLE_SHADED){
                    // create an texture that has color: (x, x, x, 0), x = 0~255
                    // cannot be used for SDL_BLENDMODE_BLEND because alpha channel is always 0
                    surf = TTF_RenderGlyph32_Shaded(ttf, index, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF), colorf::RGBA2SDLColor(0X00, 0X00, 0X00, 0X00));
                }
                else{
                    // create an texture that has color: (255, 255, 255, x), x = 0~255
                    // cannot be used for SDL_BLENDMODE_NONE, otherwise will get a white opaque block
                    surf = TTF_RenderGlyph32_Blended(ttf, index, colorf::RGBA2SDLColor(0XFF, 0XFF, 0XFF, 0XFF));
                }

                if(surf){
                    fflassert(pixSize.first  <= surf->w);
                    fflassert(pixSize.second <= surf->h);

                    // setup padding
                    // as default if crop failed or crop not happen
                    result.left   = 0;
                    result.right  = 0;
                    result.ascent = to_i32(std::get<2>(padding));

                    if(pixSize.first < surf->w || pixSize.second < surf->h){
                        if(auto minisurf = SDL_CreateRGBSurfaceWithFormat(0, pixSize.first, pixSize.second, 32, SDL_PIXELFORMAT_ARGB8888)){
                            SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);

                            // in SDL_ttf 2.24, TTF_RenderGlyph32_* internally routes through TTF_RenderUTF8_*, which returns a surface tailored to the full line height.
                            // the actual bounding box of the single glyph bitmap within this surface is located at (xstart + minx, ystart + ascent - maxy).

                            SDL_Rect src
                            {
                                std::max<int>(0, minx),
                                std::max<int>(0, TTF_FontAscent(ttf) - maxy),
                                pixSize.first,
                                pixSize.second,
                            };

                            if(SDL_BlitSurface(surf, &src, minisurf, nullptr)){
                                SDL_FreeSurface(minisurf); // blit failed, use original surface
                            }
                            else{
                                SDL_FreeSurface(surf);
                                surf = minisurf;
                                result.left  = to_i32(std::get<0>(padding));
                                result.right = to_i32(std::get<1>(padding));
                            }
                        }
                    }
                }
            }
        }
        else{
            // font doesn't support this glyph
            // we manually create a texture with a white cross: [x]
            const auto metrics = getGlyphMetrics(ttf, utf8f::str2code("a"));
            const auto padding = getGlyphPadding  (metrics);
            const auto pixSize = getGlyphPixelSize(metrics);

            fflassert(pixSize.first  > 0, pixSize);
            fflassert(pixSize.second > 0, pixSize);

            if((surf = SDL_CreateRGBSurfaceWithFormat(0, pixSize.first, pixSize.second, 32, SDL_PIXELFORMAT_ARGB8888))){
                if(fontStyle & FONTSTYLE_SOLID){
                    SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
                }
                else if(fontStyle & FONTSTYLE_SHADED){
                    SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
                }
                else{
                    SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, 255, 255, 255, 0));
                }

                const int thickness = std::max<int>(1, std::min<int>(surf->w, surf->h) / 6);
                const auto xColor = [fontStyle, surf]
                {
                    if(fontStyle & FONTSTYLE_SOLID){
                        return SDL_MapRGBA(surf->format, 255, 255, 255, 0);
                    }
                    else if(fontStyle & FONTSTYLE_SHADED){
                        return SDL_MapRGBA(surf->format, 255, 255, 255, 0);
                    }
                    else{
                        return SDL_MapRGBA(surf->format, 255, 255, 255, 255);
                    }
                }();

                SDL_Rect top    = { 0                  ,                   0,   surf->w, thickness };
                SDL_Rect bottom = { 0                  , surf->h - thickness,   surf->w, thickness };
                SDL_Rect left   = { 0                  ,                   0, thickness,   surf->h };
                SDL_Rect right  = { surf->w - thickness,                   0, thickness,   surf->h };

                SDL_FillRect(surf, &top   , xColor);
                SDL_FillRect(surf, &bottom, xColor);
                SDL_FillRect(surf, &left  , xColor);
                SDL_FillRect(surf, &right , xColor);

                int innerW = surf->w - (thickness * 2);
                int innerH = surf->h - (thickness * 2);

                for(int i = 0; i < innerW; i++){
                    const auto y = thickness + (i * innerH) / innerW;

                    SDL_Rect p1 {           thickness + i    , y, thickness, thickness }; // up-left  -> down-right
                    SDL_Rect p2 { surf->w - thickness - i - 1, y, thickness, thickness }; // up-right -> down-left

                    SDL_FillRect(surf, &p1, xColor);
                    SDL_FillRect(surf, &p2, xColor);
                }

                result.left   = to_i32(std::get<0>(padding));
                result.right  = to_i32(std::get<1>(padding));
                result.ascent = to_i32(std::get<2>(padding));
            }
        }
    }
    else{
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
                    if(auto padded = SDL_CreateRGBSurfaceWithFormat(0, surf->w + addLeft + addRight, surf->h, 32, SDL_PIXELFORMAT_ARGB8888)){
                        if(fontStyle & FONTSTYLE_SOLID){
                            SDL_FillRect(padded, nullptr, SDL_MapRGBA(padded->format, 0, 0, 0, 0));
                        }
                        else if(fontStyle & FONTSTYLE_SHADED){
                            SDL_FillRect(padded, nullptr, SDL_MapRGBA(padded->format, 0, 0, 0, 0));
                        }
                        else{
                            SDL_FillRect(padded, nullptr, SDL_MapRGBA(padded->format, 255, 255, 255, 0));
                        }

                        SDL_Rect dst{addLeft, 0, surf->w, surf->h};
                        SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);

                        if(SDL_BlitSurface(surf, nullptr, padded, &dst)){
                            SDL_FreeSurface(padded);
                            result.left = addLeft;
                            result.right = addRight;
                        }
                        else{
                            SDL_FreeSurface(surf);
                            surf = padded;
                            result.left = 0;
                            result.right = 0;
                        }

                        result.ascent = to_u32(TTF_FontAscent(ttf)); // can have 1 pixel shift for bitmap fonts
                    }
                }
            }
        }
    }

    if(surf){
        result.texture = g_sdlDevice->createTextureFromSurface(surf);
        SDL_FreeSurface(surf);
    }

    return fnReturnValue(); // result.texture can be nullptr
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

uint32_t FontexDB::hasGlphy(uint16_t ttfIndex, uint32_t codePoint)
{
    return hasGlphy(findTTF(ttfIndex), codePoint);
}

uint32_t FontexDB::hasGlphy(uint8_t fontIndex, uint8_t fontSize, uint32_t codePoint)
{
    return hasGlphy(findTTF(fontIndex, fontSize), codePoint);
}

bool FontexDB::isTransparant(TTF_Font *font, uint32_t codePoint)
{
    return isTransparant(getGlyphMetrics(font, codePoint));
}

bool FontexDB::isTransparant(const std::tuple<int, int, int, int, int> &t)
{
    const auto [minx, maxx, miny, maxy, _] = t;
    return minx == 0
        && maxx == 0
        && miny == 0
        && maxy == 0; // TBD: should I check advance > 0 ?
}

bool FontexDB::isTransparant(uint16_t ttfIndex, uint32_t codePoint)
{
    return isTransparant(getGlyphMetrics(findTTF(ttfIndex), codePoint));
}

bool FontexDB::isTransparant(uint8_t fontIndex, uint8_t fontSize, uint32_t codePoint)
{
    return isTransparant(getGlyphMetrics(findTTF(fontIndex, fontSize), codePoint));
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

std::tuple<int, int, int> FontexDB::getGlyphPadding(const std::tuple<int, int, int, int, int> &t)
{
    const auto [minx, maxx, _, maxy, advance] = t;
    return {minx, advance - maxx, maxy};
}

std::pair<int, int> FontexDB::getGlyphPixelSize(const std::tuple<int, int, int, int, int> &t)
{
    const auto [minx, maxx, miny, maxy, _] = t;
    return {maxx - minx, maxy - miny};
}
