#pragma once
#include <ranges>
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "servertaosummon.hpp"

class ServerTaoSkeleton final: public ServerTaoSummon
{
    public:
        ServerTaoSkeleton(uint64_t argMapUID, int argX, int argY, uint64_t masterUID)
            : ServerTaoSummon(DBCOM_MONSTERID(u8"变异骷髅"), argMapUID, argX, argY, DIR_DOWNLEFT, masterUID)
        {}

    protected:
        DamageNode getAttackDamage(int dc, int) const override
        {
            fflassert(dc == to_d(DBCOM_MAGICID(u8"物理攻击")));
            return PlainPhyDamage
            {
                .damage = mathf::rand<int>(std::ranges::min(getMR().dc) + std::ranges::min(m_masterSC), std::ranges::max(getMR().dc) + std::ranges::max(m_masterSC) + 1),
                .dcHit = getMR().dcHit,
            };
        }
};
