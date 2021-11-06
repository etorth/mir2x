#pragma once
#include <cmath>
#include "uidf.hpp"
#include "buffrecord.hpp"

class BattleObject;
class BaseBuff
{
    protected:
        const uint32_t m_id;

    protected:
        const BuffRecord &m_br;

    protected:
        BattleObject * const m_bo;

    protected:
        double m_accuTime = 0.0;

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
        virtual bool done() const
        {
            if(m_br.time <= 0){
                return false;
            }
            return std::lround(m_accuTime) >= m_br.time;
        }

    public:
        virtual bool update(double ms)
        {
            m_accuTime += ms;
            runOnUpdate();

            if(done()){
                runOnDone();
                return true;
            }
            return false;
        }

    public:
        virtual void runOnDone  (){}
        virtual void runOnUpdate(){}
};
