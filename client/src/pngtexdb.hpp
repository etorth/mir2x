#pragma once
#include <vector>
#include <memory>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "sdldevice.hpp"

struct PNGTexElement
{
    SDL_Texture *texture = nullptr;
};

class PNGTexDB: public innDB<uint32_t, PNGTexElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        PNGTexDB(size_t resMax)
            : innDB<uint32_t, PNGTexElement>(resMax)
        {}

    public:
        bool load(const char *texDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(texDBName);
            return true;
        }

    public:
        SDL_Texture *retrieve(uint32_t key)
        {
            if(auto p = innLoad(key)){
                return p->texture;
            }
            return nullptr;
        }

        SDL_Texture *retrieve(uint8_t fileIndex, uint16_t imageIndex)
        {
            return retrieve(to_u32((to_u32(fileIndex) << 16) + imageIndex));
        }

    public:
        std::optional<std::tuple<PNGTexElement, size_t>> loadResource(uint32_t) override;

    public:
        void freeResource(PNGTexElement &) override;
};
