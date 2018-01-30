/*
 * =====================================================================================
 *
 *       Filename: pngtexdbn.hpp
 *        Created: 03/17/2016 01:17:51
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
#include "pngtexdb.hpp"

#define PNGTEXDBN_LC_DEPTH  (2              )
#define PNGTEXDBN_LC_LENGTH (1 * 2048       )
#define PNGTEXDBN_CAPACITY  (2 * 2048 + 1024)

using PNGTexDBType = PNGTexDB<PNGTEXDBN_LC_DEPTH, PNGTEXDBN_LC_LENGTH, PNGTEXDBN_CAPACITY>;

class PNGTexDBN: public PNGTexDBType
{
    public:
        PNGTexDBN()
            : PNGTexDBType()
        {}

    public:
        virtual ~PNGTexDBN() = default;

    public:
        SDL_Texture *Retrieve(uint32_t nKey)
        {
            const auto &fnLinearCacheKey = [](uint32_t nKey) -> size_t
            {
                return (nKey & 0X0000FFFF) % PNGTEXDBN_LC_LENGTH;
            };

            return RetrieveItem(nKey, fnLinearCacheKey).Texture;
        }

        SDL_Texture *Retrieve(uint8_t nIndex, uint16_t nImage)
        {
            return Retrieve((uint32_t)(((uint32_t)(nIndex) << 16) + nImage));
        }
};
