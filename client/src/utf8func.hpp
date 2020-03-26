/*
 * =====================================================================================
 *
 *       Filename: utf8func.hpp
 *        Created: 12/12/2018 07:26:25
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
#include <vector>
#include <cstdint>

namespace UTF8Func
{
    uint32_t PeekUTF8Code(const char *);

    std::vector<int> buildUTF8Off(const char *);

    constexpr uint64_t BuildU64Key(uint8_t nFont, uint8_t nFontSize, uint8_t nFontStyle, uint32_t nUTF8Code)
    {
        return ((uint64_t)(nFont) << 48) + ((uint64_t)(nFontSize) << 40) + ((uint64_t)(nFontStyle) << 32) + (uint64_t)(nUTF8Code);
    }
}
