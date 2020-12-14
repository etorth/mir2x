/*
 * =====================================================================================
 *
 *       Filename: compress.hpp
 *        Created: 04/23/2017 21:33:02
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
#include <cstdint>
#include <cstddef>

namespace Compress
{
    int countMask(const uint8_t *, size_t);
    int countData(const uint8_t *, size_t);

    int xorEncode(uint8_t *, const uint8_t *, size_t);
    int xorDecode(uint8_t *, size_t, const uint8_t *, const uint8_t *);
}
