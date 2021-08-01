#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerBombSpider final: public Monster
{
    public:
        ServerBombSpider(ServerMap *mapPtr, int argX, int argY, int argDir)
            : Monster(DBCOM_MONSTERID(u8"爆裂蜘蛛"), mapPtr, argX, argY, argDir, 0)
        {}

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int) const override;
};
