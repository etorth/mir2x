#pragma once
#include <memory>
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "buffact.hpp"

class BattleObject;
class BaseBuffActController: public BaseBuffAct
{
    protected:
        BaseBuffActController(uint32_t, uint32_t);

    public:
        static BaseBuffActController *createController(uint32_t, uint32_t);
};
