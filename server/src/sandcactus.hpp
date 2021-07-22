/*
 * =====================================================================================
 *
 *       Filename: sandcactus.hpp
 *        Created: 07/19/2021 11:32:45
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

class SandCactus final: public Monster
{
    public:
        SandCactus(ServerMap *mapPtr, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"沙漠树魔"), mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int) const override;

    protected:
        bool canMove() const override
        {
            return false;
        }
};
