#include "uidf.hpp"
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "friendtype.hpp"
#include "server.hpp"
#include "serverguard.hpp"

extern Server *g_server;
ServerGuard::ServerGuard(const SDInitGuard &sdIG)
    : Monster
      {
          sdIG.monsterID,
          sdIG.mapUID,
          sdIG.x,
          sdIG.y,
          sdIG.direction,
          0,
      }
    , m_standX(sdIG.x)
    , m_standY(sdIG.y)
    , m_standDirection(sdIG.direction)
{
    fflassert(uidf::isGuardMode(UID()));
}

corof::awaitable<> ServerGuard::runAICoro()
{
    uint64_t targetUID = 0;
    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            const auto coLocOpt = co_await getCOLocation(targetUID);
            if(!coLocOpt.has_value()){
                continue;
            }

            const auto &coLoc = coLocOpt.value();
            if(inView(coLoc.mapUID, coLoc.x, coLoc.y)){
                if(mathf::CDistance<int>(coLoc.x, coLoc.y, X(), Y()) == 1){
                    co_await attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
                }
                else{
                    co_await jumpAttackUID(targetUID);
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
                co_await requestJump(m_standX, m_standY, m_standDirection);
            }
        }

        co_await asyncIdleWait(1000);
    }

    goDie();
}

corof::awaitable<int> ServerGuard::checkFriend(uint64_t targetUID)
{
    fflassert(targetUID);
    fflassert(targetUID != UID());

    switch(uidf::getUIDType(targetUID)){
        case UID_MON:
            {
                if(uidf::isGuardMode(targetUID)){
                    co_return FT_FRIEND;
                }

                if(uidf::isNeutralMode(targetUID)){
                    co_return FT_NEUTRAL;
                }

                co_return FT_ENEMY;
            }
        case UID_PLY:
            {
                co_return FT_NEUTRAL;
            }
        default:
            {
                co_return FT_FRIEND;
            }
    }
}

bool ServerGuard::canMove(bool checkMoveLock) const
{
    return BattleObject::canMove(checkMoveLock);
}

bool ServerGuard::canAttack(bool checkAttackLock) const
{
    if(!BattleObject::canAttack(checkAttackLock)){
        return false;
    }

    if(m_lastAction != ACTION_ATTACK){
        return true;
    }
    return g_server->getCurrTick() >= m_lastActionTime.at(ACTION_ATTACK) + getMR().attackWait;
}
