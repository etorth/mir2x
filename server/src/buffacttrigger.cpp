#include "fflerror.hpp"
#include "battleobject.hpp"
#include "dbcomid.hpp"
#include "corof.hpp"
#include "buffacttrigger.hpp"

BaseBuffActTrigger::BaseBuffActTrigger(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
{
    fflassert(getBAR().isTrigger());
    if(getBAREF().trigger.on & BATGR_TIME){
        const auto tick = 1000L / getBAREF().trigger.tps;
        const auto tgrCount = [this]()
        {
            switch(getBAREF().duration){
                case BADUR_UNLIMITED: return -1;
                case BADUR_INSTANT  : return  0;
                default             : return getBAREF().duration * getBAREF().trigger.tps;
            }
        }();

        getBuff()->getBO()->defer([bo = getBuff()->getBO(), key = actKey(), tick, tgrCount](this auto) -> corof::awaitable<>
        {
            if(auto tgrPtr = dynamic_cast<BaseBuffActTrigger *>(bo->m_buffList.hasBuffAct(key))){
                co_await tgrPtr->runOnTrigger(BATGR_TIME);
            }

            for(int i = 1; totalCount < 0 || i < totalCount; ++i){
                co_await bo->asyncWait(tick);
                if(auto tgrPtr = dynamic_cast<BaseBuffActTrigger *>(bo->m_buffList.hasBuffAct(key))){
                    co_await tgrPtr->runOnTrigger(BATGR_TIME);
                }
            }

                co_await tgrPtr->runOnDone();
            }
        });
    }
}

template<uint32_t INDEX> class IndexedBuffActTrigger: public BaseBuffActTrigger
{
    protected:
        IndexedBuffActTrigger(BaseBuff *argBuff)
            : BaseBuffActTrigger([argBuff]()
              {
                  fflassert(argBuff);
                  return argBuff;
              }(),

              [argBuff]() -> size_t
              {
                  static_assert(INDEX > 0);
                  static_assert(INDEX < DBCOM_BUFFACTENDID());

                  for(size_t buffActOff = 0; const auto &baref: argBuff->getBR().actList){
                      fflassert(baref);
                      if(DBCOM_BUFFACTID(baref.name) == INDEX){
                          return buffActOff;
                      }
                      buffActOff++;
                  }
                  throw fflvalue(argBuff, argBuff->getBR().name, DBCOM_BUFFACTRECORD(INDEX).name);
              }())
        {}
};

template<uint32_t INDEX> class BuffActTrigger: public IndexedBuffActTrigger<INDEX>
{
    private:
        friend class BaseBuffActTrigger;

    public:
        BuffActTrigger(BaseBuff *argBuff)
            : IndexedBuffActTrigger<INDEX>(argBuff)
        {}

    protected:
        corof::awaitable<> runOnTrigger(int) override
        {
            co_return;
        }
};

#define _decl_named_buff_act_trigger(name) template<> class BuffActTrigger<DBCOM_BUFFACTID(name)>: public IndexedBuffActTrigger<DBCOM_BUFFACTID(name)> \
{ \
    private: \
        friend class BaseBuffActTrigger; \
 \
    private: \
        static_assert(DBCOM_BUFFACTRECORD(name).isTrigger()); \
 \
    public: \
        BuffActTrigger(BaseBuff *argBuff) \
            : IndexedBuffActTrigger<DBCOM_BUFFACTID(name)>(argBuff) \
        {} \
 \
    protected: \
               corof::awaitable<> runOnTrigger(int) override; \
}; \
 \
corof::awaitable<>  BuffActTrigger<DBCOM_BUFFACTID(name)>::runOnTrigger

// define all triggers here
// macros usage:
//
//     _decl_named_buff_act_trigger(u8"trigger_name")(int ontgr)
//     {
//         ....
//     }
//

_decl_named_buff_act_trigger(u8"HP持续")(int trigger)
{
    if(trigger & BATGR_TIME){
        const auto [value, percentage] = std::get<BuffValuePercentage>(getBAREF().trigger.arg);
        co_await getBuff()->getBO()->updateHealth(value + std::lround(percentage * getBuff()->getBO()->getHealth().getMaxHP() / 100.0));
    }
}

_decl_named_buff_act_trigger(u8"MP持续")(int trigger)
{
    if(trigger & BATGR_TIME){
        const auto [value, percentage] = std::get<BuffValuePercentage>(getBAREF().trigger.arg);
        co_await getBuff()->getBO()->updateHealth(0, value + std::lround(percentage * getBuff()->getBO()->getHealth().getMaxMP() / 100.0));
    }
}

_decl_named_buff_act_trigger(u8"HP移动伤害")(int trigger)
{
    if(trigger & BATGR_MOVE){
        co_await getBuff()->getBO()->updateHealth(-1 * std::get<int>(getBAREF().trigger.arg));
    }
}

_decl_named_buff_act_trigger(u8"MP移动伤害")(int)
{
    co_await getBuff()->getBO()->updateHealth(0, -1);
}

#undef _decl_named_buff_act_trigger

namespace
{
    template<uint32_t INDEX> BaseBuffActTrigger * _create_buff_trigger_helper(BaseBuff *argBuff, uint32_t argBuffActID)
    {
        static_assert(INDEX > 0);
        static_assert(INDEX < DBCOM_BUFFACTENDID());

        if(argBuffActID == INDEX){
            return new BuffActTrigger<INDEX>(argBuff);
        }
        return _create_buff_trigger_helper<INDEX + 1>(argBuff, argBuffActID);
    }

    template<> BaseBuffActTrigger * _create_buff_trigger_helper<DBCOM_BUFFACTENDID()>(BaseBuff *argBuff, uint32_t argBuffActID)
    {
        throw fflvalue(argBuff, argBuffActID);
    }
}

BaseBuffActTrigger *BaseBuffActTrigger::createTrigger(BaseBuff *argBuff, size_t argBuffActOff)
{
    fflassert(argBuff);
    fflassert(argBuffActOff < argBuff->getBR().actList.size());

    const auto id = DBCOM_BUFFACTID(argBuff->getBR().actList.begin()[argBuffActOff].name);
    const auto &bar = DBCOM_BUFFACTRECORD(id);

    fflassert(id);
    fflassert(bar);

    return _create_buff_trigger_helper<1>(argBuff, id);
}
