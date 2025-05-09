#pragma once
#include <unordered_set>
#include "dbcomid.hpp"
#include "monster.hpp"

class ServerBugbatMaggot final: public Monster
{
    private:
        const size_t m_maxBatCount = 20;
        std::unordered_set<uint64_t> m_batUIDList;

    public:
        ServerBugbatMaggot(uint64_t argMapUID, int argX, int argY)
            : Monster(DBCOM_MONSTERID(u8"角蝇"), argMapUID, argX, argY, DIR_BEGIN, 0)
        {}

    protected:
        void addBat();

    protected:
        corof::awaitable<> runAICoro() override;

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
