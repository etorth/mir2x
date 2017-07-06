/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 07/06/2017 10:59:27
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
#include "sysconst.hpp"
#include "protocoldef.hpp"

struct ActionNode
{
    const int Action;
    const int ActionParam;

    const int Speed;
    const int Direction;

    const int X;
    const int Y;
    const int AimX;
    const int AimY;

    const uint32_t MapID;

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, int nAimX, int nAimY, uint32_t nMapID)
        : Action(nAction)
        , ActionParam(nActionParam)
        , Speed(nSpeed)
        , Direction(nDirection)
        , X(nX)
        , Y(nY)
        , AimX(nAimX)
        , AimY(nAimY)
        , MapID(nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, nSpeed, nDirection, nX, nY, nX, nY, nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nDirection, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, SYS_DEFSPEED, nDirection, nX, nY, nX, nY, nMapID)
    {}

    ActionNode()
        : ActionNode(ACTION_NONE, 0, 0, 0, 0, 0, 0, 0, 0)
    {}

    operator bool () const
    {
        return Action != ACTION_NONE;
    }
};
