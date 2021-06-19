/*
 * =====================================================================================
 *
 *       Filename: fontexdb.hpp
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

#pragma once
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "utf8f.hpp"
#include "fflerror.hpp"
#include "hexstr.hpp"
#include "sdldevice.hpp"

enum FontStyle: uint8_t
{
    FONTSTYLE_BOLD          = 0B0000'0001,
    FONTSTYLE_ITALIC        = 0B0000'0010,
    FONTSTYLE_UNDERLINE     = 0B0000'0100,
    FONTSTYLE_STRIKETHROUGH = 0B0000'1000,
    FONTSTYLE_SOLID         = 0B0001'0000,
    FONTSTYLE_SHADED        = 0B0010'0000,
    FONTSTYLE_BLENDED       = 0B0100'0000,
};

struct FontexElement
{
    SDL_Texture *texture = nullptr;
};

class FontexDB: public innDB<uint64_t, FontexElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;
        std::vector<ZSDB::Entry> m_entryList;

    private:
        // 0XFF00 : font index
        // 0X00FF : font point size
        std::unordered_map<uint16_t, TTF_Font *> m_ttfCache;

    private:
        std::unordered_map<uint8_t, std::vector<uint8_t>> m_fontDataCache;

    public:
        FontexDB(size_t resMax)
            : innDB<uint64_t, FontexElement>(resMax)
        {}

        virtual ~FontexDB()
        {
            for(auto &p: m_ttfCache){
                TTF_CloseFont(p.second);
            }
        }

    private:
        const std::vector<uint8_t> &findFontData(uint8_t fontIndex)
        {
            if(auto p = m_fontDataCache.find(fontIndex); p != m_fontDataCache.end()){
                return p->second;
            }

            return m_fontDataCache[fontIndex] = [this, fontIndex]() -> std::vector<uint8_t>
            {
                char fontIndexString[8];
                std::vector<uint8_t> fontDataBuf;

                if(m_zsdbPtr->decomp(hexstr::to_string<uint8_t, 1>(fontIndex, fontIndexString, true), 2, &fontDataBuf)){
                    return fontDataBuf;
                }
                return {};
            }();
        }

    private:
        TTF_Font *findTTF(uint16_t);

    public:
        bool load(const char *fontDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(fontDBName);
            m_entryList = m_zsdbPtr->getEntryList();
            return true;
        }

    public:
        SDL_Texture *retrieve(uint64_t key)
        {
            if(auto p = innLoad(key)){
                return p->texture;
            }
            return nullptr;
        }

        SDL_Texture *retrieve(uint8_t fontIndex, uint8_t fontSize, uint8_t fontStyle, uint32_t utf8Code)
        {
            return retrieve(utf8f::buildU64Key(fontIndex, fontSize, fontStyle, utf8Code));
        }

    public:
        uint8_t findFontName(const char *fontName)
        {
            fflassert(str_haschar(fontName));
            const auto fileName = str_printf("%s.TTF", fontName);

            for(const auto &entry: m_entryList){
                if(fileName == entry.fileName + 3){
                    return hexstr::to_hex<uint8_t, 1>(entry.fileName);
                }
            }
            return 0;
        }

        bool hasFont(uint8_t font)
        {
            return !findFontData(font).empty();
        }

    public:
        std::optional<std::tuple<FontexElement, size_t>> loadResource(uint64_t) override;

    public:
        void freeResource(FontexElement &) override;
};
