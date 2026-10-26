#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "sdldevice.hpp"

// each emoji is one APNG, one texture per frame, every frame has its own delay
//
//  FileName format, 11 hex digits:
//
//  [0 ~ 1] [2 ~ 3] [4 ~ 7]  [8 ~ 10].PNG
//
//  00      ESet    ESubset  frameH1
//          1Byte   2Bytes   12-bit two's complement, height above the text baseline
//
//  frame count, frame size and frame delays are read from the APNG
//  key is u32Key() = ESet << 16 | ESubset, same as the <emoji id="..."> in XML layout

struct EmojiElement
{
    int h1 = 0;
    size_t delay = 0;
    size_t frameCount = 0;
    SDL_Texture *texture = nullptr;
};

struct _EmojiStoreElement
{
    int h1 = 0;
    std::vector<std::pair<SDL_Texture *, int>> animation {};
};

class EmojiDB final: public innDB<uint32_t, _EmojiStoreElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    private:
        _EmojiStoreElement m_fallback {};

    public:
        static uint32_t u32Key(uint8_t emojiSet, uint16_t emojiSubset)
        {
            return (to_u32(emojiSet) << 16) | to_u32(emojiSubset);
        }

    public:
        EmojiDB()
            : innDB<uint32_t, _EmojiStoreElement>(1024)
        {}

        ~EmojiDB() override
        {
            clear();
            freeResource(m_fallback);
        }

    public:
        void load(const char *emojiDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(emojiDBName);
        }

    public:
        EmojiElement retrieveElement(uint32_t, size_t);

    private:
        std::optional<std::tuple<_EmojiStoreElement, size_t>> loadResource(uint32_t) override;

    private:
        void freeResource(_EmojiStoreElement &) override;

    private:
        _EmojiStoreElement *createFallback();
};
