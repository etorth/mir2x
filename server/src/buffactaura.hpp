#pragma once
#include <memory>
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "buffact.hpp"

class BattleObject;
class BaseBuffActAura: public BaseBuffAct
{
    protected:
        BaseBuffActAura(uint32_t, uint32_t);

    protected:
        ~BaseBuffActAura();

    public:
        static BaseBuffActAura *createAura(uint32_t, uint32_t);
};
