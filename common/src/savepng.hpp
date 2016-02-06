/*
 * =====================================================================================
 *
 *       Filename: savepng.hpp
 *        Created: 02/06/2016 04:25:06
 *  Last Modified: 02/06/2016 04:28:18
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
#include <png.h>
#include <cstdio>
#include <cstdint>
#include <cstring>

bool SaveRGBABufferToPNG(const uint8_t *, uint32_t, uint32_t, const char *);
