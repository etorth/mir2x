/*
 * =====================================================================================
 *
 *       Filename: dualaxeskeleton.hpp
 *        Created: 07/10/2021 02:32:45
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

class DualAxeSkeleton final: public Monster
{
    public:
        DualAxeSkeleton(ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"掷斧骷髅"), mapPtr, argX, argY, argDir, masterUID)
        {}

    protected:
        DamageNode getAttackDamage(int dc) const override
        {
            fflassert(to_u32(dc) == DBCOM_MAGICID(u8"掷斧骷髅_掷斧"));
            return MagicDamage
            {
                .magicID = dc,
                .damage = 15,
            };
        }
};
