#pragma once
#include "uidf.hpp"
#include "raiitimer.hpp"
#include "buffrecord.hpp"

class BattleObject;
class BaseBuff
{
    protected:
        uint32_t m_id;

    protected:
        const BuffRecord &m_br;

    protected:
        BattleObject *m_bo;

    protected:
        hres_timer m_timer;

    public:
        BaseBuff(uint32_t, BattleObject *);

    public:
        virtual ~BaseBuff() = default;

    public:
        uint32_t id() const
        {
            return m_id;
        }

    public:
        bool expired() const
        {
            if(m_br.time <= 0){
                return false;
            }
            return m_timer.diff_msec() >= m_br.time;
        }
};
