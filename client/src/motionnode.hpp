/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *  Last Modified: 05/30/2017 22:59:14
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
    int MotionParam;

    int Direction;
    int Speed;

    int X;
    int Y;

    int EndX;
    int EndY;

    int Frame;

    MotionNode(int nMotion, int nMotionParam, int nDirection, int nSpeed, int nX, int nY, int nEndX, int nEndY)
        : Motion(nMotion)
        , MotionParam(nMotionParam)
        , Direction(nDirection)
        , Speed(nSpeed)
        , X(nX)
        , Y(nY)
        , EndX(nEndX)
        , EndY(nEndY)
        , Frame(0)
    {}

    MotionNode(int nMotion, int nMotionParam, int nDirection, int nX, int nY)
        : MotionNode(nMotion, nMotionParam, nDirection, 0, nX, nY, nX, nY)
    {}

    MotionNode()
        : MotionNode(MOTION_NONE, 0, DIR_NONE, 0, 0)
    {}

    void Print();
};
