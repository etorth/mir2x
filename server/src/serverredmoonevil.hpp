#pragma once
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerRedMoonEvil final: public Monster
{
    public:
        ServerRedMoonEvil(uint64_t argMapUID, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"赤月恶魔"), argMapUID, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        corof::eval_poller<> updateCoroFunc() override;

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

    protected:
        bool canMove(bool) const override
        {
            return false;
        }
};
