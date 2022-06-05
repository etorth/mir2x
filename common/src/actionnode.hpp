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
    ACTION_MOVE,
    ACTION_JUMP,
    ACTION_PUSHMOVE,
    ACTION_SPACEMOVE,
    ACTION_ATTACK,
    ACTION_SPELL,
    ACTION_TRANSF,
    ACTION_HITTED,
    ACTION_SPINKICK,
    ACTION_DIE,
    ACTION_END,
};

#pragma pack(push, 1)
struct ActionNode
{
    uint16_t type      : 5;
    uint16_t speed     : 9;
    uint16_t direction : 5;

    int16_t x;
    int16_t y;

    int16_t aimX;
    int16_t aimY;

    union
    {
        uint64_t  aimUID;
        uint64_t fromUID; // for ACTION_HITTED
    };

    struct ExtParamStand
    {
        struct TaoDogStand
        {
            uint8_t standMode;
        };

        struct CannibalPlantStand
        {
            uint8_t standMode;
        };

        struct SandGhostStand
        {
            uint8_t standMode;
        };

        struct RebornZombieStand
        {
            uint8_t standMode;
        };

        struct EvilCentipedeStand
        {
            uint8_t standMode;
        };

        struct ZumaMonsterStand
        {
            uint8_t standMode;
        };

        struct ZumaTaurusStand
        {
            uint8_t standMode;
        };

        struct NPCStand
        {
            uint8_t act;
        };

        union
        {
            TaoDogStand dog;
            CannibalPlantStand cannibalPlant;
            SandGhostStand sandGhost;
            RebornZombieStand rebornZombie;
            EvilCentipedeStand evilCentipede;
            ZumaMonsterStand zumaMonster;
            ZumaTaurusStand zumaTaurus;
            NPCStand npc;
        };
    };

    struct ExtParamTransf
    {
        struct TaoDogTransf
        {
            uint8_t standModeReq;
        };

        struct CannibalPlantTransf
        {
            uint8_t standModeReq;
        };

        struct SandGhostTransf
        {
            uint8_t standModeReq;
        };

        struct RebornZombieTransf
        {
            uint8_t standModeReq;
        };

        struct EvilCentipedeTransf
        {
            uint8_t standModeReq;
        };

        struct ZumaMonsterTransf
        {
            uint8_t standModeReq;
        };

        struct ZumaTaurusTransf
        {
            uint8_t standModeReq;
        };

        union
        {
            TaoDogTransf dog;
            CannibalPlantTransf cannibalPlant;
            SandGhostTransf sandGhost;
            RebornZombieTransf rebornZombie;
            EvilCentipedeTransf evilCentipede;
            ZumaMonsterTransf zumaMonster;
            ZumaTaurusTransf zumaTaurus;
        };
    };

    struct ExtParamDie
    {
        struct TaoDogDie
        {
            uint8_t standMode;
        };

        union
        {
            TaoDogDie dog;
        };
    };

    struct ExtParamSpell
    {
        uint32_t magicID;
        uint32_t modifierID;
    };

    struct ExtParamMove
    {
        uint8_t pickUp  : 1;
        uint8_t onHorse : 1;
    };

    struct ExtParamAttack
    {
        uint32_t magicID;
        uint32_t modifierID;
    };

    struct ExtParamHitted
    {
        struct TaoDogHitted
        {
            uint8_t standMode;
        };

        union
        {
            TaoDogHitted dog;
        };
    };

    union ActionExtParam // prefer named union than anoynmous union
    {
        ExtParamStand  stand;
        ExtParamTransf transf;
        ExtParamDie    die;
        ExtParamSpell  spell;
        ExtParamMove   move;
        ExtParamAttack attack;
        ExtParamHitted hitted;
    } extParam;
};
#pragma pack(pop)

struct ActionSpinKick
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const int aimX = x;
    const int aimY = y;

    const uint64_t aimUID = 0;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_SPINKICK;
        node.speed = speed;

        node.x = x;
        node.y = y;

        node.aimX = aimX;
        node.aimY = aimY;
        node.aimUID = aimUID;
        return node;
    }
};

struct ActionDie
{
    const int x = -1;
    const int y = -1;
    const ActionNode::ExtParamDie extParam = {};

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_DIE;

        node.x = x;
        node.y = y;

        node.extParam.die = extParam;
        return node;
    }
};

struct ActionStand
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;
    ActionNode::ExtParamStand extParam = {};

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_STAND;
        node.direction = direction;

        node.x = x;
        node.y = y;

        node.extParam.stand = extParam;
        return node;
    }
};

struct ActionTransf
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;
    const ActionNode::ExtParamTransf extParam = {};

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_TRANSF;
        node.direction = direction;

        node.x = x;
        node.y = y;

        node.extParam.transf = extParam;
        return node;
    }
};

struct ActionJump
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_JUMP;
        node.direction = direction;

        node.x = x;
        node.y = y;
        return node;
    }
};

struct ActionSpawn
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

struct ActionSpell
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const int aimX = x;
    const int aimY = y;

    const uint64_t aimUID = 0;
    const uint32_t magicID = 0;
    const uint32_t modifierID = 0;

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

struct ActionMove
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const int aimX = -1;
    const int aimY = -1;

    const bool pickUp  = false;
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

        node.extParam.move.pickUp = pickUp;
        node.extParam.move.onHorse = onHorse;
        return node;
    }
};

struct ActionSpaceMove
{
    const int x = -1;
    const int y = -1;
    const int aimX = -1;
    const int aimY = -1;
    const int direction = DIR_NONE;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_SPACEMOVE;
        node.direction = direction;

        node.x = x;
        node.y = y;
        node.aimX = aimX;
        node.aimY = aimY;

        return node;
    }
};

struct ActionAttack
{
    const int speed = SYS_DEFSPEED;

    const int x = -1;
    const int y = -1;

    const uint64_t aimUID = 0;
    const uint32_t magicID = 0;
    const uint32_t modifierID = 0;

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_ATTACK;
        node.speed = speed;

        node.x = x;
        node.y = y;

        node.aimUID = aimUID;
        node.extParam.attack.magicID = magicID;
        node.extParam.attack.modifierID = modifierID;
        return node;
    }
};

struct ActionHitted
{
    const int x = -1;
    const int y = -1;
    const int direction = DIR_NONE;
    const uint64_t fromUID = 0;
    const ActionNode::ExtParamHitted extParam = {};

    operator ActionNode () const
    {
        ActionNode node;
        std::memset(&node, 0, sizeof(node));

        node.type = ACTION_HITTED;

        node.x = x;
        node.y = y;
        node.direction = direction;
        node.fromUID = fromUID;

        node.extParam.hitted = extParam;
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
        _add_action_type_case(ACTION_NONE     )
        _add_action_type_case(ACTION_SPAWN    )
        _add_action_type_case(ACTION_STAND    )
        _add_action_type_case(ACTION_MOVE     )
        _add_action_type_case(ACTION_JUMP     )
        _add_action_type_case(ACTION_PUSHMOVE )
        _add_action_type_case(ACTION_SPACEMOVE)
        _add_action_type_case(ACTION_ATTACK   )
        _add_action_type_case(ACTION_SPELL    )
        _add_action_type_case(ACTION_TRANSF   )
        _add_action_type_case(ACTION_HITTED   )
        _add_action_type_case(ACTION_SPINKICK )
        _add_action_type_case(ACTION_DIE      )
        default: return "ACTION_UNKNOWN";
    }
#undef _add_action_type_case
}

inline const char *actionName(const ActionNode &node)
{
    return actionName(node.type);
}
