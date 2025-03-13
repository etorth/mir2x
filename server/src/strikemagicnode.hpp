#include <cstdint>
#include <cstring>
#include <type_traits>
#include "mathf.hpp"
#include "damagenode.hpp"

struct StrikeMagicNode
{
    uint32_t magicID;

    int x;
    int y;

    int minDC;
    int maxDC;

    int effect;
    int element;

    DamageNode getAttackDamge() const
    {
        DamageNode node;
        std::memset(&node, 0, sizeof(node));

        node.magicID = magicID;
        node.damage = mathf::rand(minDC, maxDC);

        node.effect  = 0;
        node.element = 0;

        return node;
    }
};
static_assert(std::is_trivially_copyable_v<StrikeMagicNode>);
