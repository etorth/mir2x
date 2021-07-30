#include "fflerror.hpp"
#include "woomataurus.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::long_jmper WoomaTaurus::updateCoroFunc()
{
    const auto   lightMagicID = DBCOM_MAGICID(u8"沃玛教主_电光");
    const auto thunderMagicID = DBCOM_MAGICID(u8"沃玛教主_雷电术");

    fflassert(  lightMagicID);
    fflassert(thunderMagicID);

    const auto &  lightMR = DBCOM_MAGICRECORD(  lightMagicID);
    const auto &thunderMR = DBCOM_MAGICRECORD(thunderMagicID);

    fflassert(  lightMR);
    fflassert(thunderMR);

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
            if(co_await coro_inDCCastRange(targetUID, lightMR.castRange)){
                co_await coro_attackUID(targetUID, lightMagicID);
            }
            else if(co_await coro_inDCCastRange(targetUID, thunderMR.castRange)){
                co_await coro_attackUID(targetUID, thunderMagicID);
            }
            else{
                co_await coro_trackUID(targetUID, {});
            }
        }
        else if(masterUID()){
            co_await coro_followMaster();
        }
        else if(g_serverArgParser->forceMonsterRandomMove || hasPlayerNeighbor()){
            co_await coro_randomMove();
        }
        co_await corof::async_wait(200);
    }

    goDie();
    co_return true;
}
