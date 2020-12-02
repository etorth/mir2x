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
#include <stdexcept>
#include "strf.hpp"
#include "fflerror.hpp"
#include "totype.hpp"
#include "alphaf.hpp"
#include "autoalpha.hpp"

void CalcPixelAutoAlpha(uint32_t *pData, size_t nDataLen)
{
    alphaf::autoAlpha(pData, nDataLen);
}

void CalcShadowRemovalAlpha(uint32_t *pData, size_t nWidth, size_t nHeight, uint32_t nShadowColor)
{
    alphaf::autoShadowRemove(pData, nWidth, nHeight, nShadowColor);
}
