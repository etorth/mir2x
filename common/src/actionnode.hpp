/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 11/29/2017 14:59:47
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

struct ActionPickUp
{
    int AimX;
    int AimY;

    uint32_t ItemID;
    uint32_t MapID;

    ActionPickUp(int nAimX = -1, int nAimY = -1, uint32_t nItemID = 0, uint32_t nMapID = 0)
        : AimX(nAimX)
        , AimY(nAimY)
        , ItemID(nItemID)
        , MapID(nMapID)
    {}
};

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

    const uint32_t AimUID;
    const uint32_t MapID;

    // don't change it to avoid override error
    // should be very careful for the following cotrs and it's short format

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, int nAimX, int nAimY, uint32_t nAimUID, uint32_t nMapID)
        : Action(nAction)
        , ActionParam(nActionParam)
        , Speed(nSpeed)
        , Direction(nDirection)
        , X(nX)
        , Y(nY)
        , AimX(nAimX)
        , AimY(nAimY)
        , AimUID(nAimUID)
        , MapID(nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, int nAimX, int nAimY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, nSpeed, nDirection, nX, nY, nAimX, nAimY, 0, nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, int nAimUID, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, nSpeed, nDirection, nX, nY, -1, -1, nAimUID, nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nSpeed, int nDirection, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, nSpeed, nDirection, nX, nY, nX, nY, 0, nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nDirection, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, SYS_DEFSPEED, nDirection, nX, nY, nX, nY, 0, nMapID)
    {}

    ActionNode(int nAction, int nActionParam, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, nActionParam, SYS_DEFSPEED, DIR_NONE, nX, nY, nX, nY, 0, nMapID)
    {}

    ActionNode(int nAction, int nX, int nY, uint32_t nMapID)
        : ActionNode(nAction, 0, SYS_DEFSPEED, DIR_NONE, nX, nY, nX, nY, 0, nMapID)
    {}

    ActionNode()
        : ActionNode(ACTION_NONE, 0, 0, 0, 0, 0, 0, 0, 0, 0)
    {}

    operator bool () const
    {
        return Action != ACTION_NONE;
    }
};
