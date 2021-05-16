/*
 * =====================================================================================
 *
 *       Filename: damagenode.cpp
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

#include <cstring>
#include "dbcomid.hpp"
#include "damagenode.hpp"

PlainPhyDamage::operator DamageNode() const
{
    DamageNode node;
    std::memset(&node, 0, sizeof(node));

    node.magicID = DBCOM_MAGICID(u8"物理攻击");
    node.damage = damage;
    return node;
}

MagicDamage::operator DamageNode() const
{
    DamageNode node;
    std::memset(&node, 0, sizeof(node));

    node.magicID = magicID;
    node.damage = damage;
    return node;
}
