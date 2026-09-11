#pragma once
#include <array>
#include <ranges>
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

    bool randPickLC() const; // luckCurse affects
    int  randPickDC() const;
    int  randPickMC() const;
    int  randPickSC() const;

    int randPickAC () const; // luckCurse doesn't affect
    int randPickMAC() const;

    int minDC() const { return std::ranges::min(dc); }
    int maxDC() const { return std::ranges::max(dc); }
    int minMC() const { return std::ranges::min(mc); }
    int maxMC() const { return std::ranges::max(mc); }
    int minSC() const { return std::ranges::min(sc); }
    int maxSC() const { return std::ranges::max(sc); }
};

// server/client uses same CombatNode calculation
// player's other attributes may affect how CombatNode -> DamageNode, but shall not affect CombatNode itself
CombatNode getCombatNode(const SDWear &, const SDLearnedMagicList &, int, int);
