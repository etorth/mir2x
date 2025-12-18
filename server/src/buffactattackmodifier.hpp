#pragma once
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "buffact.hpp"

class BaseBuff;
class BaseBuffActAttackModifier: public BaseBuffAct
{
    private:
        friend class BaseBuffAct;

    protected:
        mathf::ARBVar m_roller;

    protected:
        BaseBuffActAttackModifier(BaseBuff *, size_t);

    protected:
        static BaseBuffActAttackModifier *createAttackModifier(BaseBuff *, size_t);

    public:
        uint32_t rollBuff();
        uint32_t rollModifier();
};
