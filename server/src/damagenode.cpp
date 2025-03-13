#include <cstring>
#include "dbcomid.hpp"
#include "damagenode.hpp"

PlainPhyDamage::operator DamageNode() const
{
    DamageNode node;
    std::memset(&node, 0, sizeof(node));

    node.magicID = DBCOM_MAGICID(u8"物理攻击");
    node.damage = damage;
    node.dcHit = dcHit;
    node.modifierID = this->modifierID;
    return node;
}

MagicDamage::operator DamageNode() const
{
    DamageNode node;
    std::memset(&node, 0, sizeof(node));

    node.magicID = magicID;
    node.damage = damage;
    node.mcHit = mcHit;
    node.modifierID = this->modifierID;
    return node;
}
