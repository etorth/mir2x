#pragma once
#include <vector>
#include <memory>
#include "dbcomid.hpp"
#include "fflerror.hpp"
#include "buffact.hpp"

class BaseBuff;
class BaseBuffActAura: public BaseBuffAct
{
    private:
        friend class BaseBuffAct;

    protected:
        const uint32_t m_auraBuffID;

    protected:
        BaseBuffActAura(BaseBuff *, size_t);

    public:
        void dispatch();
        void transmit(uint64_t);

    public:
        uint32_t getAuraBuffID() const
        {
            return m_auraBuffID;
        }

    protected:
        static BaseBuffActAura *createAura(BaseBuff *, size_t);
};
