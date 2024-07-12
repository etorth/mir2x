#include "uidf.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "friendtype.hpp"
#include "monoserver.hpp"
#include "serverguard.hpp"

extern MonoServer *g_monoServer;
ServerGuard::ServerGuard(uint32_t monID, ServerMap *mapPtr, int argX, int argY, int argDir)
    : Monster(monID, mapPtr, argX, argY, argDir, 0)
    , m_standX(argX)
    , m_standY(argY)
    , m_standDirection(argDir)
{
    fflassert(uidf::isGuardMode(UID()));
}

corof::eval_poller ServerGuard::updateCoroFunc()
{
    uint64_t targetUID = 0;
    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            const auto [targetMapID, targetX, targetY] = co_await coro_getCOGLoc(targetUID);
            if(inView(targetMapID, targetX, targetY)){
                if(mathf::CDistance<int>(targetX, targetY, X(), Y()) == 1){
                    co_await coro_attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
                }
                else{
                    co_await coro_jumpAttackUID(targetUID);
                }
            }
            else{
                m_inViewCOList.erase(targetUID);
                targetUID = 0;
            }
        }

        if(!targetUID){
            if(X() == m_standX && Y() == m_standY){
                if(Direction() != m_standDirection){
                    m_direction = m_standDirection;
                    dispatchAction(makeActionStand());
                }
            }
            else{
                co_await coro_jumpGLoc(m_standX, m_standY, m_standDirection);
            }
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}

void ServerGuard::checkFriend(uint64_t targetUID, std::function<void(int)> fnOp)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
            {
                if(uidf::isGuardMode(targetUID)){
                    if(fnOp){
                        fnOp(FT_FRIEND);
                    }
                    return;
                }

                if(uidf::isNeutralMode(targetUID)){
                    if(fnOp){
                        fnOp(FT_NEUTRAL);
                    }
                    return;
                }

                if(fnOp){
                    fnOp(FT_ENEMY);
                }
                return;
            }
        case UID_PLY:
            {
                if(fnOp){
                    fnOp(FT_NEUTRAL);
                }
                return;
            }
        default:
            {
                if(fnOp){
                    fnOp(FT_FRIEND);
                }
                return;
            }
    }
}

bool ServerGuard::canMove() const
{
    return BattleObject::canMove();
}

bool ServerGuard::canAttack() const
{
    if(!BattleObject::canAttack()){
        return false;
    }

    if(m_lastAction != ACTION_ATTACK){
        return true;
    }
    return g_monoServer->getCurrTick() >= m_lastActionTime.at(ACTION_ATTACK) + getMR().attackWait;
}
