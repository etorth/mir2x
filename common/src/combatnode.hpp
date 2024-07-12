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

    struct AddElem
    {
        int fire    = 0;
        int ice     = 0;
        int light   = 0;
        int wind    = 0;
        int holy    = 0;
        int dark    = 0;
        int phantom = 0;
    };

    AddElem dcElem {};
    AddElem acElem {};

    struct AddLoad
    {
        int body      = 0;
        int weapon    = 0;
        int inventory = 0;
    }
    load {};

    int randPickDC() const;
    int randPickMC() const;
    int randPickSC() const;

    int minDC() const { return std::min<int>(dc[0], dc[1]); }
    int maxDC() const { return std::max<int>(dc[0], dc[1]); }
    int minMC() const { return std::min<int>(mc[0], mc[1]); }
    int maxMC() const { return std::max<int>(mc[0], mc[1]); }
    int minSC() const { return std::min<int>(sc[0], sc[1]); }
    int maxSC() const { return std::max<int>(sc[0], sc[1]); }
};

// server/client uses same CombatNode calculation
// player's other attributes may affect how CombatNode -> DamageNode, but shall not affect CombatNode itself
CombatNode getCombatNode(const SDWear &, const SDLearnedMagicList &, int, int);
