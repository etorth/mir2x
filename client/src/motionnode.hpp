/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *  Last Modified: 04/26/2017 16:05:43
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
#include "motion.hpp"
#include "protocoldef.hpp"
struct MotionNode
{
    int Motion;
    int Direction;
    int Speed;

    int X;
    int Y;

    int EndX;
    int EndY;

    int Frame;

    uint32_t ID;

    MotionNode(int nMotion, int nDirection, int nSpeed, int nX, int nY, int nEndX, int nEndY)
        : Motion(nMotion)
        , Direction(nDirection)
        , Speed(nSpeed)
        , X(nX)
        , Y(nY)
        , EndX(nEndX)
        , EndY(nEndY)
        , Frame(0)
        , ID(0)
    {}

    MotionNode(int nMotion, int nDirection, int nX, int nY)
        : MotionNode(nMotion, nDirection, 0, nX, nY, nX, nY)
    {}

    MotionNode()
        : MotionNode(MOTION_NONE, DIR_NONE, 0, 0)
    {}

    void Print();
};
