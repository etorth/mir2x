#include <optional>
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "raiitimer.hpp"
#include "serverevilcentipede.hpp"

corof::awaitable<> ServerEvilCentipede::runAICoro()
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
            }
            else{
                idleTime = hres_tstamp().to_sec();
                m_inViewCOList.erase(targetUID);
                targetUID = 0;
            }
        }
        else if(!idleTime.has_value()){
            idleTime = hres_tstamp().to_sec();
        }
        else if(hres_tstamp().to_sec() - idleTime.value() > 30ULL){
            setStandMode(false);
        }

        co_await asyncIdleWait(1000);
    }
}
