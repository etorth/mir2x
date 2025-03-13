#pragma once
#include <SDL2/SDL.h>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "sdldevice.hpp"

// layout of an emoji on a texture
// on a single picture, from left to right
// |
// V  ->| FW |<-
// ---- +----+----+----+----+-+
//      |    |    |    |    | |
// FH   | 0  | 1  | 2  | 3  | |
//      |    |    |    |    | |
// ---- +----+----+----+----+-+
// ^    |    |    |    |    | |
// |    | 4  | 5  | 6  | 7  | |
//      |    |    |    |    | |
//      +----+----+----+----+-+
//      |    |    |    |    | |
//      +----+----+----+----+-+
//
//  FileName format:
//
//  [0 ~ 1] [2 ~ 5] [6 ~ 7] [8 ~ 9]  [10 ~ 11] [12 ~ 15] [16 ~ 19] [20 ~ 23].PNG
//
//  ESet    ESubset EFrame  FrameCnt fps       frameW    frameH    frameH1
//  1Byte   2Bytes  1Byte   1Byte    1Byte     2Bytes    2Bytes    2Bytes
//                  +-----
//                  ^
//                  |
//                  +------- always set it as zero
//                             and when retrieving we can just plus the frame index here

struct EmojiElement
{
    int frameW     = 0;
    int frameH     = 0;
    int frameH1    = 0;
    int fps        = 0;
    int frameCount = 0;

    SDL_Texture *texture = nullptr;
};

class EmojiDB: public innDB<uint32_t, EmojiElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        static uint32_t u32Key(uint8_t emojiSet, uint16_t emojiSubset)
        {
            return (to_u32(emojiSet) << 24) | (to_u32(emojiSubset) << 8);
        }

        static uint32_t u32Key(uint8_t emojiSet, uint16_t emojiSubset, uint8_t emojiIndex)
        {
            return u32Key(emojiSet, emojiSubset) | to_u32(emojiIndex);
        }

    public:
        EmojiDB()
            : innDB<uint32_t, EmojiElement>(1024)
        {}

    public:
        bool load(const char *emojiDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(emojiDBName);
            return true;
        }

    public:
        SDL_Texture *retrieve(uint32_t,                   int *, int *, int *, int *, int *, int *, int *);
        SDL_Texture *retrieve(uint8_t, uint16_t, uint8_t, int *, int *, int *, int *, int *, int *, int *);

    public:
        std::optional<std::tuple<EmojiElement, size_t>> loadResource(uint32_t) override;

    public:
        void freeResource(EmojiElement &) override;
};
