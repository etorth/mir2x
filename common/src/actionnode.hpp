/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 05/05/2017 17:48:26
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

    uint32_t MapID;

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, int nEndX, int nEndY, uint32_t nMapID)
        : Action(nAction)
        , ActionParam(nActionParam)
        , Speed(nSpeed)
        , Direction(nDirection)
        , X(nX)
        , Y(nY)
        , EndX(nEndX)
        , EndY(nEndY)
        , MapID(nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nDirection, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, 0, nDirection, nX, nY, nX, nY, nMapID)
    {}

    ActionNode()
        : ActionNode(ACTION_NONE, 0, 0, 0, 0, 0, 0, 0, 0)
    {}

    void Print() const;
};
