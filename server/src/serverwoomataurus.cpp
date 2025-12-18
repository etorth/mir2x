#include "fflerror.hpp"
#include "servermsg.hpp"
#include "friendtype.hpp"
#include "serverwoomataurus.hpp"

corof::awaitable<> ServerWoomaTaurus::runAICoro()
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
    while(!m_sdHealth.dead()){
        if(targetUID && !(co_await validTarget(targetUID))){
            targetUID = 0;
        }

        if(!targetUID){
            targetUID = co_await pickTarget();
        }

        if(targetUID){
            if(co_await inDCCastRange(targetUID, lightMR.castRange)){
                co_await attackUID(targetUID, lightMagicID);
            }
            else if(co_await inDCCastRange(targetUID, thunderMR.castRange)){
                if(co_await attackUID(targetUID, thunderMagicID)){
                    for(const auto &[uid, coLoc]: m_inViewCOList){
                        if(uid == targetUID){
                            continue;
                        }

                        switch(co_await checkFriend(uid)){
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
                co_await trackUID(targetUID, {});
            }
        }
        else if(masterUID()){
            co_await followMaster();
        }
        else{
            co_await randomMove();
        }

        co_await asyncIdleWait(1000);
    }
}

void ServerWoomaTaurus::sendThunderBolt(uint64_t uid)
{
    SMCastMagic smFM;
    std::memset(&smFM, 0, sizeof(smFM));

    smFM.UID    = UID();
    smFM.mapUID = mapUID();
    smFM.Magic  = DBCOM_MAGICID(u8"沃玛教主_雷电术");
    smFM.Speed  = MagicSpeed();
    smFM.X      = X();
    smFM.Y      = Y();
    smFM.AimUID = uid;

    dispatchInViewCONetPackage(SM_CASTMAGIC, smFM);
    addDelay(300, [uid, this](bool)
    {
        dispatchAttackDamage(uid, DBCOM_MAGICID(u8"沃玛教主_雷电术"), 0);
    });
}
