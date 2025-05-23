#include <optional>
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "raiitimer.hpp"
#include "serverrebornzombie.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::awaitable<> ServerRebornZombie::runAICoro()
{
    uint64_t targetUID = 0;
    std::optional<uint64_t> idleTime;

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
                idleTime.reset();
                setStandMode(true);
                if(mathf::CDistance<int>(coLoc.x, coLoc.y, X(), Y()) <= 1){
                    co_await attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
                }
                else{
                    co_await trackUID(targetUID, DBCOM_MAGICRECORD(u8"物理攻击").castRange);
                }
            }
            else{
                m_inViewCOList.erase(targetUID);
                targetUID = 0;
            }
        }
        else if(g_serverArgParser->sharedConfig().forceMonsterRandomMove || hasPlayerNeighbor()){
            if(m_standMode){
                co_await randomMove();
            }
        }
        else{
            if(!idleTime.has_value()){
                idleTime = hres_tstamp().to_sec();
            }
            else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
                setStandMode(false);
            }
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
