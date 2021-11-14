#pragma once
#include <cstdint>
#include <functional>
#include "buffact.hpp"

class BattleObject;
class BaseBuffActAttributeModifier: public BaseBuffAct
{
    protected:
        BattleObject * const m_bo;

    protected:
        const std::function<void()> m_onDone;

    protected:
        BaseBuffActAttributeModifier(BattleObject *, uint32_t, uint32_t);

    public:
        ~BaseBuffActAttributeModifier() override;

    public:
        static BaseBuffActAttributeModifier *createAttributeModifier(BattleObject *, uint32_t, uint32_t);
};
