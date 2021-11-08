#include "fflerror.hpp"
#include "bufftrigger.hpp"
#include "battleobject.hpp"

namespace
{
    template<uint32_t BTRG_INDEX> BaseBuffTrigger * _create_buff_trigger_helper(uint32_t triggerId, int arg)
    {
        static_assert(BTRG_INDEX > 0);
        static_assert(BTRG_INDEX < DBCOM_BUFFTRIGGERENDID());

        if(triggerId == BTRG_INDEX){
            return new BuffTrigger<BTRG_INDEX>(arg);
        }
        return _create_buff_trigger_helper<BTRG_INDEX + 1>(triggerId, arg);
    }

    template<> BaseBuffTrigger * _create_buff_trigger_helper<DBCOM_BUFFTRIGGERENDID()>(uint32_t triggerId, int arg)
    {
        throw bad_value(triggerId, arg);
    }
}

std::unique_ptr<BaseBuffTrigger> BaseBuffTrigger::createTrigger(uint32_t id, int arg)
{
    fflassert(id > 0);
    fflassert(id < DBCOM_BUFFTRIGGERENDID());

    const auto &tr = DBCOM_BUFFTRIGGERRECORD(id);
    fflassert(tr);

    return std::unique_ptr<BaseBuffTrigger>(_create_buff_trigger_helper<1>(id, arg));
}

void BuffTrigger<DBCOM_BUFFTRIGGERID(u8"HP恢复")>::runOnTrigger(BattleObject *bo, int)
{
    bo->updateHealth(m_arg);
}

void BuffTrigger<DBCOM_BUFFTRIGGERID(u8"HP伤害")>::runOnTrigger(BattleObject *bo, int)
{
    bo->updateHealth(m_arg);
}
