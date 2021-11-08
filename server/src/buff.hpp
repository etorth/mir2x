#pragma once
#include <cmath>
#include <tuple>
#include <vector>
#include "uidf.hpp"
#include "buffrecord.hpp"

class BattleObject;
class BaseBuffTrigger;
class BaseBuffModifier;

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

    protected:
        std::vector<std::unique_ptr<BaseBuffModifier>> m_modList;
        std::vector<std::tuple<long, std::unique_ptr<BaseBuffTrigger>>> m_tgrList;

    public:
        BaseBuff(uint32_t, BattleObject *);

    public:
        virtual ~BaseBuff();

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
        virtual void runOnDone()
        {
        }

    public:
        virtual void runOnUpdate();

    public:
        void runOnTrigger(int);
};
