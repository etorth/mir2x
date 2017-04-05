/*
 * =====================================================================================
 *
 *       Filename: motion.hpp
 *        Created: 04/04/2017 23:57:00
 *  Last Modified: 04/05/2017 12:14:54
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

enum MotionType: int
{
    MOTION_NONE = 0,

    MOTION_STAND,
    MOTION_WALK,
    MOTION_ATTACK,
    MOTION_UNDERATTACK,
    MOTION_DIE,

    MOTION_RUN,
    MOTION_PUSH,
    MOTION_BACKROLL,

    MOTION_CUT,
    MOTION_KICK,
    MOTION_ARROWSHOOT,
    MOTION_CASTMAGIC0,
    MOTION_CASTMAGIC1,
    MOTION_WITHSTAND,
    MOTION_UNDERKICK0,
    MOTION_UNDERKICK1,
    MOTION_BEFOREATTACK,

    MOTION_ONEHANDATTACK0,
    MOTION_ONEHANDATTACK1,      // half-moon blade
    MOTION_ONEHANDATTACK2,      // swoop-sword
    MOTION_ONEHANDATTACK3,      // double-attack

    MOTION_TWOHANDATTACK0,
    MOTION_TWOHANDATTACK1,
    MOTION_TWOHANDATTACK2,
    MOTION_TWOHANDATTACK3,

    MOTION_FISHING,
    MOTION_GETFISH,
    MOTION_STARTFISHING,
    MOTION_STOPFISHING,

    MOTION_HORSEWALK,
    MOTION_HORSERUN,
    MOTION_HORSEDIE,
    MOTION_HORSEUNDERATTACK,
};
