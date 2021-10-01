#pragma once
#include <array>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include "serdesmsg.hpp"
#include "protocoldef.hpp"

struct CombatNode
{
    int  dc[2] = {0, 0};
    int  ac[2] = {0, 0};
    int mdc[2] = {0, 0};
    int mac[2] = {0, 0};
    int sdc[2] = {0, 0};

    int hit     = 0;
    int dodge   = 0;
    int speed   = 0;
    int comfort = 0;
};

// server/client uses same CombatNode calculation
// player's other attributes may affect how CombatNode -> DamageNode, but shall not affect CombatNode itself
CombatNode getCombatNode(const SDWear &, uint64_t, int);
