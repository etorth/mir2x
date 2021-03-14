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
       1000,  //  0: need 100 to reach next level from current level 0
       1650,  //  1: need 165 to reach next level from current level 1
       3330,  //  2: ...
       6040,  //  3
       9770,  //  4
      14540,  //  5
      20360,  //  6
      27220,  //  7
      35140,  //  8
      44120,  //  9
      54170,  // 10
      65310,  // 11
      77550,  // 12
      90910,  // 13
     105390,  // 14
     121020,  // 15
     137830,  // 16
     155840,  // 17
     175080,  // 18
     195590,  // 19
     217410,  // 20
     240590,  // 21
     265170,  // 22
     291230,  // 23
     318830,  // 24
     348070,  // 25
     379030,  // 26
     411830,  // 27
     446600,  // 28
     483480,  // 29
     522650,  // 30
     564310,  // 31
     608680,  // 32
     656030,  // 33
     706660,  // 34
     760910,  // 35
     819200,  // 36
     882000,  // 37
     949820,  // 38
    1023310,  // 39
    1103170,  // 40
    1190220,  // 41
    1285430,  // 42
    1389880,  // 43
    1504850,  // 44
    1631790,  // 45
    1772390,  // 46
    1928590,  // 47
    2102650,  // 48
    2297130,  // 49
    2515040,  // 50
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
