#include "quest.hpp"
#include "totype.hpp"

void Quest::on_AM_METRONOME(const ActorMsgPack &)
{
}

void Quest::on_AM_RUNQUESTTRIGGER(const ActorMsgPack &mpk)
{
    std::visit(SDQuestTriggerDispatcher
    {
        [&mpk, this](const SDQuestTriggerLevelUp &sdQTLU)
        {
            m_luaRunner->spawn(m_runnerSeqID++, {}, str_printf("_RSVD_NAME_trigger(SYS_ON_LEVELUP, %llu, %d, %d)", to_llu(mpk.from()), sdQTLU.oldLevel, sdQTLU.newLevel));
        },

        [&mpk, this](const SDQuestTriggerKill &sdQTK)
        {
            m_luaRunner->spawn(m_runnerSeqID++, {}, str_printf("_RSVD_NAME_trigger(SYS_ON_KILL, %llu, %llu)", to_llu(mpk.from()), to_llu(sdQTK.monsterID)));
        },

        [](auto)
        {
            throw fflerror("invalid quest trigger");
        },
    }, mpk.deserialize<SDQuestTriggerVar>());
}
