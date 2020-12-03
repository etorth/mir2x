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
    //===================================
    MOTION_NONE  = 0,
    MOTION_BEGIN = MOTION_NONE + 1,
    MOTION_STAND = MOTION_BEGIN,
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
    MOTION_END,

    //===================================
    MOTION_MON_NONE,
    MOTION_MON_BEGIN = MOTION_MON_NONE + 1,
    MOTION_MON_STAND = MOTION_MON_BEGIN,
    MOTION_MON_WALK,
    MOTION_MON_ATTACK0,
    MOTION_MON_HITTED,
    MOTION_MON_DIE,

    MOTION_MON_ATTACK1,
    MOTION_MON_SPELL0,
    MOTION_MON_SPELL1,
    MOTION_MON_APPEAR,
    MOTION_MON_SPECIAL,
    MOTION_MON_END,

    //===================================
    MOTION_NPC_NONE,
    MOTION_NPC_BEGIN = MOTION_NPC_NONE + 1,
    MOTION_NPC_STAND = MOTION_NPC_BEGIN,
    MOTION_NPC_ACT,
    MOTION_NPC_ACTEXT,
    MOTION_NPC_END,
};
