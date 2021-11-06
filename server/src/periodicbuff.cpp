#include "periodicbuff.hpp"
#include "battleobject.hpp"

PeriodicBuff::PeriodicBuff(uint32_t id, BattleObject *bo, std::function<void(PeriodicBuff *)> trigger)
    : BaseBuff(id, bo)
    , m_trigger(trigger)
{}

void PeriodicBuff::update()
{
    const auto neededCount = m_timer.diff_msec() * m_br.tps / 1000ULL;
    while(neededCount > m_triggeredCount){
        if(m_trigger){
            m_trigger(this);
        }
        m_triggeredCount++;
    }
}
