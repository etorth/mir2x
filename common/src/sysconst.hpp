/*
 * =====================================================================================
 *
 *       Filename: sysconst.hpp
 *        Created: 04/11/2016 22:24:56
 *  Last Modified: 03/28/2017 11:14:21
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
#include <cstdint>

// In code of mirx, the MAX_Y_COUNT_FOR_OBJ_H is 44, means we need to check 44 * 32 in
// height when drawing map because of the long object slice. Do some math the screen
// height is 600, then for object slice it's (44 * 32 - 600) / 32 = 25.25, means there
// are 26 cells of one object slice at most, then design data structure for object
// rendering method based on this information

const int SYS_MAPGRIDXP    = 48;
const int SYS_MAPGRIDYP    = 32;
const int SYS_OBJMAXW      = 48;
const int SYS_OBJMAXH      = 96;
const int SYS_MAXR         = 40;
const int SYS_MAPVISIBLEW  = 500;
const int SYS_MAPVISIBLEH  = 400;
const int SYS_MAPVISIBLECD = 100;

const char *SYS_MAPNAME(uint32_t);
