/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
 *  Last Modified: 04/09/2017 23:23:27
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

// define of directioin
//
//               0
//            7     1
//          6    +--> 2
//            5  |  3
//               V
//               4
//
enum DirectionType: int
{
    DIR_NONE = 0,
    DIR_UP,
    DIR_UPRIGHT,
    DIR_RIGHT,
    DIR_DOWNRIGHT,
    DIR_DOWN,
    DIR_DOWNLEFT,
    DIR_LEFT,
    DIR_UPLEFT,
};

enum ActionType: int
{
    ACTION_NONE = 0,
    ACTION_STAND,
    ACTION_MOVE,
    ACTION_ATTACK,
    ACTION_UNDERATTACK,
    ACTION_DIE,
    ACTION_EXTENSION,
};

enum CreatureType: int
{
    CREATURE_NONE = 0,
    CREATURE_PLAYER,
    CREATURE_MONSTER,
    CREATURE_NPC,
};

enum MonsterIDType: uint32_t
{
    MONSTERID_NONE,
    MONSTERID_DEER,
    MONSTERID_PHEASANT,
    MONSTERID_ZUMA0,
    MONSTERID_ZUMA1,
    MONSTERID_ZUMA2,
    MONSTERID_ZUMA3,
};
