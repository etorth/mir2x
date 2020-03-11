/*
 * =====================================================================================
 *
 *       Filename: autoalpha.hpp
 *        Created: 02/12/2019 06:24:06
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

void CalcPixelAutoAlpha(uint32_t *, size_t);
void CalcShadowRemovalAlpha(uint32_t *, size_t, size_t, uint32_t);
