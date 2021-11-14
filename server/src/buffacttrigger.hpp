#pragma once
#include <memory>
#include "fflerror.hpp"
#include "buffact.hpp"

class BattleObject;
class BaseBuffActTrigger: public BaseBuffAct
{
    private:
        template<uint32_t> friend class BuffActTrigger;

    protected:
        BaseBuffActTrigger(uint32_t argBuffID, uint32_t argBuffActID)
            : BaseBuffAct(argBuffID, argBuffActID)
        {
            fflassert(getBAR().isTrigger());
        }

    public:
        virtual void runOnTrigger(BattleObject *, int) = 0;

    public:
        static BaseBuffActTrigger *createTrigger(uint32_t, uint32_t);
};

template<uint32_t INDEX> class BuffActTrigger: public BaseBuffActTrigger
{
    public:
        BuffActTrigger(uint32_t argBuffID)
            : BaseBuffActTrigger(argBuffID, INDEX)
        {}

    public:
        void runOnTrigger(BattleObject *, int)
        {}
};
