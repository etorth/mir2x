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

            amH.mapUID = mapUID();
            amH.x = X();
            amH.y = Y();

            amH.addHP = 10;
            amH.addMP =  5;

            m_actorPod->post(p->first, {AM_HEAL, amH});
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

corof::awaitable<> ServerAntHealer::runAICoro()
{
    while(!m_sdHealth.dead()){
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
            co_await asyncWait(1000);
            continue;
        }

        if(const auto targetUID = co_await pickHealTarget(); targetUID && m_inViewCOList.count(targetUID)){
            sendHeal(targetUID);
            co_await asyncWait(1000);
            continue;
        }

        co_await asyncIdleWait(1000);
    }
}
