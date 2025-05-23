#include "servereviltentacle.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::awaitable<> ServerEvilTentacle::runAICoro()
{
    uint64_t targetUID = 0;
    while(m_sdHealth.hp > 0){
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
                if(mathf::CDistance<int>(coLoc.x, coLoc.y, X(), Y()) <= 3){
                    co_await trackAttackUID(targetUID);
                }
                else{
                    co_await jumpAttackUID(targetUID);
                }
            }
            else{
                targetUID = 0;
                m_inViewCOList.erase(targetUID);
            }
        }
        else{
            co_await randomMove();
        }

        if(g_serverArgParser->sharedConfig().forceMonsterRandomMove || hasPlayerNeighbor()){
            co_await asyncWait(1000);
        }
        else{
            m_idleWaitToken.emplace();
            co_await asyncWait(0, std::addressof(m_idleWaitToken.value())); // infinite wait till cancel
        }
    }

    goDie();
}
