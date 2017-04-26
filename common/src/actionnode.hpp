/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 04/25/2017 01:11:59
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
#include <cassert>
#include "protocoldef.hpp"

struct ActionNode
{
    int Action;
    int ActionParam;

    int Speed;
    int Direction;

    int X;
    int Y;
    int EndX;
    int EndY;

    uint32_t ID;

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, int nEndX, int nEndY)
        : Action(nAction)
        , ActionParam(nActionParam)
        , Speed(nSpeed)
        , Direction(nDirection)
        , X(nX)
        , Y(nY)
        , EndX(nEndX)
        , EndY(nEndY)
        , ID(0)
    {
        // assert((nAction      >= 0) && (nAction      <= (int)(std::numeric_limits<decltype(Action     )>::max())));
        // assert((nActionParam >= 0) && (nActionParam <= (int)(std::numeric_limits<decltype(ActionParam)>::max())));
        // assert((nSpeed       >= 0) && (nSpeed       <= (int)(std::numeric_limits<decltype(Speed      )>::max())));
        // assert((nDirection   >= 0) && (nDirection   <= (int)(std::numeric_limits<decltype(Direction  )>::max())));
        // assert((nX           >= 0) && (nX           <= (int)(std::numeric_limits<decltype(X          )>::max())));
        // assert((nY           >= 0) && (nY           <= (int)(std::numeric_limits<decltype(Y          )>::max())));
        // assert((nEndX        >= 0) && (nEndX        <= (int)(std::numeric_limits<decltype(EndX       )>::max())));
        // assert((nEndY        >= 0) && (nEndY        <= (int)(std::numeric_limits<decltype(EndY       )>::max())));
    }

    ActionNode(int nAction, int nActionParam, int nDirection, int nX, int nY)
        : ActionNode(nAction, nActionParam, 0, nDirection, nX, nY, nX, nY)
    {}

    ActionNode()
        : ActionNode(ACTION_NONE, 0, 0, 0, 0, 0, 0, 0)
    {}

    void Print() const;
};
