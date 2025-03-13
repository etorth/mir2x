#pragma once
#include <vector>
#include <memory>
#include <cstddef>
#include "fflerror.hpp"
#include "dbcomid.hpp"

class BaseBuff;
class BaseBuffAct
{
    private:
        friend class BaseBuff;

    protected:
        BaseBuff * const m_buff;

    protected:
        const size_t m_actOff;

    protected:
        const uint32_t m_id;

    public:
        BaseBuffAct(BaseBuff *, size_t);

    public:
        virtual ~BaseBuffAct() = default;

    protected:
        static BaseBuffAct *createBuffAct(BaseBuff *, size_t);

    public:
        uint32_t id() const
        {
            return m_id;
        }

    public:
        BaseBuff * getBuff()
        {
            return m_buff;
        }

        const BaseBuff *getBuff() const
        {
            return m_buff;
        }

    public:
        virtual bool done() const; // if duration has passed

    public:
        const BuffRecord & getBR() const;

    public:
        const BuffActRecord & getBAR() const;

    public:
        const BuffRecord::BuffActRecordRef & getBAREF() const;
};
