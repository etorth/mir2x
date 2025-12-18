#include "mathf.hpp"
#include "pathf.hpp"
#include "fflerror.hpp"
#include "raiitimer.hpp"
#include "friendtype.hpp"
#include "servertaodog.hpp"

corof::awaitable<> ServerTaoDog::runAICoro()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            idleTime.reset();
            setStandMode(true);
            co_await trackAttackUID(targetUID);
        }

        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }

            if(co_await queryDead(masterUID())){
                goDie();
            }
            else{
                co_await followMaster();
            }
        }

        co_await asyncWait(1000);
    }
}

corof::awaitable<bool> ServerTaoDog::attackUID(uint64_t targetUID, int dcType)
{
    fflassert(to_u32(dcType) == DBCOM_MAGICID(u8"神兽_喷火"));
    if(!canAttack(true)){
        co_return false;
    }

    if(!dcValid(dcType, true)){
        co_return false;
    }

    m_attackLock = true;
    const auto attckLockSg = sgf::guard([this]() noexcept { m_attackLock = false; });

    const auto coLocOpt = co_await getCOLocation(targetUID);
    if(!coLocOpt.has_value()){
        co_return false;
    }

    const auto & coLoc = coLocOpt.value();
    const auto &mr = DBCOM_MAGICRECORD(dcType);

    if(!pathf::inDCCastRange(mr.castRange, X(), Y(), coLoc.x, coLoc.y)){
        co_return false;
    }

    if(const auto newDir = pathf::getOffDir(X(), Y(), coLoc.x, coLoc.y); pathf::dirValid(newDir)){
        m_direction = newDir;
    }

    if(!canAttack(false)){
        co_return false;
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

    std::unordered_set<uint64_t> uidList;
    foreachInViewCO([&mr, &coLoc, &uidList, this](const COLocation &inViewCOLoc)
    {
        for(int d = 0; d <= mr.castRange.distance; ++d){
            if(const auto [attackGX, attackGY] = pathf::getFrontGLoc(X(), Y(), Direction(), d); (inViewCOLoc.mapUID == mapUID()) && (inViewCOLoc.x == attackGX) && (inViewCOLoc.y == attackGY)){
                uidList.insert(inViewCOLoc.uid);
            }
        }
    });

    uidList.insert(targetUID);
    addDelay(550, [dcType, modifierID, uidList, thisptr = this](this auto, bool) -> corof::awaitable<>
    {
        AMAttack amA;
        std::memset(&amA, 0, sizeof(amA));

        amA.UID = thisptr->UID();
        amA.mapUID = thisptr->mapUID();

        amA.X = thisptr->X();
        amA.Y = thisptr->Y();
        amA.damage = thisptr->getAttackDamage(dcType, modifierID);

        for(const auto uid: uidList){
            switch(co_await thisptr->checkFriend(uid)){
                case FT_ENEMY:
                    {
                        thisptr->m_actorPod->post(uid, {AM_ATTACK, amA});
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
    });
    co_return true;
}

corof::awaitable<> ServerTaoDog::onAMAttack(const ActorMsgPack &mpk)
{
    const auto amA = mpk.conv<AMAttack>();
    if(amA.UID == UID()){
        co_return;
    }

    if(m_sdHealth.dead()){
        notifyDead(amA.UID);
        co_return;
    }

    setStandMode(true);
    co_await Monster::onAMAttack(mpk);
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
