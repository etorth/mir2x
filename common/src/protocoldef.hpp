/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
 *  Last Modified: 07/04/2017 12:36:05
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
#include "motion.hpp"

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
    DIR_MAX,
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
    ACTION_MAX,
};

enum ActExtType: int
{
    ACTEXT_NONE = 0,

    ACTEXT_FLYIN,
    ACTEXT_FLYOUT,

    ACTEXT_FADEIN,
    ACTEXT_FADEOUT,
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
    MID_NONE,

    MID_DEER,
    MID_M10,

    MID_PHEASANT,

    MID_ZUMA_ARCHER,
    MID_ZUMA_WARRIOR,

    MID_MAX = 65535,
};

enum DCType: int
{
    DC_NONE = 0,
    DC_PHY_PLAIN,
    DC_PHY_WIDESWORD,
    DC_MAG_FIRE,
    DC_MAG_EXPLODE,
};

enum ECType: int
{
    EC_NONE = 0,
    EC_PLAIN,
    EC_FIRE,
    EC_ICE,
    EC_LIGHT,
    EC_WIND,
    EC_HOLY,
    EC_DARK,
    EC_PHANTOM,
    EC_MAX,
};

enum EffectType: int
{
    ET_NONE = 0,
    ET_PIERCE,
    ET_PARALYSIS,
};

enum GenType: int
{
    GT_NONE = 0,
    GT_MALE,
    GT_FEMALE,
};

enum MonsterAttackType: int
{
    MA_NONE = 0,
    MA_NORMAL,
    MA_DOGZ,
    MA_ATTACKALL,
};

enum WeaponIDType: int
{
    WEAPON_NONE =   0,
    WEAPON_MAX  = 256,
};

enum DressIDType : int
{
    DRESS_NONE =   0,
    DRESS_MAX  = 256,
};
