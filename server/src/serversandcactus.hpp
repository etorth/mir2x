#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerSandCactus final: public Monster
{
    public:
        ServerSandCactus(ServerMap *mapPtr, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"沙漠树魔"), mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::eval_poller<> updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int, int) const override;

    protected:
        bool canMove() const override
        {
            return false;
        }
};
