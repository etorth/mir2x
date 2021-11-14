#pragma once
#include <memory>
#include <functional>
#include "dbcomrecord.hpp"

class BattleObject;
class BaseBuffActAttributeModifier: public BaseBuffAct
{
    protected:
        BattleObject * const m_bo;

    protected:
        const std::function<void()> m_onDone;

    public:
        BaseBuffActAttributeModifier(BattleObject *, uint32_t, uint32_t);

    public:
        ~BaseBuffActAttributeModifier() override;
};
