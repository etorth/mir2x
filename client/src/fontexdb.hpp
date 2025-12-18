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
#include "fontstyle.hpp"

struct FontexElement
{
    uint32_t textEncode = 0;
    SDL_Texture *texture = nullptr;
};

// key & 0X00FF000000000000) >> 48: font index --+
// key & 0X0000FF0000000000) >> 40: font size  --+---> combined as ttf index
// key & 0X000000FF00000000) >> 32: font style
// key & 0X00000000FFFFFFFF) >>  0: utf8 code or long text encode

// use lower 4 bytes as utf8 code or long text encode
// when it's: 0XFFXXXXXX: long text encode
//            0XXXXXXXXX: ottherwise, it's utf8 code with length <= 4

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

    private:
        uint32_t m_longTextIndexMax = 0;
        std::vector<uint32_t> m_longTextIndexList;

    private:
        std::unordered_map<std::string, uint32_t > m_longText2Encode;
        std::unordered_map<uint32_t, const char *> m_encode2LongText;

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
        void load(const char *fontDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(fontDBName);
            m_entryList = m_zsdbPtr->getEntryList();
        }

    public:
        SDL_Texture *retrieve(uint64_t key)
        {
            if(auto p = innLoad(key)){
                return p->texture;
            }
            return nullptr;
        }

        SDL_Texture *retrieve(uint8_t fontIndex, uint8_t fontSize, uint8_t fontStyle, const char *utf8String)
        {
            return retrieve(utf8f::buildU64Key(fontIndex, fontSize, fontStyle, textEncode(utf8String)));
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

    private:
        uint32_t textEncode(const char *utf8String)
        {
            fflassert(utf8String);

            if(const auto size = std::strlen(utf8String); size <= 4){
                uint32_t utf8Code = 0;
                std::memcpy(&utf8Code, utf8String, size);
                return utf8Code;
            }

            if(auto p = m_longText2Encode.find(utf8String); p != m_longText2Encode.end()){
                return p->second;
            }

            const auto currIndex = [this] -> uint32_t
            {
                if(m_longTextIndexList.empty()){
                    return ++m_longTextIndexMax;
                }
                else{
                    const auto index = m_longTextIndexList.back();
                    m_longTextIndexList.pop_back();
                    return index;
                }
            }();

            if(currIndex > 0X00FFFFFF){
                throw fflerror("long text count exceeds limit: %llu", to_llu(currIndex));
            }

            const auto encodedIndex = currIndex | UINT32_C(0XFF000000);
            const auto insertedString = m_longText2Encode.try_emplace(utf8String, encodedIndex);
            const auto insertedEncode = m_encode2LongText.try_emplace(encodedIndex, insertedString.first->first.c_str());

            fflassert(insertedString.second);
            fflassert(insertedEncode.second);

            return encodedIndex;
        }
};
