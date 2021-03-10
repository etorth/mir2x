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
#include <type_traits>

// In code of mirx, the MAX_Y_COUNT_FOR_OBJ_H is 44, means we need to check 44 * 32 in
// height when drawing map because of the long object slice. Do some math the screen
// height is 600, then for object slice it's (44 * 32 - 600) / 32 = 25.25, means there
// are 26 cells of one object slice at most, then design data structure for object
// rendering method based on this information

constexpr double SYS_PI = 3.14159265359;
constexpr int SYS_DEFFPS = 10;

constexpr int SYS_TARGETRGN_GAPX = 10;
constexpr int SYS_TARGETRGN_GAPY = 8;

constexpr int SYS_MAPGRIDXP = 48;
constexpr int SYS_MAPGRIDYP = 32;

constexpr int SYS_OBJMAXW = 3;
constexpr int SYS_OBJMAXH = 25;

constexpr int SYS_MAXR         = 40;
constexpr int SYS_MAPVISIBLEW  = 60;
constexpr int SYS_MAPVISIBLEH  = 40;
constexpr int SYS_MAPVISIBLECD = 100;

constexpr int SYS_MAXPLAYERNUM = 8192;

constexpr int SYS_MAXDROPITEM     = 10;
constexpr int SYS_MAXDROPITEMGRID = 81;

constexpr int SYS_MINSPEED =  20;
constexpr int SYS_DEFSPEED = 100;
constexpr int SYS_MAXSPEED = 500;

constexpr int SYS_INVGRIDCW =  6;
constexpr int SYS_INVGRIDCH =  8;
constexpr int SYS_INVGRIDPW = 38;
constexpr int SYS_INVGRIDPH = 38;
constexpr int SYS_INVGRIDMAXHOLD = 99;

constexpr int SYS_MAXTARGET = 8;
constexpr int SYS_MAXACTOR  = 65521;

constexpr uint32_t SYS_TEXNIL = 0XFFFFFFFF;
constexpr int SYS_MAXNPCDISTANCE = 10;

constexpr char SYS_NPCINIT [] = "RSVD_NPC_INIT__2967391362393263";
constexpr char SYS_NPCDONE [] = "RSVD_NPC_DONE__6381083734343264";
constexpr char SYS_NPCQUERY[] = "RSVD_NPC_QUERY_8619263917692639";
constexpr char SYS_NPCERROR[] = "RSVD_NPC_ERROR_8619263917692639";

constexpr size_t SYS_EXP[]
{
       100,  //  0: need 100 to reach next level from current level 0
       165,  //  1: need 165 to reach next level from current level 1
       333,  //  2: ...
       604,  //  3
       977,  //  4
      1454,  //  5
      2036,  //  6
      2722,  //  7
      3514,  //  8
      4412,  //  9
      5417,  // 10
      6531,  // 11
      7755,  // 12
      9091,  // 13
     10539,  // 14
     12102,  // 15
     13783,  // 16
     15584,  // 17
     17508,  // 18
     19559,  // 19
     21741,  // 20
     24059,  // 21
     26517,  // 22
     29123,  // 23
     31883,  // 24
     34807,  // 25
     37903,  // 26
     41183,  // 27
     44660,  // 28
     48348,  // 29
     52265,  // 30
     56431,  // 31
     60868,  // 32
     65603,  // 33
     70666,  // 34
     76091,  // 35
     81920,  // 36
     88200,  // 37
     94982,  // 38
    102331,  // 39
    110317,  // 40
    119022,  // 41
    128543,  // 42
    138988,  // 43
    150485,  // 44
    163179,  // 45
    177239,  // 46
    192859,  // 47
    210265,  // 48
    229713,  // 49
    251504,  // 50
};

constexpr inline size_t SYS_SUMEXP(uint32_t level)
{
    size_t sumExp = 0;
    for(size_t i = 0; i < level && i < std::extent_v<decltype(SYS_EXP)>; ++i){
        sumExp += SYS_EXP[i];
    }
    return sumExp;
}

constexpr inline uint32_t SYS_LEVEL(size_t exp)
{
    for(uint32_t level = 0; level + 1 < std::extent_v<decltype(SYS_EXP)>; ++level){
        if(exp >= SYS_SUMEXP(level) && exp < SYS_SUMEXP(level + 1)){
            return level;
        }
    }
    return std::extent_v<decltype(SYS_EXP)>;
}
