#pragma once
#include <cstdint>
#include <cstddef>
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"
#include "buffact.hpp"
#include "fflerror.hpp"

class BaseBuff;
class BaseBuffActTrigger: public BaseBuffAct
{
    private:
        template<uint32_t> friend class BuffActTrigger;

    protected:
        BaseBuffActTrigger(BaseBuff *argBuff, size_t argBuffActOff)
            : BaseBuffAct(argBuff, argBuffActOff)
        {
            fflassert(getBAR().isTrigger());
        }

    public:
        virtual void runOnTrigger(int) = 0;

    public:
        static BaseBuffActTrigger *createTrigger(BaseBuff *, size_t);
};
