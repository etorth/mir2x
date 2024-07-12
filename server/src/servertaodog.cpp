#include "mathf.hpp"
#include "pathf.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "servertaodog.hpp"

corof::eval_poller ServerTaoDog::updateCoroFunc()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            idleTime.reset();
            setStandMode(true);
            co_await coro_trackAttackUID(targetUID);
        }

        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }

            if(m_actorPod->checkUIDValid(masterUID())){
                co_await coro_followMaster();
            }
            else{
                co_await corof::async_wait(200);
            }
        }
    }

    goDie();
    co_return true;
}

void ServerTaoDog::attackUID(uint64_t targetUID, int dcType, std::function<void()> onOK, std::function<void()> onError)
{
    fflassert(to_u32(dcType) == DBCOM_MAGICID(u8"神兽_喷火"));
    if(!canAttack()){
        if(onError){
            onError();
        }
        return;
    }

    if(!dcValid(dcType, true)){
        if(onError){
            onError();
        }
        return;
    }

    m_attackLock = true;
    getCOLocation(targetUID, [this, dcType, targetUID, onOK, onError](const COLocation &coLoc)
    {
        fflassert(m_attackLock);
        m_attackLock = false;

        const auto &mr = DBCOM_MAGICRECORD(dcType);
        if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
            if(onError){
                onError();
            }
            return;
        }

        if(const auto newDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(newDir)){
            m_direction = newDir;
        }

        if(!canAttack()){
            if(onError){
                onError();
            }
            return;
        }

        const auto [buffID, modifierID] = m_buffList.rollAttackModifier();
        dispatchAction(ActionAttack
        {
            .speed = attackSpeed(),
            .x = X(),
            .y = Y(),
            .aimUID = targetUID,
            .extParam
            {
                .magicID = to_u32(dcType),
                .modifierID = to_u32(modifierID),
            },
        });

        if(buffID){
            sendBuff(buffID, 0, buffID);
        }

        addDelay(550, [dcType, modifierID, this]()
        {
            std::vector<std::tuple<int, int>> gridList;
            for(const auto r: {1, 2}){
                if(const auto [attackGX, attackGY] = pathf::getFrontGLoc(X(), Y(), Direction(), r); m_map->groundValid(attackGX, attackGY)){
                    gridList.push_back({attackGX, attackGY});
                }
            }

            if(gridList.empty()){
                return;
            }

            AMAttack amA;
            std::memset(&amA, 0, sizeof(amA));

            amA.UID = UID();
            amA.mapID = m_map->UID();

            amA.X = X();
            amA.Y = Y();
            amA.damage = getAttackDamage(dcType, modifierID);

            foreachInViewCO([amA, gridList, this](const COLocation &inViewCOLoc)
            {
                if(std::find(gridList.begin(), gridList.end(), std::make_tuple(inViewCOLoc.x, inViewCOLoc.y)) == gridList.end()){
                    return;
                }

                queryFinalMaster(UID(), [inViewCOLoc, amA, this](uint64_t selfFinalMaster)
                {
                    queryFinalMaster(inViewCOLoc.uid, [inViewCOLoc, selfFinalMaster, amA, this](uint64_t enemyFinalMaster)
                    {
                        if(selfFinalMaster == enemyFinalMaster){
                            return;
                        }

                        if(uidf::getUIDType(enemyFinalMaster) == UID_MON){
                            m_actorPod->forward(inViewCOLoc.uid, {AM_ATTACK, amA});
                            return;
                        }

                        // TODO
                        // need more check
                    });

                });
            });
        });

        if(onOK){
            onOK();
        }
    },

    [this, targetUID, onError]()
    {
        m_attackLock = false;
        m_inViewCOList.erase(targetUID);

        if(onError){
            onError();
        }
    });
}

void ServerTaoDog::onAMAttack(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAttack>();
    if(amA.UID == UID()){
        return;
    }

    if(m_dead.get()){
        notifyDead(amA.UID);
        return;
    }

    setStandMode(true);
    Monster::onAMAttack(mpk);
}

DamageNode ServerTaoDog::getAttackDamage(int dc, int modifierID) const
{
    fflassert(to_u32(dc) == DBCOM_MAGICID(u8"神兽_喷火"));
    return MagicDamage
    {
        .magicID = dc,
        .damage = mathf::rand<int>(getMR().mc[0] + m_masterSC[0], getMR().mc[1] + m_masterSC[1]),
        .mcHit = getMR().mcHit,
        .modifierID = modifierID,
    };
}
