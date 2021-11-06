#include "fflerror.hpp"
#include "servershipwrecklord.hpp"

corof::eval_poller ServerShipwreckLord::updateCoroFunc()
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

    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await coro_pickTarget();
        }

        if(targetUID){
            if(co_await coro_inDCCastRange(targetUID, phyMR.castRange)){
                co_await coro_attackUID(targetUID, phyDC);
            }
            else if(lastPushTime.diff_sec() >= pushCoolTime && (co_await coro_inDCCastRange(targetUID, pushMR.castRange))){
                co_await coro_attackUID(targetUID, pushDC);
                lastPushTime.reset();
            }
            else{
                const auto [targetMapID, targetGX, targetGY] = co_await coro_getCOGLoc(targetUID);
                if(mapID() == targetMapID){
                    if(mathf::LDistance2<int>(targetGX, targetGY, X(), Y()) < followDistance * followDistance){
                        co_await coro_trackUID(targetUID, {});
                    }
                    else{
                        co_await coro_jumpUID(targetUID);
                    }
                }
                else{
                    m_inViewCOList.erase(targetUID);
                    targetUID = 0;
                }
            }
        }
        co_await corof::async_wait(1000);
    }

    goDie();
    co_return true;
}
