#pragma once
#include <cstdint>
#include <functional>
#include "dbcomid.hpp"
#include "basebuff.hpp"

class PeriodicBuff: public BaseBuff
{
    private:
        uint64_t m_triggeredCount = 0;

    private:
        std::function<void(PeriodicBuff *)> m_trigger;

    public:
        PeriodicBuff(uint32_t, BattleObject *, std::function<void(PeriodicBuff *)>);

    public:
        void update();
};
