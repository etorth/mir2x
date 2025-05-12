#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerAntHealer final: public Monster
{
    public:
        ServerAntHealer(uint64_t argMapUID, int argX, int argY, int argDir, uint64_t masterUID)
            : Monster(DBCOM_MONSTERID(u8"蚂蚁道士"), argMapUID, argX, argY, argDir, masterUID)
        {}

    protected:
        corof::awaitable<> runAICoro() override;

    protected:
        void sendHeal(uint64_t);
};
