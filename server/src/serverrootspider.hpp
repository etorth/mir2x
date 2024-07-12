#pragma once
#include <unordered_set>
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerRootSpider final: public Monster
{
    private:
        const size_t m_maxBatCount = 20;
        std::unordered_set<uint64_t> m_batUIDList;

    public:
        ServerRootSpider(ServerMap *mapPtr, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"幻影蜘蛛"), mapPtr, argX, argY, DIR_BEGIN + std::rand() % 3, 0)
        {}

    protected:
        void addBombSpider();

    protected:
        corof::eval_poller updateCoroFunc() override;

    protected:
        ActionNode makeActionStand() const override
        {
            return ActionStand
            {
                .direction = Direction(),
                .x = X(),
                .y = Y(),
            };
        }

    protected:
        bool canMove() const override
        {
            return false;
        }
};
