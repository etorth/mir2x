/*
 * =====================================================================================
 *
 *       Filename: fontexdbn.hpp
 *        Created: 03/17/2016 01:17:51
 *  Last Modified: 07/04/2017 14:18:48
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
#include "fontexdb.hpp"

#define FONTEXDBN_LC_DEPTH  (2              )
#define FONTEXDBN_LC_LENGTH (1 * 2048       )
#define FONTEXDBN_CAPACITY  (2 * 2048 + 1024)

using FontexDBType = FontexDB<FONTEXDBN_LC_DEPTH,FONTEXDBN_LC_LENGTH, FONTEXDBN_CAPACITY>;

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
        SDL_Texture *Retrieve(uint64_t nKey)
        {
            const auto fnLinearCacheKey = [](uint64_t nKey)
            {
                return (nKey & 0X0000FFFF) % FONTEXDBN_LC_LENGTH;
            };

            FontexItem stItem;
            RetrieveItem(nKey, &stItem, fnLinearCacheKey);
            return stItem.Texture;
        }

        SDL_Texture *Retrieve(uint8_t nFontIndex, uint8_t nFontSize, uint8_t nFontStyle, uint32_t nUTF8Code)
        {
            uint64_t nKey = 0
                + (((uint64_t)nFontIndex) << 48)
                + (((uint64_t)nFontSize ) << 40)
                + (((uint64_t)nFontStyle) << 32) 
                + (((uint64_t)nUTF8Code)  <<  0);
            return Retrieve(nKey);
        }
};
