/*
 * =====================================================================================
 *
 *       Filename: sysconst.hpp
 *        Created: 04/11/2016 22:24:56
 *  Last Modified: 12/24/2017 22:15:54
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
#include <vector>
#include <cstdint>

// In code of mirx, the MAX_Y_COUNT_FOR_OBJ_H is 44, means we need to check 44 * 32 in
// height when drawing map because of the long object slice. Do some math the screen
// height is 600, then for object slice it's (44 * 32 - 600) / 32 = 25.25, means there
// are 26 cells of one object slice at most, then design data structure for object
// rendering method based on this information

const int SYS_DEFFPS = 10;

const int SYS_TARGETRGN_GAPX = 10;
const int SYS_TARGETRGN_GAPY = 8;

const int SYS_MAPGRIDXP = 48;
const int SYS_MAPGRIDYP = 32;

const int SYS_OBJMAXW = 3;
const int SYS_OBJMAXH = 15;

const int SYS_MAXR         = 40;
const int SYS_MAPVISIBLEW  = 60;
const int SYS_MAPVISIBLEH  = 40;
const int SYS_MAPVISIBLECD = 100;

const int SYS_MAXPLAYERNUM = 8192;

const int SYS_MAXDROPITEM     = 10;
const int SYS_MAXDROPITEMGRID = 81;

const int SYS_MINSPEED =  20;
const int SYS_DEFSPEED = 100;
const int SYS_MAXSPEED = 500;

const int SYS_INVGRIDW  = 6;
const int SYS_INVGRIDPW = 38;
const int SYS_INVGRIDPH = 38;
