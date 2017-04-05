/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *  Last Modified: 04/05/2017 14:07:34
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
struct MotionNode
{
    int Motion;
    int Direction;
    int Speed;

    int X;
    int Y;

    int NextX;
    int NextY;

    MotionNode(int nMotion, int nDirection, int nSpeed, int nX, int nY, int nNextX, int nNextY)
        : Motion(nMotion)
        , Direction(nDirection)
        , Speed(nSpeed)
        , X(nX)
        , Y(nY)
        , NextX(nNextX)
        , NextY(nNextY)
    {}

    MotionNode(int nMotion, int nDirection, int nX, int nY)
        : Motion(nMotion)
        , Direction(nDirection)
        , Speed(0)
        , X(nX)
        , Y(nY)
        , NextX(nX)
        , NextY(nY)
    {}

    MotionNode()
        : Motion(MOTION_NONE)
    {}

    void Print();
};
