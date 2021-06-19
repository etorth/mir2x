/*
 * =====================================================================================
 *
 *       Filename: pngtexoffdb.hpp
 *        Created: 02/26/2016 21:48:43
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
#include <tuple>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>
#include <SDL2/SDL.h>
#include <unordered_map>

#include "zsdb.hpp"
#include "inndb.hpp"
#include "hexstr.hpp"
#include "totype.hpp"

struct PNGTexOffElement
{
    int dx = 0;
    int dy = 0;
    SDL_Texture *texture = nullptr;
};

class PNGTexOffDB: public innDB<uint32_t, PNGTexOffElement>
{
    private:
        std::unique_ptr<ZSDB> m_zsdbPtr;

    public:
        PNGTexOffDB(size_t resMax)
            : innDB<uint32_t, PNGTexOffElement>(resMax)
        {}

    public:
        bool load(const char *texOffDBName)
        {
            m_zsdbPtr = std::make_unique<ZSDB>(texOffDBName);
            return true;
        }

    public:
        std::tuple<SDL_Texture *, int, int> retrieve(uint32_t key)
        {
            int dx = 0;
            int dy = 0;
            auto texPtr = retrieve(key, &dx, &dy);
            return {texPtr, dx, dy};
        }

        SDL_Texture *retrieve(uint32_t key, int *pdx, int *pdy)
        {
            if(auto p = innLoad(key)){
                if(pdx){
                    *pdx = p->dx;
                }

                if(pdy){
                    *pdy = p->dy;
                }
                return p->texture;
            }
            return nullptr;
        }

        SDL_Texture *retrieve(uint8_t fileIndex, uint16_t imageIndex, int *pdx, int *pdy)
        {
            return retrieve(to_u32((to_u32(fileIndex) << 16) + imageIndex), pdx, pdy);
        }

    public:
        std::optional<std::tuple<PNGTexOffElement, size_t>> loadResource(uint32_t) override;

    public:
        void freeResource(PNGTexOffElement &) override;
};
