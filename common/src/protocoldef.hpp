/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
 *  Last Modified: 03/31/2017 00:34:40
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
enum ActionType: uint8_t
{
    ACTION_NONE    = 0,
    ACTION_STAND   = 1,
    ACTION_WALK    = 2,
    ACTION_ATTACK  = 3,
    ACTION_DIE     = 4,
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
    DIR_NONE        = 8,
};
