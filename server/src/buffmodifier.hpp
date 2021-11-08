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
        const int m_type;

    protected:
        std::pair<SDTaggedValMap *, int> m_sdTaggedVal;

    public:
        BaseBuffModifier(BattleObject *, int, int);

    public:
        virtual ~BaseBuffModifier()
        {
            if(m_sdTaggedVal.first){
                m_sdTaggedVal.first->erase(m_sdTaggedVal.second);
            }
        }

    public:
        int type() const
        {
            return m_type;
        }
};
