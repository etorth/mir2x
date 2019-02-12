/*
 * =====================================================================================
 *
 *       Filename: autoalpha.cpp
 *        Created: 02/12/2019 06:24:47
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

#include <cmath>
#include <algorithm>
#include "autoalpha.hpp"

void CalcPixelAutoAlpha(uint32_t *pData, size_t nDataLen)
{
    if(!(pData && nDataLen)){
        return;
    }

    for(size_t nIndex = 0; nIndex < nDataLen; ++nIndex){
        uint8_t a = ((pData[nIndex] & 0XFF000000) >> 24);

        if(a == 0){
            continue;
        }

        uint8_t r = ((pData[nIndex] & 0X00FF0000) >> 16);
        uint8_t g = ((pData[nIndex] & 0X0000FF00) >>  8);
        uint8_t b = ((pData[nIndex] & 0X000000FF) >>  0);

        a = std::max<uint8_t>({r, g, b});

        r = std::lround(1.0 * r * 255.0 / a);
        g = std::lround(1.0 * g * 255.0 / a);
        b = std::lround(1.0 * b * 255.0 / a);

        pData[nIndex] = ((uint32_t)(a) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b);
    }
}
