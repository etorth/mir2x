/*
 * =====================================================================================
 *
 *       Filename: focustype.hpp
 *        Created: 06/27/2017 11:39:59
 *  Last Modified: 08/08/2017 22:59:19
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
enum FocusType: int
{
    FOCUS_NONE   = 0,
    FOCUS_MOUSE,
    FOCUS_MAGIC,
    FOCUS_FOLLOW,
    FOCUS_ATTACK,
    FOCUS_MAX,
};
