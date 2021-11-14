#pragma once
#include <cmath>
#include <tuple>
#include <vector>
#include <memory>
#include <cstdint>
#include "uidf.hpp"
#include "buffrecord.hpp"
#include "dbcomrecord.hpp"

class BattleObject;
class BaseBuffAct;
class BuffList;

class BaseBuff
{
    private:
        friend class BuffList;

    private:
        struct BuffActRunner
        {
            long tpsCount = 0;
            std::unique_ptr<BaseBuffAct> ptr;
        };

    protected:
        const uint32_t m_id;

    protected:
        BattleObject * const m_bo;

    protected:
        double m_accuTime = 0.0;

    protected:
        std::vector<BuffActRunner> m_actList;

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
            if(getBR().duration <= 0){
                return false;
            }
            return std::lround(m_accuTime) >= getBR().duration;
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
        virtual void runOnUpdate();
        virtual void runOnTrigger(int);
        virtual void runOnDone();

    public:
        const BuffRecord &getBR() const
        {
            return DBCOM_BUFFRECORD(id());
        }
};
