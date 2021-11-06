#include "pathf.hpp"
#include "raiitimer.hpp"
#include "serverminotaurguardian.hpp"

ServerMinotaurGuardian::ServerMinotaurGuardian(uint32_t monID, ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t argMasterUID)
    : Monster(monID, mapPtr, argX, argY, argDir, argMasterUID)
{
    fflassert(isMonster(u8"潘夜左护卫") || isMonster(u8"潘夜右护卫"));
}

corof::eval_poller ServerMinotaurGuardian::updateCoroFunc()
{
    const auto [shortDC, longDC] = [this]() -> std::tuple<uint32_t, uint32_t>
    {
        if(isMonster(u8"潘夜左护卫")){
            return
            {
                DBCOM_MAGICID(u8"潘夜左护卫_火魔杖"),
                DBCOM_MAGICID(u8"潘夜左护卫_火球术"),
            };
        }
        else{
            return
            {
                DBCOM_MAGICID(u8"潘夜右护卫_电魔杖"),
                DBCOM_MAGICID(u8"潘夜右护卫_雷电术"),
            };
        }
    }();

    fflassert(shortDC);
    fflassert( longDC);

    fflassert(DBCOM_MAGICRECORD(shortDC));
    fflassert(DBCOM_MAGICRECORD( longDC));

    uint64_t targetUID = 0;
    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            if(co_await coro_inDCCastRange(targetUID, DBCOM_MAGICRECORD(shortDC).castRange)){
                co_await coro_attackUID(targetUID, shortDC);
            }
            else if(co_await coro_inDCCastRange(targetUID, DBCOM_MAGICRECORD(longDC).castRange)){
                co_await coro_attackUID(targetUID, longDC);
            }
            else{
                co_await coro_trackUID(targetUID, {});
            }
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

DamageNode ServerMinotaurGuardian::getAttackDamage(int dc) const
{
    if(isMonster(u8"潘夜左护卫")){
        switch(dc){
            case DBCOM_MAGICID(u8"潘夜左护卫_火魔杖"):
            case DBCOM_MAGICID(u8"潘夜左护卫_火球术"):
                {
                    for(const auto &[uid, coLoc]: m_inViewCOList){
                        if((uidf::getUIDType(uid) == UID_MON) && (uidf::getMonsterID(uid) == DBCOM_MONSTERID(u8"潘夜右护卫"))){
                            return MagicDamage
                            {
                                .magicID = dc,
                                .damage  = 40,
                            };
                        }
                    }

                    return MagicDamage
                    {
                        .magicID = dc,
                        .damage  = 20,
                    };
                }
            default:
                {
                    throw bad_reach();
                }
        }
    }
    else{
        switch(dc){
            case DBCOM_MAGICID(u8"潘夜右护卫_电魔杖"):
            case DBCOM_MAGICID(u8"潘夜右护卫_雷电术"):
                {
                    for(const auto &[uid, coLoc]: m_inViewCOList){
                        if((uidf::getUIDType(uid) == UID_MON) && (uidf::getMonsterID(uid) == DBCOM_MONSTERID(u8"潘夜左护卫"))){
                            return MagicDamage
                            {
                                .magicID = dc,
                                .damage  = 40,
                            };
                        }
                    }

                    return MagicDamage
                    {
                        .magicID = dc,
                        .damage  = 20,
                    };
                }
            default:
                {
                    throw bad_reach();
                }
        }
    }
}
