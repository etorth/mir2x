#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerBombSpider final: public Monster
{
    public:
        ServerBombSpider(uint64_t argMapUID, int argX, int argY, int argDir)
            : Monster(DBCOM_MONSTERID(u8"爆裂蜘蛛"), argMapUID, argX, argY, argDir, 0)
        {}

    protected:
        corof::awaitable<> runAICoro() override
        {
            uint64_t targetUID = 0;
            while(!m_sdHealth.dead()){
                if(targetUID && !(co_await validTarget(targetUID))){
                    targetUID = 0;
                }

                if(!targetUID){
                    targetUID = co_await pickTarget();
                }

                if(!targetUID){
                    goDie();
                    break;
                }

                const auto coLocOpt = co_await getCOLocation(targetUID);
                if(!coLocOpt.has_value()){
                    continue;
                }

                const auto &coLoc = coLocOpt.value();
                if((mapID() != coLoc.mapUID) || (mathf::LDistance2<int>(X(), Y(), coLoc.x, coLoc.y) <= 1)){
                    goDie();
                    break;
                }

                co_await trackUID(targetUID, {});
                co_await asyncWait(200);
            }
        }
};
