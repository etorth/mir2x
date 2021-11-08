#pragma once
#include <memory>
#include <utility>
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "serdesmsg.hpp"

class BattleObject;
class BaseBuffModifier
{
    protected:
        BattleObject * const m_bo;

    protected:
        const int m_type;

    protected:
        const int m_tag;

    public:
        BaseBuffModifier(BattleObject *, int, int);

    public:
        virtual ~BaseBuffModifier();

    public:
        int type() const
        {
            return m_type;
        }
};
