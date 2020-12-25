/*
 * =====================================================================================
 *
 *       Filename: actionnode.hpp
 *        Created: 04/06/2017 13:03:56
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
#include <cstring>
#include "sysconst.hpp"
#include "protocoldef.hpp"

enum ActionType: int
{
    ACTION_NONE  = 0,
    ACTION_BEGIN = 1,
    ACTION_SPAWN = 1,
    ACTION_STAND,
    ACTION_PICKUP,
    ACTION_MOVE,
    ACTION_PUSHMOVE,
    ACTION_SPACEMOVE1,
    ACTION_SPACEMOVE2,
    ACTION_ATTACK,
    ACTION_SPELL,
    ACTION_TRANSF,
    ACTION_HITTED,
    ACTION_DIE,
    ACTION_END,
};

#pragma pack(push, 1)
struct ActionNode
{
    uint16_t type      : 5;
    uint16_t speed     : 9;
    uint16_t direction : 5;

    uint16_t x;
    uint16_t y;
    uint16_t aimX;
    uint16_t aimY;

    uint64_t aimUID;
    union ActionExtParam
    {
        struct ExtParamNPCStand
        {
            uint8_t act;
        }npc;

        struct ExtParamDie
        {
            uint8_t fadeOut;
        }die;

        struct ExtParamDogStand
        {
            uint8_t standMode;
        }dogStand;

        struct ExtParamSpell
        {
            uint32_t magicID;
        }spell;

        struct ExtParamDogTransf
        {
            uint8_t standMode;
        }transf;

        struct ExtParamMove
        {
            uint8_t onHorse;
        }move;

        struct ExtParamAttack
        {
            uint32_t damageID;
        }attack;

        struct ExtParamPickUp
        {
            uint32_t itemID;
        }pickUp;
    } extParam;
};
#pragma pack(pop)

struct _ActionDie
{
    const int x = -1;
    const int y = -1;
    const bool fadeOut = true;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_DIE;

        node.x = x;
        node.y = y;

        node.extParam.die.fadeOut = fadeOut;
        return node;
    }
};

struct _ActionStand
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_STAND;
        node.direction = direction;

        node.x = x;
        node.y = y;

        return node;
    }
};

struct _ActionTransf
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;
    const bool standMode = false;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_TRANSF;
        node.direction = direction;

        node.x = x;
        node.y = y;

        node.extParam.transf.standMode = standMode;
        return node;
    }
};

struct _ActionSpawn
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_SPAWN;
        node.direction = direction;

        node.x = x;
        node.y = y;

        return node;
    }
};

struct _ActionSpell
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const int aimX = x;
    const int aimY = y;

    const uint64_t aimUID  = 0;
    const uint32_t magicID = 0;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_SPELL;
        node.speed  = speed;

        node.x = x;
        node.y = y;

        node.aimX = aimX;
        node.aimY = aimY;

        node.aimUID = aimUID;
        node.extParam.spell.magicID = magicID;
        return node;
    }
};

struct _ActionMove
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const int aimX = -1;
    const int aimY = -1;

    const bool onHorse = false;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_MOVE;
        node.speed  = speed;

        node.x = x;
        node.y = y;

        node.aimX = aimX;
        node.aimY = aimY;

        node.extParam.move.onHorse = onHorse;
        return node;
    }
};

struct _ActionSpaceMove1
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_SPACEMOVE1;
        node.direction = direction;

        node.x = x;
        node.y = y;

        return node;
    }
};

struct _ActionSpaceMove2
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_SPACEMOVE2;
        node.direction = direction;

        node.x = x;
        node.y = y;

        return node;
    }
};

struct _ActionAttack
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const uint64_t aimUID = 0;
    const uint32_t damageID = 0;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_ATTACK;
        node.speed = speed;

        node.x = x;
        node.y = y;

        node.aimUID = aimUID;
        node.extParam.attack.damageID = damageID;
        return node;
    }
};

struct _ActionHitted
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_HITTED;

        node.x = x;
        node.y = y;
        node.direction = direction;

        return node;
    }
};

struct _ActionPickUp
{
    int x = -1;
    int y = -1;

    uint32_t itemID = 0;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_PICKUP;

        node.x = x;
        node.y = y;

        node.extParam.pickUp.itemID = itemID;
        return node;
    }
};

inline bool actionValid(int type)
{
    return type >= ACTION_BEGIN && type < ACTION_END;
}

inline bool actionValid(const ActionNode &node)
{
    return actionValid(node.type);
}

inline const char *actionName(int type)
{
#define _add_action_type_case(t) case t: return #t;
    switch(type){
        _add_action_type_case(ACTION_NONE      )
        _add_action_type_case(ACTION_SPAWN     )
        _add_action_type_case(ACTION_STAND     )
        _add_action_type_case(ACTION_PICKUP    )
        _add_action_type_case(ACTION_MOVE      )
        _add_action_type_case(ACTION_PUSHMOVE  )
        _add_action_type_case(ACTION_SPACEMOVE1)
        _add_action_type_case(ACTION_SPACEMOVE2)
        _add_action_type_case(ACTION_ATTACK    )
        _add_action_type_case(ACTION_SPELL     )
        _add_action_type_case(ACTION_TRANSF    )
        _add_action_type_case(ACTION_HITTED    )
        _add_action_type_case(ACTION_DIE       )
        default: return "ACTION_UNKNOWN";
    }
#undef _add_action_type_case
}

inline const char *actionName(const ActionNode &node)
{
    return actionName(node.type);
}
