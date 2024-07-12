#include "pathf.hpp"
#include "fflerror.hpp"
#include "serveranthealer.hpp"

void ServerAntHealer::sendHeal(uint64_t uid)
{
    fflassert(uid);
    if(const auto p = m_inViewCOList.find(uid); p != m_inViewCOList.end()){
        const auto magicID = DBCOM_MAGICID(u8"蚂蚁道士_治疗");
        const auto &mr = DBCOM_MAGICRECORD(magicID);

        fflassert(magicID);
        fflassert(mr);

        if(pathf::inDCCastRange(mr.castRange, X(), Y(), p->second.x, p->second.y)){
            AMHeal amH;
            std::memset(&amH, 0, sizeof(amH));

            amH.mapID = mapID();
            amH.x = X();
            amH.y = Y();

            amH.addHP = 10;
            amH.addMP =  5;

            m_actorPod->forward(p->first, {AM_HEAL, amH});
            dispatchAction(ActionAttack
            {
                .speed = attackSpeed(),
                .x = X(),
                .y = Y(),
                .aimUID = uid,
                .extParam
                {
                    .magicID = to_u32(magicID),
                },
            });
        }
    }
}

corof::eval_poller ServerAntHealer::updateCoroFunc()
{
    while(m_sdHealth.hp > 0){
        if(m_sdHealth.hp < m_sdHealth.maxHP){
            dispatchAction(ActionAttack
            {
                .speed = attackSpeed(),
                .x = X(),
                .y = Y(),
                .aimUID = UID(),
                .extParam
                {
                    .magicID = to_u32(DBCOM_MAGICID(u8"蚂蚁道士_治疗")),
                },
            });

            updateHealth(20);
        }
        else{
            const auto targetUID = co_await coro_pickHealTarget();
            if(targetUID && m_inViewCOList.count(targetUID)){
                sendHeal(targetUID);
            }
        }
        co_await corof::async_wait(1000);
    }

    goDie();
    co_return true;
}
