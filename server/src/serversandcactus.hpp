#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerSandCactus final: public Monster
{
    public:
        ServerSandCactus(uint64_t argMapUID, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"沙漠树魔"), argMapUID, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::eval_poller<> updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int, int) const override;

    protected:
        bool canMove(bool) const override
        {
            return false;
        }
};
