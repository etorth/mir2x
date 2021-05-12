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
    int type;
    int damage;

    int effect;
    int element;

    operator bool () const
    {
        return type != 0;
    }
};
static_assert(std::is_trivially_copyable_v<DamageNode>);

struct PlainPhyDamage
{
    int damage = 0;
    operator DamageNode() const;
};
