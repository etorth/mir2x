/*
 * =====================================================================================
 *
 *       Filename: motion.hpp
 *        Created: 04/04/2017 23:57:00
 *  Last Modified: 06/21/2017 23:13:58
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

// following is from Define.h
//
// _MT_STAND		
// _MT_ARROWATTACK	
// _MT_SPELL1		
// _MT_SPELL2		
// _MT_HOLD		
// _MT_PUSHBACK	
// _MT_PUSHBACKFLY	
// _MT_ATTACKMODE	
// _MT_CUT			
// _MT_ONEVSWING
// _MT_TWOVSWING	
// _MT_ONEHSWING	
// _MT_TWOHSWING	
// _MT_SPEARVSWING	
// _MT_SPEARHSWING	
// _MT_HITTED		
// _MT_WHEELWIND	
// _MT_RANDSWING	
// _MT_BACKDROPKICK
// _MT_DIE			
// _MT_SPECIALDIE	
// _MT_WALK		
// _MT_RUN			
// _MT_MOODEPO		
// _MT_ROLL		
// _MT_FISHSTAND	
// _MT_FISHHAND	
// _MT_FISHTHROW	
// _MT_FISHPULL	
// _MT_HORSESTAND	
// _MT_HORSEWALK	
// _MT_HORSERUN	
// _MT_HORSEHIT	

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

    MOTION_PETRIFIED,

    MOTION_MAX,
};
