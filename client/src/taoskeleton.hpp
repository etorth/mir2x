/*
 * =====================================================================================
 *
 *       Filename: taoskeleton.hpp
 *        Created: 08/31/2015 08:26:19
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
#include "monster.hpp"

class TaoSkeleton: public Monster
{
    public:
        TaoSkeleton(uint64_t uid, ProcessRun *proc, const ActionNode &action)
            : Monster(uid, proc, action)
        {
            checkMonsterNameEx(u8"变异骷髅");
        }
};
