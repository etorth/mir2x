#pragma once
#include <tuple>
#include <memory>
#include <vector>
#include <cstdint>
#include <SDL3/SDL.h>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "totype.hpp"
#include "fflerror.hpp"
#include "magicrecord.hpp"

// each direction of a magic gfx sequence is one APNG, one texture per frame
//
//  FileName format, 26 hex digits:
//
//  [0 ~ 1] [2 ~ 6] [7] [8 ~ 10] [11 ~ 13] [14 ~ 16] [17 ~ 19] [20 ~ 22] [23 ~ 25].PNG
//
//  file    image   dir dx       dy        head x    head y    tail x    tail y
//  1Byte   20 bits 4b  every field below is 12-bit two's complement
//
//  key is seqID(gfxID, dir) = gfxID << 4 | dir, the first 8 hex digits of the file name
//  gfxID is MagicGfxEntry::gfxID, file << 20 | image
//
//  all frames share one offset, draw a frame at anchor + (dx, dy)
//  head and tail are relative to the anchor, head is the hit point of a flying magic
//  frame delays in the APNG are ignored, magic timing comes from MagicGfxEntry::speed

struct _MagicStoreElement
{
    int dx = 0;
    int dy = 0;

    int headX = 0;
    int headY = 0;

    int tailX = 0;
    int tailY = 0;

    std::vector<SDL_Texture *> frameList {};
};

class MagicDB final: public innDB<uint32_t, _MagicStoreElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        static uint32_t seqID(uint32_t gfxID, int dir)
        {
            fflassert(gfxID < (to_u32(1) << 28), gfxID);
            fflassert(dir >= 0 && dir < 16, dir);
            return (gfxID << 4) | to_u32(dir);
        }

    public:
        MagicDB(size_t resMax)
            : innDB<uint32_t, _MagicStoreElement>(resMax)
        {}

        ~MagicDB() override
        {
            clear();
        }

    public:
        void load(const char *magicDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(magicDBName);
        }

    public:
        // returns {nullptr, 0, 0} if no APNG or frame out of range
        std::tuple<SDL_Texture *, int, int> retrieve(uint32_t, int);

    public:
        // returns 0 if no APNG
        int frameCount(uint32_t);

        // frames to play for direction dir of the entry
        // MagicGfxEntry::frameCount if set, otherwise the APNG frame count
        int frameCount(const MagicGfxEntry *, int dir);

    public:
        // returns {0, 0} if no APNG
        std::tuple<int, int> head(uint32_t);
        std::tuple<int, int> tail(uint32_t);

    private:
        std::optional<std::tuple<_MagicStoreElement, size_t>> loadResource(uint32_t) override;

    private:
        void freeResource(_MagicStoreElement &) override;
};
