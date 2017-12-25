/*
 * =====================================================================================
 *
 *       Filename: reg.hpp
 *        Created: 12/21/2017 13:48:41
 *  Last Modified: 12/21/2017 15:43:04
 *
 *    Description: regs are initialized as zero
 *                 then when define state, define zero as most *normal* one
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

enum: uint8_t
{
    REG_NONE = 0,

    REG_NEVERDIE,       // would never die
                        // by default is zero: false

    REG_LIFE,
    REG_LIFE_ALIVE  = 0,
    REG_LIFE_DEAD   = 1,
    REG_LIFE_GHOST  = 2,

    REG_ATTACK,
    REG_ATTACK_NORMAL = 0,
    REG_ATTACK_ALL,
    REG_ATTACK_DOGZ,
    REG_ATTACK_PEACE,

    REG_HORSE,
    REG_HORSE_RED,
    REG_HORSE_BLACK,
    REG_HORSE_WHITE,
};
