#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerMonsterTree final: public Monster
{
    public:
        ServerMonsterTree(uint32_t monsterID, const ServerMap *mapPtr, int argX, int argY)
            : Monster(monsterID, mapPtr, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::eval_poller updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .direction = DIR_BEGIN,
                .x = X(),
                .y = Y(),
            };
        }
};
