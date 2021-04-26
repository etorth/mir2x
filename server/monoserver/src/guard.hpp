/*
 * =====================================================================================
 *
 *       Filename: guard.hpp
 *        Created: 04/26/2021 02:32:45
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
#include "dbcomid.hpp"
#include "monster.hpp"

class Guard: public Monster
{
    public:
        Guard(uint32_t, ServiceCore *, ServerMap *, int, int, int);
};
