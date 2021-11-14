#pragma once
#include <memory>
#include "fflerror.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"

class BattleObject;
class BaseBuffAct
{
    protected:
        const uint32_t m_buffID;

    protected:
        const uint32_t m_buffActID;

    public:
        BaseBuffAct(uint32_t, uint32_t);

    public:
        virtual ~BaseBuffAct() = default;

    public:
        static BaseBuffAct *createBuffAct(BattleObject *, uint32_t, uint32_t);

    public:
        uint32_t buffID() const
        {
            return m_buffID;
        }

        uint32_t buffActID() const
        {
            return m_buffActID;
        }

    public:
        const auto &getBR() const
        {
            return DBCOM_BUFFRECORD(buffID());
        }

        const auto &getBAR() const
        {
            return DBCOM_BUFFACTRECORD(buffActID());
        }

        const auto &getBAREF() const
        {
            for(const auto &baref: getBR().actList){
                if(baref.isBuffActRef(getBAR().name)){
                    return baref;
                }
            }
            throw fflvalue(getBR().name, getBAR().name);
        }
};
