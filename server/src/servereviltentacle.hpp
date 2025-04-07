#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerEvilTentacle: public Monster
{
    public:
        ServerEvilTentacle(uint32_t monID, uint64_t argMapUID, int argX, int argY, int argDir)
            : Monster(monID, argMapUID, argX, argY, argDir, 0)
        {
            fflassert(isMonster(u8"触角神魔") || isMonster(u8"爆毒神魔"));
        }

    protected:
        corof::eval_poller<> updateCoroFunc() override;
};
