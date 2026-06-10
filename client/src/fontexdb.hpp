#pragma once
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <utility>
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
    uint32_t textEncode = 0; // this is part of the resource because textEncode can refer to a long-text-string

    int32_t left  :  8 = 0;
    int32_t right :  8 = 0;
    int32_t ascent: 16 = 0;

    SDL_Texture *texture = nullptr;
};

// key & 0X00FF000000000000) >> 48: font index --+
// key & 0X0000FF0000000000) >> 40: font size  --+---> combined as ttf index
// key & 0X000000FF00000000) >> 32: font style
// key & 0X00000000FFFFFFFF) >>  0: utf8 code or long text encode

// use lower 4 bytes:
// 1. used as a single unicode point returned by utf8f::str2code(), max value is: 0x0010_FFFF
// 2. used as a valid utf8-string buffer:
//
//        const std::string buf = get_valid_utf_8_string_with_lenght_less_than_or_equal_to_4();
//        assert(buf.size() <= 4);
//
//        uint32_t encode = 0;
//        std::memcpy(&encode, buf.data(), buf.size());
//
//    in this way, encode has maximal possible value: 0xBFDF_BFDF
//    it's from a valid UTF-8 string with two chars, packed with little-endian: U+07FF U+07FF
//
// 3. used as a long-string index

class FontexDB: public innDB<uint64_t, FontexElement>
{
    private:
        constexpr static uint32_t R1_MAX = 0x0010FFFFU; // single unicode point
        constexpr static uint32_t R2_MAX = 0xBFDFBFDFU; // raw utf-8 buffer with size <= 4
        constexpr static uint32_t R3_MAX = 0x400F401FU; // long string index

        constexpr static uint32_t R1_BASE = 0;
        constexpr static uint32_t R2_BASE = R1_BASE + R1_MAX + 1;
        constexpr static uint32_t R3_BASE = R2_BASE + R2_MAX + 1;

        static_assert(R3_BASE + R3_MAX == 0xFFFFFFFFU);

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
        std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> m_longText2Encode;
        std::unordered_map<uint32_t, const char *>                     m_encode2LongText;

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
        SDL_Texture *retrieve(uint64_t key, int *left = nullptr, int *right = nullptr, int *ascent = nullptr)
        {
            if(auto p = innLoad(key)){
                if(left  ) *  left = p->  left;
                if(right ) * right = p-> right;
                if(ascent) *ascent = p->ascent;
                return p->texture;
            }
            return nullptr;
        }

        SDL_Texture *retrieve(uint8_t fontIndex, uint8_t fontSize, uint8_t fontStyle, const char *utf8String, int *left = nullptr, int *right = nullptr, int *ascent = nullptr)
        {
            return retrieve(utf8f::buildU64Key(fontIndex, fontSize, fontStyle, encodeString(utf8String)), left, right, ascent);
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

        size_t fontCount() const
        {
            return m_entryList.size();
        }

        std::tuple<std::string, std::string> fontName(uint8_t font)
        {
            if(!hasFont(font)){
                throw fflpanic("invalid font index: {}", font);
            }

            if(auto ttfPtr = findTTF((to_u16(font) << 8) | UINT16_C(16))){
                const auto familyName = TTF_FontFaceFamilyName(ttfPtr);
                const auto  styleName = TTF_FontFaceStyleName (ttfPtr);

                fflassert(str_haschar(familyName));
                fflassert(str_haschar( styleName));

                return {familyName, styleName};
            }
            throw fflpanic("failed to load font: {}", font);
        }

        int fontAscent(uint8_t font)
        {
            if(!hasFont(font)){
                throw fflpanic("invalid font index: {}", font);
            }

            if(auto ttfPtr = findTTF((to_u16(font) << 8) | UINT16_C(16))){
                return TTF_FontAscent(ttfPtr);
            }
            throw fflpanic("failed to load font: {}", font);
        }

        int fontHeight(uint8_t font)
        {
            if(!hasFont(font)){
                throw fflpanic("invalid font index: {}", font);
            }

            if(auto ttfPtr = findTTF((to_u16(font) << 8) | UINT16_C(16))){
                return TTF_FontHeight(ttfPtr);
            }
            throw fflpanic("failed to load font: {}", font);
        }

        int fontLineSkip(uint8_t font)
        {
            if(!hasFont(font)){
                throw fflpanic("invalid font index: {}", font);
            }

            if(auto ttfPtr = findTTF((to_u16(font) << 8) | UINT16_C(16))){
                return TTF_FontLineSkip(ttfPtr);
            }
            throw fflpanic("failed to load font: {}", font);
        }

        bool fontMono(uint8_t font)
        {
            if(!hasFont(font)){
                throw fflpanic("invalid font index: {}", font);
            }

            if(auto ttfPtr = findTTF((to_u16(font) << 8) | UINT16_C(16))){
                return TTF_FontFaceIsFixedWidth(ttfPtr) != 0;
            }
            throw fflpanic("failed to load font: {}", font);
        }

    public:
        std::optional<std::tuple<FontexElement, size_t>> loadResource(uint64_t) override;

    public:
        void freeResource(FontexElement &) override;

    private:
        uint32_t encodeString(const char *utf8String)
        {
            fflassert(utf8String);

            if(const auto size = std::strlen(utf8String); size <= 4){
                uint32_t utf8Code = 0;
                std::memcpy(&utf8Code, utf8String, size);
                return encodeRange(2, utf8Code);
            }

            if(auto p = m_longText2Encode.find(utf8String); p != m_longText2Encode.end()){
                return p->second.first;
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

            if(currIndex >= R3_MAX){
                throw fflpanic("long text count exceeds limit: {}", currIndex);
            }

            const auto encodedIndex = encodeRange(3, currIndex);
            const auto insertedString = m_longText2Encode.try_emplace(utf8String, std::make_pair(encodedIndex, 0)); // allocate slot only, no resource refers to it now
            const auto insertedEncode = m_encode2LongText.try_emplace(encodedIndex, insertedString.first->first.c_str());

            fflassert(insertedString.second);
            fflassert(insertedEncode.second);

            return encodedIndex;
        }

    private:
        static uint32_t encodeRange(int range, uint32_t val)
        {
            switch(range){
                case 1 : fflassert(val <= R1_MAX); return R1_BASE + val;
                case 2 : fflassert(val <= R2_MAX); return R2_BASE + val;
                case 3 : fflassert(val <= R3_MAX); return R3_BASE + val;
                default:                           throw  fflvalue(range, val);
            }
        }

        static std::pair<size_t, uint32_t> decodeRange(uint32_t val)
        {
            if     (val <= R1_BASE + R1_MAX) return {1, val - R1_BASE};
            else if(val <= R2_BASE + R2_MAX) return {2, val - R2_BASE};
            else                             return {3, val - R3_BASE};
        }

    public:
        static uint32_t hasGlphy(TTF_Font *, uint32_t);

    private:
        static bool isTransparant(TTF_Font *, uint32_t);
        static bool isTransparant(const std::tuple<int, int, int, int, int> &);

    private:
        static std::tuple<int, int, int, int, int> getGlyphMetrics(TTF_Font *, uint32_t); // returns everything

    private:
        static std::tuple<int, int, int> getGlyphPadding  (const std::tuple<int, int, int, int, int> &);
        static std::pair <int, int>      getGlyphPixelSize(const std::tuple<int, int, int, int, int> &);

    private:
        static std::tuple<int, int, int> getGlyphPadding  (TTF_Font *font, uint32_t codePoint){ return getGlyphPadding  (getGlyphMetrics(font, codePoint)); }
        static std::pair <int, int>      getGlyphPixelSize(TTF_Font *font, uint32_t codePoint){ return getGlyphPixelSize(getGlyphMetrics(font, codePoint)); }
};
