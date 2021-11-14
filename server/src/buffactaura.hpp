#pragma once
#include <vector>
#include <memory>
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "buffact.hpp"

class BattleObject;
class BaseBuffActAura: public BaseBuffAct
{
    private:
        BattleObject * const m_bo;

    protected:
        BaseBuffActAura(BattleObject *, uint32_t, uint32_t);

    protected:
        ~BaseBuffActAura();

    public:
        static BaseBuffActAura *createAura(BattleObject *, uint32_t, uint32_t);

    public:
        void transmit();

    private:
        void transmitHelper(std::vector<uint64_t>);

    public:
        uint32_t getBuffID() const
        {
            return DBCOM_BUFFID(getBAR().aura.buff);
        }
};
