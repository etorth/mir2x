#include "totype.hpp"
#include "periodicbuff.hpp"
#include "battleobject.hpp"

PeriodicBuff::PeriodicBuff(uint32_t id, BattleObject *bo, std::function<void(PeriodicBuff *)> trigger)
    : BaseBuff(id, bo)
    , m_trigger(trigger)
{}

void PeriodicBuff::runOnUpdate()
{
    for(const auto needed = to_uz(std::lround(m_accuTime * m_br.tps / 1000.0)); m_triggerCount < needed; ++m_triggerCount){
        if(m_trigger){
            m_trigger(this);
        }
    }
}
