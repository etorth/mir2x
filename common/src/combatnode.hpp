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
    int dc[2] = {0, 0};
    int mc[2] = {0, 0};
    int sc[2] = {0, 0};

    int  ac[2] = {0, 0};
    int mac[2] = {0, 0};

    int dcHit = 0;
    int mcHit = 0;

    int dcDodge = 0;
    int mcDodge = 0;

    int speed = 0;
    int comfort = 0;
    int luckCurse = 0;

    struct AddLoad
    {
        int hand      = 0;
        int body      = 0;
        int inventory = 0;
    }
    load {};

    int minDC() const
    {
        return std::min<int>(dc[0], dc[1]);
    }

    int maxDC() const
    {
        return std::max<int>(dc[0], dc[1]);
    }

    int randPickDC() const;
};

// server/client uses same CombatNode calculation
// player's other attributes may affect how CombatNode -> DamageNode, but shall not affect CombatNode itself
CombatNode getCombatNode(const SDWear &, uint64_t, int);
