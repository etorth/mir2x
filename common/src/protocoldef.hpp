/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
 *  Last Modified: 06/14/2016 23:58:07
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
enum ActionType: int{
    ACTION_UNKNOWN = 0,

    ACTION_STAND   = 1,  // monster
    ACTION_WALK    = 2,  // monster
    ACTION_ATTACK  = 3,  // monster
    ACTION_DIE     = 4,  // monster
};

enum CreatureType: int{
    CREATURE_UNKNOWN = 0,
    CREATURE_PLAYER,
    CREATURE_MONSTER,
};
