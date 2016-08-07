/*
 * =====================================================================================
 *
 *       Filename: animationdraw.hpp
 *        Created: 06/22/2016 22:24:59
 *  Last Modified: 08/07/2016 11:14:07
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
    uint32_t Action;
    uint32_t Direction;
    uint32_t Frame;

    int      X;
    int      Y;
    int      R;

    _AnimationDraw(uint32_t nMonsterID = 0, int nX = 0, int nY = 0, int nR = 0)
        : MonsterID(nMonsterID)
        , Action(0)
        , Direction(0)
        , Frame(0)
        , X(nX)
        , Y(nY)
        , R(nR)
    {}
}AnimationDraw;
