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
        BattleObject * const m_bo;

    protected:
        const uint64_t m_fromUID;

    protected:
        const uint32_t m_id;

    protected:
        double m_accuTime = 0.0;

    protected:
        std::vector<BuffActRunner> m_runList;

    public:
        BaseBuff(BattleObject *, uint64_t, uint32_t);

    public:
        virtual ~BaseBuff();

    public:
        uint32_t id() const
        {
            return m_id;
        }

        uint64_t fromUID() const
        {
            return m_fromUID;
        }

    public:
        BattleObject * getBO()
        {
            return m_bo;
        }

        const BattleObject * getBO() const
        {
            return m_bo;
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
