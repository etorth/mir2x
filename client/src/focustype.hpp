/*
 * =====================================================================================
 *
 *       Filename: focustype.hpp
 *        Created: 06/27/2017 11:39:59
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
    FOCUS_NONE  = 0,
    FOCUS_BEGIN = 1,
    FOCUS_SOLID = 1, // draw everything normally, rest only draw body
    FOCUS_ALPHA,     // draw body with alpha, used when behind buildings
    FOCUS_MOUSE,
    FOCUS_MAGIC,
    FOCUS_FOLLOW,
    FOCUS_ATTACK,
    FOCUS_CLICKNPC,
    FOCUS_END,
};
