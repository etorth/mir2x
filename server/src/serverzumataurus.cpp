#include "pathf.hpp"
#include "serverzumataurus.hpp"

corof::eval_poller ServerZumaTaurus::updateCoroFunc()
{
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
            setStandMode(true);
            co_await coro_trackAttackUID(targetUID);
        }

        else if(masterUID()){
            setStandMode(true);
            co_await coro_followMaster();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

void ServerZumaTaurus::onAMAttack(const ActorMsgPack &mpk)
{
    if(m_standMode){
        Monster::onAMAttack(mpk);
    }
}

void ServerZumaTaurus::attackUID(uint64_t targetUID, int dcType, std::function<void()> onOK, std::function<void()> onError)
{
    fflassert(to_u32(dcType) == DBCOM_MAGICID(u8"祖玛教主_火墙"));
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

        addDelay(550, [dcType, coLoc, this]()
        {
            AMCastFireWall amCFW;
            std::memset(&amCFW, 0, sizeof(amCFW));

            amCFW.minDC = 5;
            amCFW.maxDC = 9;

            amCFW.duration = 5 * 1000;
            amCFW.dps      = 3;

            for(const int dir: {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT}){
                if(dir == DIR_NONE){
                    amCFW.x = coLoc.x;
                    amCFW.y = coLoc.y;
                }
                else{
                    std::tie(amCFW.x, amCFW.y) = pathf::getFrontGLoc(coLoc.x, coLoc.y, dir, 1);
                }

                if(m_map->groundValid(amCFW.x, amCFW.y)){
                    m_actorPod->forward(m_map->UID(), {AM_CASTFIREWALL, amCFW});
                }
            }
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
