#pragma once
#include <cstddef>
#include "buffact.hpp"

class BaseBuff;
class BaseBuffActController: public BaseBuffAct
{
    private:
        friend class BaseBuffAct;

    protected:
        BaseBuffActController(BaseBuff *, size_t);

    protected:
        static BaseBuffActController *createController(BaseBuff *, size_t);
};
