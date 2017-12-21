/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
 *  Last Modified: 12/21/2017 01:06:57
 *
 *    Description:
 *
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

struct ActionDie
{
    int X = -1;
    int Y = -1;
    int Direction = -1;

    ActionDie(int nX, int nY, int nDirection)
        : X(nX)
        , Y(nY)
        , Direction(nDirection)
    {}
};

struct ActionStand
{
    int X = -1;
    int Y = -1;
    int Direction = -1;

    ActionStand(int nX, int nY, int nDirection)
        : X(nX)
        , Y(nY)
        , Direction(nDirection)
    {}
};

struct ActionSpell
{
    int X    = -1;
    int Y    = -1;
    int AimX = -1;
    int AimY = -1;

    uint32_t AimUID  = 0;
    uint32_t MagicID = 0;

    ActionSpell(int nX, int nY, uint32_t nAimUID, uint32_t nMagicID)
        : X(nX)
        , Y(nY)
        , AimUID(nAimUID)
        , MagicID(nMagicID)
    {}

    ActionSpell(int nX, int nY, int nAimX, int nAimY, uint32_t nMagicID)
        : X(nX)
        , Y(nY)
        , AimX(nAimX)
        , AimY(nAimY)
        , MagicID(nMagicID)
    {}
};

struct ActionMove
{
    int X     = -1;
    int Y     = -1;
    int AimX  = -1;
    int AimY  = -1;
    int Speed = -1;
    int Horse = -1;

    ActionMove(int nX, int nY, int nAimX, int nAimY, int nSpeed, int nHorse)
        : X(nX)
        , Y(nY)
        , AimX(nAimX)
        , AimY(nAimY)
        , Speed(nSpeed)
        , Horse(nHorse)
    {}
};

struct ActionPushMove
{
    int X;
    int Y;

    ActionPushMove(int nX = -1, int nY = -1)
        : X(nX)
        , Y(nY)
    {}
};

struct ActionSpaceMove1
{
    int X;
    int Y;
    int Direction;

    ActionSpaceMove1(int nX = -1, int nY = -1, int nDirection = -1)
        : X(nX)
        , Y(nY)
        , Direction(nDirection)
    {}
};

struct ActionSpaceMove2
{
    int X;
    int Y;
    int Direction;

    ActionSpaceMove2(int nX = -1, int nY = -1, int nDirection = -1)
        : X(nX)
        , Y(nY)
        , Direction(nDirection)
    {}
};

struct ActionAttack
{
    int X = -1;
    int Y = -1;

    int DC    = -1;
    int Speed = -1;

    uint32_t AimUID = 0;

    ActionAttack(int nX, int nY, int nDC, int nSpeed, uint32_t nAimUID)
        : X(nX)
        , Y(nY)
        , DC(nDC)
        , Speed(nSpeed)
        , AimUID(nAimUID)
    {}
};

struct ActionHitted
{
    int X;
    int Y;
    int Direction;

    ActionHitted(int nX = -1, int nY = -1, int nDirection = -1)
        : X(nX)
        , Y(nY)
        , Direction(nDirection)
    {}
};

struct ActionPickUp
{
    int X;
    int Y;

    uint32_t ItemID;

    ActionPickUp(int nX = -1, int nY = -1, uint32_t nItemID = 0)
        : X(nX)
        , Y(nY)
        , ItemID(nItemID)
    {}
};

struct ActionNode
{
    const int Action    =  ACTION_NONE;
    const int Speed     = -1;
    const int Direction = -1;

    const int X    = -1;
    const int Y    = -1;
    const int AimX = -1;
    const int AimY = -1;

    const uint32_t AimUID      = 0;
    const uint32_t ActionParam = 0;

    ActionNode()
        : Action(ACTION_NONE)
    {}

    ActionNode(int nAction, int nSpeed, int nDirection, int nX, int nY, int nAimX, int nAimY, uint32_t nAimUID, uint32_t nActionParam)
        : Action(nAction)
        , Speed(nSpeed)
        , Direction(nDirection)
        , X(nX)
        , Y(nY)
        , AimX(nAimX)
        , AimY(nAimY)
        , AimUID(nAimUID)
        , ActionParam(nActionParam)
    {}

    ActionNode(const ActionDie &rstDie)
        : Action(ACTION_DIE)
        , Direction(rstDie.Direction)
        , X(rstDie.X)
        , Y(rstDie.Y)
    {}

    ActionNode(const ActionStand &rstStand)
        : Action(ACTION_STAND)
        , Direction(rstStand.Direction)
        , X(rstStand.X)
        , Y(rstStand.Y)
    {}

    ActionNode(const ActionMove &rstMove)
        : Action(ACTION_MOVE)
        , Speed(rstMove.Speed)
        , X(rstMove.X)
        , Y(rstMove.Y)
        , AimX(rstMove.AimX)
        , AimY(rstMove.AimY)
        , ActionParam(rstMove.Horse)
    {}

    ActionNode(const ActionSpell &rstSpell)
        : Action(ACTION_SPELL)
        , X(rstSpell.X)
        , Y(rstSpell.Y)
        , AimX(rstSpell.AimX)
        , AimY(rstSpell.AimY)
        , AimUID(rstSpell.AimUID)
        , ActionParam(rstSpell.MagicID)
    {}

    ActionNode(const ActionPushMove &rstPushMove)
        : Action(ACTION_PUSHMOVE)
        , X(rstPushMove.X)
        , Y(rstPushMove.Y)
    {}

    ActionNode(const ActionSpaceMove1 &rstSpaceMove1)
        : Action(ACTION_SPACEMOVE1)
        , X(rstSpaceMove1.X)
        , Y(rstSpaceMove1.Y)
    {}

    ActionNode(const ActionSpaceMove2 &rstSpaceMove2)
        : Action(ACTION_SPACEMOVE2)
        , X(rstSpaceMove2.X)
        , Y(rstSpaceMove2.Y)
    {}

    ActionNode(const ActionAttack &rstAttack)
        : Action(ACTION_ATTACK)
        , Speed(rstAttack.Speed)
        , X(rstAttack.X)
        , Y(rstAttack.Y)
        , AimUID(rstAttack.AimUID)
        , ActionParam(rstAttack.DC)
    {}

    ActionNode(const ActionHitted &rstHitted)
        : Action(ACTION_HITTED)
        , Direction(rstHitted.Direction)
        , X(rstHitted.X)
        , Y(rstHitted.Y)
    {}

    ActionNode(const ActionPickUp &rstPickUp)
        : Action(ACTION_PICKUP)
        , X(rstPickUp.X)
        , Y(rstPickUp.Y)
        , ActionParam(rstPickUp.ItemID)
    {}

    operator bool () const
    {
        return Action != ACTION_NONE;
    }

    const char *ActionName() const
    {
        switch(Action){
            case ACTION_NONE       : return "ACTION_NONE";
            case ACTION_STAND      : return "ACTION_STAND";
            case ACTION_PICKUP     : return "ACTION_PICKUP";
            case ACTION_MOVE       : return "ACTION_MOVE";
            case ACTION_PUSHMOVE   : return "ACTION_PUSHMOVE";
            case ACTION_SPACEMOVE1 : return "ACTION_SPACEMOVE1";
            case ACTION_SPACEMOVE2 : return "ACTION_SPACEMOVE2";
            case ACTION_ATTACK     : return "ACTION_ATTACK";
            case ACTION_SPELL      : return "ACTION_SPELL";
            case ACTION_HITTED     : return "ACTION_HITTED";
            case ACTION_DIE        : return "ACTION_DIE";
            default                : return "ACTION_UNKNOWN";
        }
    }
};
