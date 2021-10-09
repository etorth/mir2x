/*
 * =====================================================================================
 *
 *       Filename: damagenode.hpp
 *        Created: 07/21/2017 17:12:19
 *    Description: description of a damage
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <array>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <cinttypes>
#include "protocoldef.hpp"

struct DamageNode
{
    int magicID;
    int damage;

    int dcHit;
    int mcHit;
    int effect; // 穿刺 吸血 etc

    operator bool () const
    {
        return magicID != 0;
    }
};
static_assert(std::is_trivially_copyable_v<DamageNode>);

struct PlainPhyDamage
{
    int damage = 0;
    int dcHit = 0;
    operator DamageNode() const;
};

struct MagicDamage
{
    int magicID = 0;
    int damage = 0;
    int mcHit = 0;
    operator DamageNode() const;
};
