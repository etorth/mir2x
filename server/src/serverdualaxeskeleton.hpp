#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerDualAxeSkeleton final: public Monster
{
    public:
        ServerDualAxeSkeleton(ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"掷斧骷髅"), mapPtr, argX, argY, argDir, masterUID)
        {}

    protected:
        DamageNode getAttackDamage(int dc, int) const override
        {
            fflassert(to_u32(dc) == DBCOM_MAGICID(u8"掷斧骷髅_掷斧"));
            return MagicDamage
            {
                .magicID = dc,
                .damage = 15,
            };
        }
};
