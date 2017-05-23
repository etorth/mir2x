/*
 * =====================================================================================
 *
 *       Filename: zumamonster.hpp
 *        Created: 04/08/2017 16:30:48
 *  Last Modified: 05/20/2017 15:48:42
 *
 *    Description: zuma monster which can be petrified
 *                 can't apply any action when petrified, also can't attack them
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
#include "monster.hpp"

class ZumaMonster: public Monster
{
    protected:
        ZumaMonster(uint32_t nUID, uint32_t nMonsterID, ProcessRun *pRun)
            : Monster(nUID, nMonsterID, pRun)
        {}

    public:
       ~ZumaMonster() = default;

    public:
        static ZumaMonster *Create(uint32_t, uint32_t, ProcessRun *, const ActionNode &);
};
