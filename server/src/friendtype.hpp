/*
 * =====================================================================================
 *
 *       Filename: friendtype.hpp
 *        Created: 09/23/2017 22:30:18
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
enum FriendType: int
{
    FT_NONE    = 0,
    FT_ERROR   = 1,
    FT_FRIEND  = 2,
    FT_ENEMY   = 3,
    FT_NEUTRAL = 4,
};
