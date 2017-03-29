/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
 *  Last Modified: 03/28/2017 11:51:27
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

// since we have stand / walk / attack / dead for monster, so call
// it ``motion state" may be inproper, action state is better
enum ActionType: int
{
    ACTION_UNKNOWN = 0,

    ACTION_STAND   = 1,  // monster
    ACTION_WALK    = 2,  // monster
    ACTION_ATTACK  = 3,  // monster
    ACTION_DIE     = 4,  // monster
};

enum CreatureType: int
{
    CREATURE_NONE = 0,
    CREATURE_PLAYER,
    CREATURE_MONSTER,
};

// define of directioin
//
//               0
//            7     1
//          6    +--> 2
//            5  |  3
//               V
//               4
//
enum _Direction: int
{
    DIR_UP          = 0,
    DIR_DOWN        = 4,
    DIR_LEFT        = 6,
    DIR_RIGHT       = 2,
    DIR_UPLEFT      = 7,
    DIR_UPRIGHT     = 1,
    DIR_DOWNLEFT    = 5,
    DIR_DOWNRIGHT   = 3,
    DIR_UNKNOWN     = 8,
};
