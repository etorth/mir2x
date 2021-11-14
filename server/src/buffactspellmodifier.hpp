#pragma once
#include "fflerror.hpp"
#include "buffact.hpp"

class BaseBuff;
class BaseBuffActSpellModifier: public BaseBuffAct
{
    protected:
        BaseBuffActSpellModifier(BaseBuff *, size_t);

    protected:
        static BaseBuffActSpellModifier *createSpellModifier(BaseBuff *, size_t);
};
