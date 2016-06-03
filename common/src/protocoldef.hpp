/*
 * =====================================================================================
 *
 *       Filename: protocoldef.hpp
 *        Created: 06/03/2016 11:40:51
 *  Last Modified: 06/03/2016 12:08:52
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

enum MotionStateType: int{
    MOTION_UNKNOWN = 0,
    MOTION_STAND,
    MOTION_WALK,
    MOTION_RUN,
};

enum CreatureType: int{
    CREATURE_UNKNOWN = 0,
    CREATURE_PLAYER,
    CREATURE_MONSTER,
};
