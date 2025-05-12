#include "pathf.hpp"
#include "serversandcactus.hpp"
#include "raiitimer.hpp"

corof::awaitable<> ServerSandCactus::runAICoro()
{
    const auto magicID = DBCOM_MAGICID(u8"沙漠树魔_喷刺");
    const auto &mr = DBCOM_MAGICRECORD(magicID);

    fflassert(mr);
    fflassert(mr.castRange);

    uint64_t targetUID = 0;
    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            if(co_await inDCCastRange(targetUID, mr.castRange)){
                co_await attackUID(targetUID, magicID);
            }
        }
        co_await asyncWait(200);
    }

    goDie();
}

DamageNode ServerSandCactus::getAttackDamage(int dc, int) const
{
    fflassert(to_u32(dc) == DBCOM_MAGICID(u8"沙漠树魔_喷刺"));
    return MagicDamage
    {
        .magicID = dc,
        .damage = 15,
    };
}
