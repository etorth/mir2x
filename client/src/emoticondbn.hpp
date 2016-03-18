/*
 * =====================================================================================
 *
 *       Filename: emoticondbn.hpp
 *        Created: 03/17/2016 01:17:51
 *  Last Modified: 03/18/2016 15:55:31
 *
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

#define FONTEXDBN_LC_DEPTH  (2              )
#define FONTEXDBN_LC_LENGTH (2048           )
#define FONTEXDBN_CAPACITY  (2 * 2048 + 1024)

using FontexDBType = FontexDB<FONTEXDBN_LC_DEPTH, FONTEXDBN_LC_LENGTH, FONTEXDBN_CAPACITY>;

class FontexDBN: public FontexDBType
{
    public:
        FontexDBN(): FontexDBType()
        {
            extern FontexDBN *g_FontexDBN;
            if(g_FontexDBN){
                throw std::runtime_error("one instance for FontexDBN please");
            }
        }

        virtual ~FontexDBN() = default;

    public:
        SDL_Texture *Retrieve(uint32_t nKey)
        {
            const auto &fnLinearCacheKey = [&](uint32_t nKey)->size_t
            {
                return (nKey & 0X0000FFFF) % FONTEXDBN_LC_LENGTH;
            };

            return RetrieveItem(nKey, fnLinearCacheKey);
        }

        SDL_Texture *Retrieve(uint8_t nIndex, uint16_t nImage)
        {
            return Retrieve((uint32_t)(((uint32_t)(nIndex) << 16) + nImage));
        }
};
