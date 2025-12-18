#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include "buffact.hpp"

class BaseBuff;
class BaseBuffActAttributeModifier: public BaseBuffAct
{
    private:
        friend class BaseBuffAct;

    protected:
        const std::function<void()> m_onDone;

    protected:
        BaseBuffActAttributeModifier(BaseBuff *, size_t);

    public:
        ~BaseBuffActAttributeModifier() override;

    protected:
        static BaseBuffActAttributeModifier *createAttributeModifier(BaseBuff *, size_t);
};
