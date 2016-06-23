/*
 * =====================================================================================
 *
 *       Filename: animationdraw.hpp
 *        Created: 06/22/2016 22:24:59
 *  Last Modified: 06/23/2016 00:56:52
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

typedef struct _AnimationDraw{
    uint32_t MonsterID;
    int      X;
    int      Y;

    _AnimationDraw(uint32_t nMonsterID = 0, int nX = 0, int nY = 0)
        : MonsterID(nMonsterID)
        , X(nX)
        , Y(nY)
    {}
}AnimationDraw;
