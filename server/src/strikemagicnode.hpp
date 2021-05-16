/*
 * =====================================================================================
 *
 *       Filename: strikemagicnode.hpp
 *        Created: 05/03/2016 13:19:07
 *    Description:
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
