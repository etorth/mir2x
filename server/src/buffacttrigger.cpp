#include "fflerror.hpp"
#include "battleobject.hpp"
#include "dbcomid.hpp"
#include "buffacttrigger.hpp"

#define _decl_buff_act_trigger(name) \
template<> class BuffActTrigger<DBCOM_BUFFACTID(name)>: public BaseBuffActTrigger \
{ \
    public: \
        BuffActTrigger(uint32_t argBuffID) \
            : BaseBuffActTrigger(argBuffID, DBCOM_BUFFACTID(name)) \
        {} \
 \
    public: \
        void runOnTrigger(BattleObject *, int); \
}; \
\
void  BuffActTrigger<DBCOM_BUFFACTID(name)>::runOnTrigger

_decl_buff_act_trigger(u8"HP")(BattleObject *bo, int)
{
    bo->updateHealth(5);
}

_decl_buff_act_trigger(u8"HP恢复")(BattleObject *bo, int)
{
    bo->updateHealth(5);
}

#undef _decl_buff_act_trigger

namespace
{
    template<uint32_t INDEX> BaseBuffActTrigger * _create_buff_trigger_helper(uint32_t argBuffID, uint32_t argBuffActID)
    {
        static_assert(INDEX > 0);
        static_assert(INDEX < DBCOM_BUFFACTENDID());

        if(argBuffActID == INDEX){
            return new BuffActTrigger<INDEX>(argBuffID);
        }
        return _create_buff_trigger_helper<INDEX + 1>(argBuffID, argBuffActID);
    }

    template<> BaseBuffActTrigger * _create_buff_trigger_helper<DBCOM_BUFFACTENDID()>(uint32_t argBuffID, uint32_t argBuffActID)
    {
        throw fflvalue(argBuffID, argBuffActID);
    }
}

BaseBuffActTrigger *BaseBuffActTrigger::createTrigger(uint32_t argBuffID, uint32_t argBuffActID)
{
    return _create_buff_trigger_helper<1>(argBuffID, argBuffActID);
}
