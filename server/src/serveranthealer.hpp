#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerAntHealer final: public Monster
{
    public:
        ServerAntHealer(ServerMap *mapPtr, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"蚂蚁道士"), mapPtr, argX, argY, argDir, masterUID)
        {}

    protected:
        corof::eval_poller updateCoroFunc() override;

    protected:
        void sendHeal(uint64_t);
};
