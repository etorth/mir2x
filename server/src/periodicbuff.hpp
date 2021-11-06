#pragma once
#include <cstdint>
#include <functional>
#include "dbcomid.hpp"
#include "basebuff.hpp"

class PeriodicBuff: public BaseBuff
{
    private:
        size_t m_triggerCount = 0;

    private:
        std::function<void(PeriodicBuff *)> m_trigger;

    public:
        PeriodicBuff(uint32_t, BattleObject *, std::function<void(PeriodicBuff *)>);

    public:
        void runOnUpdate() override;
};
