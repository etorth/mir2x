#include "fflerror.hpp"
#include "servershipwrecklord.hpp"

corof::awaitable<> ServerShipwreckLord::runAICoro()
{
    const auto  phyDC = DBCOM_MAGICID(u8"物理攻击");
    const auto pushDC = DBCOM_MAGICID(u8"霸王教主_野蛮冲撞");

    fflassert( phyDC);
    fflassert(pushDC);

    const auto & phyMR = DBCOM_MAGICRECORD( phyDC);
    const auto &pushMR = DBCOM_MAGICRECORD(pushDC);

    fflassert( phyMR);
    fflassert(pushMR);

    uint64_t targetUID = 0;
    constexpr int followDistance = 5;

    hres_timer lastPushTime;
    constexpr uint64_t pushCoolTime = 5;

    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            if(co_await inDCCastRange(targetUID, phyMR.castRange)){
                co_await attackUID(targetUID, phyDC);
            }
            else if(lastPushTime.diff_sec() >= pushCoolTime && (co_await inDCCastRange(targetUID, pushMR.castRange))){
                co_await attackUID(targetUID, pushDC);
                lastPushTime.reset();
            }
            else{
                const auto coLocOpt = co_await getCOLocation(targetUID);
                if(!coLocOpt.has_value()){
                    continue;
                }

                const auto &coLoc = coLocOpt.value();
                if(mapUID() == coLoc.mapUID){
                    if(mathf::LDistance2<int>(coLoc.x, coLoc.y, X(), Y()) < followDistance * followDistance){
                        co_await trackUID(targetUID, {});
                    }
                    else{
                        co_await jumpUID(targetUID);
                    }
                }
                else{
                    m_inViewCOList.erase(targetUID);
                    targetUID = 0;
                }
            }
        }

        co_await asyncIdleWait(1000);
    }
}
