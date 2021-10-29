/*
 * =====================================================================================
 *
 *       Filename: servertaodog.cpp
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

#include "mathf.hpp"
#include "pathf.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "servertaodog.hpp"

corof::eval_poller ServerTaoDog::updateCoroFunc()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(m_sdHealth.HP > 0){
        if(targetUID && !m_actorPod->checkUIDValid(targetUID)){
            m_inViewCOList.erase(targetUID);
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

        if(const auto newDir = PathFind::GetDirection(X(), Y(), coLoc.x, coLoc.y); directionValid(newDir)){
            m_direction = newDir;
        }

        if(!canAttack()){
            if(onError){
                onError();
            }
            return;
        }

        dispatchAction(ActionAttack
        {
            .speed = attackSpeed(),
            .x = X(),
            .y = Y(),
            .aimUID = targetUID,
            .damageID = to_u32(dcType),
        });

        addDelay(550, [dcType, this]()
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
            amA.damage = getAttackDamage(dcType);

            foreachInViewCO([amA, gridList, this](const COLocation &coLoc)
            {
                if(std::find(gridList.begin(), gridList.end(), std::make_tuple(coLoc.x, coLoc.x)) == gridList.end()){
                    return;
                }

                queryFinalMaster(UID(), [coLoc, amA, this](uint64_t selfFinalMaster)
                {
                    queryFinalMaster(coLoc.uid, [coLoc, selfFinalMaster, amA, this](uint64_t enemyFinalMaster)
                    {
                        if(selfFinalMaster == enemyFinalMaster){
                            return;
                        }

                        if(uidf::getUIDType(enemyFinalMaster) == UID_MON){
                            m_actorPod->forward(coLoc.uid, {AM_ATTACK, amA});
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
    const auto amAK = mpk.conv<AMAttack>();
    if(m_dead.get()){
        notifyDead(amAK.UID);
    }
    else{
        if(const auto &mr = DBCOM_MAGICRECORD(amAK.damage.magicID); !pathf::inDCCastRange(mr.castRange, X(), Y(), amAK.X, amAK.Y)){
            switch(uidf::getUIDType(amAK.UID)){
                case UID_MON:
                case UID_PLY:
                    {
                        AMMiss amM;
                        std::memset(&amM, 0, sizeof(amM));

                        amM.UID = amAK.UID;
                        m_actorPod->forward(amAK.UID, {AM_MISS, amM});
                        return;
                    }
                default:
                    {
                        return;
                    }
            }
        }

        if(amAK.UID != masterUID()){
            setStandMode(true);
            addOffenderDamage(amAK.UID, amAK.damage);
        }

        dispatchAction(ActionHitted
        {
            .x = X(),
            .y = Y(),
            .direction = Direction(),
            .extParam
            {
                .dog
                {
                    .standMode = m_standMode,
                }
            }
        });
        struckDamage(amAK.damage);
    }
}

DamageNode ServerTaoDog::getAttackDamage(int dc) const
{
    fflassert(to_u32(dc) == DBCOM_MAGICID(u8"神兽_喷火"));
    return MagicDamage
    {
        .magicID = dc,
        .damage = mathf::rand<int>(getMR().mc[0] + m_masterSC[0], getMR().mc[1] + m_masterSC[1]),
        .mcHit = getMR().mcHit,
    };
}
