/*
 * =====================================================================================
 *
 *       Filename: motionnode.hpp
 *        Created: 04/05/2017 12:38:46
 *  Last Modified: 07/08/2017 19:35:30
 *
 *    Description: for field MotionNode::Speed
 *                  
 *                      means % Speed of default speed
 *
 *                 i.e. if default speed is 100 FPS:
 *                  
 *                      MotionNode::Speed :  20 : FPS =  20 : min
 *                                           50 : FPS =  50 : slow
 *                                          100 : FPS = 100 : default
 *                                          200 : FPS = 200 : fast
 *                                          500 : FPS = 500 : max
 *                     
 *                  currently support speed : 20 ~ 500 => speed x5 or d5 
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
#include "sysconst.hpp"
#include "protocoldef.hpp"

struct MotionNode
{
    // part-1 : const fields
    //          description of this motion
    int Motion;
    int MotionParam;

    int Direction;
    int Speed;

    int X;
    int Y;

    int EndX;
    int EndY;

    // part-2 : mutable field
    //          always initialized as 0 and get updated later
    int Frame;
    int FadeOut;

    // don't put any simple data memeber check in constructor
    // in main code there is need to return an invalid motion node

    // if put check method in constructor
    // I need to abort if checking failed this is not good
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
        , FadeOut(0)
    {}

    MotionNode(int nMotion, int nMotionParam, int nDirection, int nSpeed, int nX, int nY)
        : MotionNode(nMotion, nMotionParam, nDirection, nSpeed, nX, nY, nX, nY)
    {}

    MotionNode(int nMotion, int nMotionParam, int nDirection, int nX, int nY)
        : MotionNode(nMotion, nMotionParam, nDirection, SYS_DEFSPEED, nX, nY, nX, nY)
    {}

    MotionNode()
        : MotionNode(MOTION_NONE, 0, DIR_NONE, 0, 0)
    {}

    operator bool () const
    {
        return false
            || ((Motion > MOTION_NONE)     && (Motion < MOTION_MAX))
            || ((Motion > MOTION_MON_NONE) && (Motion < MOTION_MON_MAX));
    }

    void Print();
};
