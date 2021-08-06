/*
 * =====================================================================================
 *
 *       Filename: serversandcactus.cpp
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
#include "serversandcactus.hpp"
#include "raiitimer.hpp"

corof::long_jmper ServerSandCactus::updateCoroFunc()
{
    const auto magicID = DBCOM_MAGICID(u8"沙漠树魔_喷刺");
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    fflassert(mr);
    fflassert(mr.castRange);

    uint64_t targetUID = 0;
    while(m_sdHealth.HP > 0){
        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            m_inViewCOList.erase(targetUID);
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            if(co_await coro_inDCCastRange(targetUID, mr.castRange)){
                co_await coro_attackUID(targetUID, magicID);
            }
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

DamageNode ServerSandCactus::getAttackDamage(int dc) const
{
    fflassert(to_u32(dc) == DBCOM_MAGICID(u8"沙漠树魔_喷刺"));
    return MagicDamage
    {
        .magicID = dc,
        .damage = 15,
    };
}
