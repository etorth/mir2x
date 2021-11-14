#pragma once
#include "mathf.hpp"
#include "dbcomid.hpp"
#include "buffact.hpp"

class BaseBuffActAttackModifier: public BaseBuffAct
{
    protected:
        mathf::ARBVar m_roller;

    public:
        BaseBuffActAttackModifier(uint32_t, uint32_t);

    public:
        static BaseBuffActAttackModifier *createAttackModifier(uint32_t);

    public:
        uint32_t rollBuff();
        uint32_t rollModifier();
};
