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
        const auto buffSeq = getBuff()->buffSeq();
        const auto actOff  = actOff();
        getBuff()->getBO()->defer([boPtr = getBuff()->getBO(), buffSeq, actOff, thisptr = this](this auto) -> corof::awaitable<>
        {
            if(auto buffPtr = boPtr->hasBuff(buffSeq)){
                if(auto tgrPtr = dynamic_cast<BaseBuffActTrigger *>(buffPtr->hasBuffAct(actOff))){
                    const auto tick = 1000L / tgrPtr->getBAREF().trigger.tps;
                    const auto totalCount = tgrPtr->getBAREF().trigger.tps * tgrPtr->getBAREF().duration;

                    co_await runOnTrigger(BATGR_TIME);
                    for(int i = 1; i < totalCount; ++i){
                        co_await getBuff()->getBO()->asyncWait(tick);
                        co_await runOnTrigger(BATGR_TIME);
                    }
                }
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
