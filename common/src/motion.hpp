/*
 * =====================================================================================
 *
 *       Filename: motion.hpp
 *        Created: 04/04/2017 23:57:00
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
    // human motion section
    // 1. for human motion we don't have MOTION_ATTACK
    //    instead we have more detailed attack motion definition
    // 2. MOTION_ARROWATTACK is not valid in current DB

    MOTION_NONE = 0,

    MOTION_STAND,
    MOTION_ARROWATTACK,
    MOTION_SPELL0,
    MOTION_SPELL1,
    MOTION_HOLD,
    MOTION_PUSHBACK,
    MOTION_PUSHBACKFLY,
    MOTION_ATTACKMODE,
    MOTION_CUT,
    MOTION_ONEVSWING,
    MOTION_TWOVSWING,
    MOTION_ONEHSWING,
    MOTION_TWOHSWING,
    MOTION_SPEARVSWING,
    MOTION_SPEARHSWING,
    MOTION_HITTED,
    MOTION_WHEELWIND,
    MOTION_RANDSWING,
    MOTION_BACKDROPKICK,
    MOTION_DIE,
    MOTION_ONHORSEDIE,
    MOTION_WALK,
    MOTION_RUN,
    MOTION_MOODEPO,
    MOTION_ROLL,
    MOTION_FISHSTAND,
    MOTION_FISHHAND,
    MOTION_FISHTHROW,
    MOTION_FISHPULL,
    MOTION_ONHORSESTAND,
    MOTION_ONHORSEWALK,
    MOTION_ONHORSERUN,
    MOTION_ONHORSEHITTED,

    MOTION_MAX,

    // monster motion section
    // 1. don't share human motion indices
    //    because when using it we use int rather than the MotionType
    //    if share the motion index we could confuse ourselves for debug
    //
    // 2. monster motion use ATTACK0 and ATTACK1
    //    the first five motions are usually used and rest are speciall
    MOTION_MON_NONE,

    MOTION_MON_STAND,
    MOTION_MON_WALK,
    MOTION_MON_ATTACK0,
    MOTION_MON_HITTED,
    MOTION_MON_DIE,

    MOTION_MON_ATTACK1,
    MOTION_MON_SPELL0,
    MOTION_MON_SPELL1,
    MOTION_MON_APPEAR,
    MOTION_MON_SPECIAL,

    MOTION_MON_MAX,
};
