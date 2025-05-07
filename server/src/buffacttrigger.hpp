#pragma once
#include <cstdint>
#include <cstddef>
#include "dbcomid.hpp"
#include "buffact.hpp"
#include "fflerror.hpp"

class BaseBuff;
class BaseBuffActTrigger: public BaseBuffAct
{
    private:
        template<uint32_t> friend class BuffActTrigger;

    protected:
        BaseBuffActTrigger(BaseBuff *, size_t);

    public:
        virtual void runOnTrigger(int) = 0;

    public:
        static BaseBuffActTrigger *createTrigger(BaseBuff *, size_t);
};
