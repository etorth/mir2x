#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerBombSpider final: public Monster
{
    public:
        ServerBombSpider(ServerMap *mapPtr, int argX, int argY, int argDir)
            : Monster(DBCOM_MONSTERID(u8"爆裂蜘蛛"), mapPtr, argX, argY, argDir, 0)
        {}

    protected:
        corof::eval_poller updateCoroFunc() override
        {
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
                    const auto [targetMapID, targetGX, targetGY] = co_await coro_getCOPLoc(targetUID);
                    if((mapID() != targetMapID) || (mathf::LDistance2<int>(X(), Y(), targetGX, targetGY) <= 1)){
                        break; // goDie
                    }
                    else{
                        co_await coro_trackUID(targetUID, {});
                    }
                }
                else{
                    break; // goDie
                }
                co_await corof::async_wait(200);
            }

            goDie();
            co_return true;
        }
};
