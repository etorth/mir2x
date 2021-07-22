/*
 * =====================================================================================
 *
 *       Filename: sandcactus.cpp
 *        Created: 04/10/2016 02:32:45
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

#include "pathf.hpp"
#include "sandcactus.hpp"
#include "raiitimer.hpp"

corof::long_jmper SandCactus::updateCoroFunc()
{
    const auto magicID = DBCOM_MAGICID(u8"沙漠树魔_喷刺");
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    fflassert(mr);
    fflassert(mr.castRange);

    while(HP() > 0){
        if(const uint64_t targetUID = co_await coro_pickTarget()){
            if(co_await coro_inDCCastRange(targetUID, mr.castRange)){
                co_await coro_attackUID(targetUID, magicID);
            }
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

DamageNode SandCactus::getAttackDamage(int dc) const
{
    fflassert(to_u32(dc) == DBCOM_MAGICID(u8"沙漠树魔_喷刺"));
    return MagicDamage
    {
        .magicID = dc,
        .damage = 15,
    };
}
