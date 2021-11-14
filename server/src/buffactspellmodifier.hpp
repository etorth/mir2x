#pragma once
#include "buffact.hpp"
#include "fflerror.hpp"

class BaseBuffActSpellModifier: public BaseBuffAct
{
    public:
        BaseBuffActSpellModifier(uint32_t, uint32_t);

    public:
        static BaseBuffActSpellModifier *createSpellModifier(uint32_t, uint32_t);
};
