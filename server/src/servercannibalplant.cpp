#include <optional>
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "raiitimer.hpp"
#include "servercannibalplant.hpp"

corof::awaitable<> ServerCannibalPlant::runAICoro()
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
            co_await attackUID(targetUID, DBCOM_MAGICID(u8"物理攻击"));
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
