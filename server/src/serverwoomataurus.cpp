#include "fflerror.hpp"
#include "servermsg.hpp"
#include "friendtype.hpp"
#include "serverwoomataurus.hpp"
#include "serverargparser.hpp"

extern ServerArgParser *g_serverArgParser;
corof::eval_poller<> ServerWoomaTaurus::updateCoroFunc()
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
    while(m_sdHealth.hp > 0){
        if(targetUID && !(co_await coro_validTarget(targetUID))){
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
                if(co_await coro_attackUID(targetUID, thunderMagicID)){
                    for(const auto &[uid, coLoc]: m_inViewCOList){
                        if(uid == targetUID){
                            continue;
                        }

                        switch(co_await coro_checkFriend(uid)){
                            case FT_ENEMY:
                                {
                                    sendThunderBolt(uid);
                                    break;
                                }
                            default:
                                {
                                    break;
                                }
                        }
                    }
                }
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
}

void ServerWoomaTaurus::sendThunderBolt(uint64_t uid)
{
    SMCastMagic smFM;
    std::memset(&smFM, 0, sizeof(smFM));

    smFM.UID    = UID();
    smFM.mapID  = mapID();
    smFM.Magic  = DBCOM_MAGICID(u8"沃玛教主_雷电术");
    smFM.Speed  = MagicSpeed();
    smFM.X      = X();
    smFM.Y      = Y();
    smFM.AimUID = uid;

    dispatchInViewCONetPackage(SM_CASTMAGIC, smFM);
    addDelay(300, [uid, this]()
    {
        dispatchAttackDamage(uid, DBCOM_MAGICID(u8"沃玛教主_雷电术"), 0);
    });
}
