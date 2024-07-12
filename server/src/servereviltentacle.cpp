#include "servereviltentacle.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::eval_poller ServerEvilTentacle::updateCoroFunc()
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
                if(mathf::CDistance<int>(targetX, targetY, X(), Y()) <= 3){
                    co_await coro_trackAttackUID(targetUID);
                }
                else{
                    co_await coro_jumpAttackUID(targetUID);
                }
            }
            else{
                targetUID = 0;
                m_inViewCOList.erase(targetUID);
            }
        }
        else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
            co_await coro_randomMove();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}
