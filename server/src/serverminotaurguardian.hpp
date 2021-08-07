#pragma once
#include <cstdint>
#include "monster.hpp"

class ServerMinotaurGuardian final: public Monster
{
    public:
        ServerMinotaurGuardian(uint32_t, ServerMap *, int, int, int, uint64_t);

    protected:
        corof::long_jmper updateCoroFunc() override;

    protected:
        DamageNode getAttackDamage(int) const override;
};
