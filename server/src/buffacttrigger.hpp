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

    private:
        long m_tpsCount = 0;

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

    public:
        void checkTimedTrigger(); // check if need trigger BATGR_TIME
};
