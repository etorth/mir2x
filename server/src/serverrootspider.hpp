#pragma once
#include <unordered_set>
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerRootSpider final: public Monster
{
    private:
        const size_t m_maxBatCount = 20;
        std::unordered_set<uint64_t> m_childUIDList;

    public:
        ServerRootSpider(uint64_t argMapUID, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"幻影蜘蛛"), argMapUID, argX, argY, DIR_BEGIN + mathf::rand() % 3, 0)
        {}

    protected:
        corof::awaitable<> addBombSpider();

    protected:
        corof::awaitable<> runAICoro() override;

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
        bool canMove(bool) const override
        {
            return false;
        }
};
