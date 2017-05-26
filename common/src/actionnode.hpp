/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 05/25/2017 16:38:45
 *
 *    Description: ActionNode is used by both server and client
 *                 then don't define Print() for it
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
#include "protocoldef.hpp"

struct ActionNode
{
    const int Action;
    const int ActionParam;

    const int Speed;
    const int Direction;

    const int X;
    const int Y;
    const int EndX;
    const int EndY;

    const uint32_t MapID;

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
};
