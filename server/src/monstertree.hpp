/*
 * =====================================================================================
 *
 *       Filename: monstertree.hpp
 *        Created: 04/07/2016 03:48:41 AM
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

class MonsterTree final: public Monster
{
    public:
        MonsterTree(uint32_t monsterID, const ServerMap *mapPtr, int argX, int argY)
            : Monster(monsterID, mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .x = X(),
                .y = Y(),
                .direction = DIR_BEGIN,
            };
        }
};
