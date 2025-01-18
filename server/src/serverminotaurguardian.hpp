#pragma once
#include <cstdint>
#include "monster.hpp"

class ServerMinotaurGuardian final: public Monster
{
    public:
        ServerMinotaurGuardian(uint32_t, ServerMap *, int, int, int, uint64_t);

    protected:
        corof::eval_poller<> updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int, int) const override;
};
