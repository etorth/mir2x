/*
 * =====================================================================================
 *
 *       Filename: fontexdb.hpp
 *        Created: 02/24/2016 17:51:16
 *    Description: this class only releases resource automatically
 *                 on loading new resources
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
#include <map>
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "zsdb.hpp"
#include "inndb.hpp"
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

struct FontexEntry
{
    SDL_Texture *Texture;
};

class FontexDB: public innDB<uint64_t, FontexEntry>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;
        std::vector<ZSDB::Entry> m_entryList;

    private:
        // 0XFF00 : font index
        // 0X00FF : font point size
        std::map<uint16_t, TTF_Font *> m_TTFCache;

    private:
        std::map<uint8_t, std::vector<uint8_t>> m_fontDataCache;

    public:
        FontexDB(size_t nResMax)
            : innDB<uint64_t, FontexEntry>(nResMax)
            , m_zsdbPtr()
            , m_TTFCache()
            , m_fontDataCache()
        {}

        virtual ~FontexDB()
        {
            for(auto &stEntry: m_TTFCache){
                TTF_CloseFont(stEntry.second);
            }
        }

    private:
        const std::vector<uint8_t> &RetrieveFontData(uint8_t nFontIndex)
        {
            if(auto p = m_fontDataCache.find(nFontIndex); p != m_fontDataCache.end()){
                return p->second;
            }

            m_fontDataCache[nFontIndex] = [this, nFontIndex]() -> std::vector<uint8_t>
            {
                char szFontIndexString[8];
                std::vector<uint8_t> stFontDataBuf;
                if(m_zsdbPtr->Decomp(hexstr::to_string<uint8_t, 1>(nFontIndex, szFontIndexString, true), 2, &stFontDataBuf)){
                    return stFontDataBuf;
                }
                return {};
            }();

            return m_fontDataCache[nFontIndex];
        }

        TTF_Font *RetrieveTTF(uint16_t nTTFIndex)
        {
            uint8_t nFontIndex = ((nTTFIndex & 0XFF00) >> 8);
            uint8_t nFontSize  = ((nTTFIndex & 0X00FF) >> 0);

            if(auto p = m_TTFCache.find(nTTFIndex); p != m_TTFCache.end()){
                return p->second;
            }

            m_TTFCache[nTTFIndex] = [this, nFontSize, nFontIndex]() -> TTF_Font *
            {
                if(auto &stFontDataBuf = RetrieveFontData(nFontIndex); !stFontDataBuf.empty()){
                    extern SDLDevice *g_sdlDevice;
                    return g_sdlDevice->CreateTTF(stFontDataBuf.data(), stFontDataBuf.size(), nFontSize);
                }
                return nullptr;
            }();

            return m_TTFCache[nTTFIndex];
        }

    public:
        bool Load(const char *szFontexDBName)
        {
            try{
                m_zsdbPtr = std::make_unique<ZSDB>(szFontexDBName);
                m_entryList = m_zsdbPtr->GetEntryList();
            }catch(...){
                return false;
            }
            return true;
        }

    public:
        SDL_Texture *Retrieve(uint64_t nKey)
        {
            if(FontexEntry stEntry {nullptr}; this->RetrieveResource(nKey, &stEntry)){
                return stEntry.Texture;
            }
            return nullptr;
        }

        SDL_Texture *Retrieve(uint8_t nFontIndex, uint8_t nFontSize, uint8_t nFontStyle, uint32_t nUTF8Code)
        {
            uint64_t nKey = 0
                + (((uint64_t)nFontIndex) << 48)
                + (((uint64_t)nFontSize ) << 40)
                + (((uint64_t)nFontStyle) << 32)
                + (((uint64_t)nUTF8Code)  <<  0);
            return Retrieve(nKey);
        }

    public:
        uint8_t findFontName(const char *fontName)
        {
            if(!fontName){
                return 0;
            }

            const auto fileName = str_printf("%s.TTF", fontName);
            for(const auto &entry: m_entryList){
                if(fileName == entry.FileName + 3){
                    return hexstr::to_hex<uint8_t, 1>(entry.FileName);
                }
            }
            return 0;
        }

        bool hasFont(uint8_t font)
        {
            return !RetrieveFontData(font).empty();
        }

    public:
        virtual std::tuple<FontexEntry, size_t> loadResource(uint64_t nKey)
        {
            FontexEntry stEntry {nullptr};

            uint16_t nTTFIndex  = ((nKey & 0X00FFFF0000000000) >> 40);
            uint8_t  nFontStyle = ((nKey & 0X000000FF00000000) >> 32);
            uint32_t nUTF8Code  = ((nKey & 0X00000000FFFFFFFF) >>  0);

            auto pFont = RetrieveTTF(nTTFIndex);
            if(!pFont){
                return {stEntry, 0};
            }

            TTF_SetFontKerning(pFont, 0);

            if(nFontStyle){
                int nTTFontStyle = 0;
                if(nFontStyle & FONTSTYLE_BOLD){
                    nTTFontStyle &= TTF_STYLE_BOLD;
                }

                if(nFontStyle & FONTSTYLE_ITALIC){
                    nTTFontStyle &= TTF_STYLE_ITALIC;
                }

                if(nFontStyle & FONTSTYLE_UNDERLINE){
                    nTTFontStyle &= TTF_STYLE_UNDERLINE;
                }

                if(nFontStyle & FONTSTYLE_STRIKETHROUGH){
                    nTTFontStyle &= TTF_STYLE_STRIKETHROUGH;
                }

                TTF_SetFontStyle(pFont, nTTFontStyle);
            }

            char szUTF8[8];
            SDL_Surface *pSurface = nullptr;

            szUTF8[4] = 0;
            std::memcpy(szUTF8, &nUTF8Code, sizeof(nUTF8Code));

            if(nFontStyle & FONTSTYLE_SOLID){
                pSurface = TTF_RenderUTF8_Solid(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF});
            }else if(nFontStyle & FONTSTYLE_SHADED){
                pSurface = TTF_RenderUTF8_Shaded(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF}, {0X00, 0X00, 0X00, 0X00});
            }else{
                // blended is by default but with lowest priority
                // mask the bits if we really need to set SOLID/SHADOWED/BLENDED
                pSurface = TTF_RenderUTF8_Blended(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF});
            }

            if(!pSurface){
                return {stEntry, 0};
            }

            extern SDLDevice *g_sdlDevice;
            stEntry.Texture = g_sdlDevice->CreateTextureFromSurface(pSurface);
            SDL_FreeSurface(pSurface);

            return {stEntry, stEntry.Texture ? 1 : 0};
        }

        virtual void freeResource(FontexEntry &rstEntry)
        {
            if(rstEntry.Texture){
                SDL_DestroyTexture(rstEntry.Texture);
                rstEntry.Texture = nullptr;
            }
        }
};
