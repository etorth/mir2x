#pragma once
#include <tuple>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>
#include <SDL3/SDL.h>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "totype.hpp"

struct PNGTexOffElement
{
    int dx = 0;
    int dy = 0;
    std::vector<SDL_Texture *> frameList {};
};

class PNGTexOffDB final: public innDB<uint32_t, PNGTexOffElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        PNGTexOffDB(size_t resMax)
            : innDB<uint32_t, PNGTexOffElement>(resMax)
        {}

    public:
        ~PNGTexOffDB() override
        {
            clear();
        }

    public:
        void load(const char *texOffDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(texOffDBName);
        }

    public:
        // returns nullptr if frame is out of range
        // static images, like equip, are APNG with 1 frame
        std::tuple<SDL_Texture *, int, int> retrieve(uint32_t, int = 0);

    public:
        // human sequence key: hero, hair, helmet, weapon, selectchar
        //
        //   gfxID: id << 9 | motion << 3 | direction
        //   key  : direction 0-2, motion 3-8, layer 9, shadow 10, gender 11, id 12-19
        //
        static uint32_t humanKey(uint32_t gfxID, bool gender, bool shadow = false, int layer = 0)
        {
            return 0
                | (((gfxID >> 0) & 0X01FF) <<  0)
                | ((to_u32(layer) & 0X0001) <<  9)
                | ((to_u32(shadow)        ) << 10)
                | ((to_u32(gender)        ) << 11)
                | (((gfxID >> 9) & 0X00FF) << 12);
        }

        // creature sequence key: monster, npc
        //
        //   key: direction 0-2, motion 3-6, layer 7 (always 0), shadow 8, look 9-19
        //
        static uint32_t creatureKey(int look, int motion, int direction, bool shadow = false)
        {
            return 0
                | ((to_u32(direction) & 0X0007) << 0)
                | ((to_u32(motion   ) & 0X000F) << 3)
                | ( to_u32(shadow   )           << 8)
                | ((to_u32(look     ) & 0X07FF) << 9);
        }

    public:
        std::optional<std::tuple<PNGTexOffElement, size_t>> loadResource(uint32_t) override;

    public:
        void freeResource(PNGTexOffElement &) override;
};
