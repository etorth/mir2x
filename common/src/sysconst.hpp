/*
 * =====================================================================================
 *
 *       Filename: sysconst.hpp
 *        Created: 04/11/2016 22:24:56
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

const int SYS_MAXTARGET = 8;

const uint32_t SYS_NEEDEXP[]
{
             0 , // 0
           100 , // 1
           200 , // 2
           300 , // 3
           400 , // 4
           600 , // 5
           900 , // 6
          1200 , // 7
          1700 , // 8
          2500 , // 9
          6000 , //10
          8000 , //11
         10000 , //12
         15000 , //13
         30000 , //14
         40000 , //15
         50000 , //16
         70000 , //17
        100000 , //18
        120000 , //19
        140000 , //20
        250000 , //21
        300000 , //22
        350000 , //23
        400000 , //24
        500000 , //25
        700000 , //26
       1000000 , //27
       1400000 , //28
       1800000 , //29
       2000000 , //30
       2400000 , //31
       2800000 , //32
       3200000 , //33
       3600000 , //34
       4000000 , //35
       4800000 , //36
       5600000 , //37
       8200000 , //38
       9000000 , //39
      12000000 , //40
      16000000 , //41
      30000000 , //42
      50000000 , //43
      80000000 , //44
     120000000 , //45
     480000000 , //46
    1000000000 , //47
    3000000000 , //48
    3500000000 , //49
    4000000000 , //50
};
