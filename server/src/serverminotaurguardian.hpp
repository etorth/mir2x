#pragma once
#include <cstdint>
#include "monster.hpp"

class ServerMinotaurGuardian final: public Monster
{
    public:
        ServerMinotaurGuardian(uint32_t, uint64_t, int, int, int, uint64_t);

    protected:
        corof::awaitable<> runAICoro() override;

    protected:
        DamageNode getAttackDamage(int, int) const override;
};
