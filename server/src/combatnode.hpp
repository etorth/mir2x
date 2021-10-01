#pragma once
#include <array>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include "protocoldef.hpp"

struct CombatNode
{
    int  dc[2];
    int  ac[2];
    int mdc[2];
    int mac[2];
    int sdc[2];

    int hit;
    int dodge;
    int speed;
    int comfort;
};
static_assert(std::is_trivially_copyable_v<CombatNode>);
