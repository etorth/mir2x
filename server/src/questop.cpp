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
            m_luaRunner->spawn(m_runnerSeqID++, {}, str_printf("_RSVD_NAME_callTriggers(SYS_ON_LEVELUP, %llu, %d, %d)", to_llu(mpk.from()), sdQTLU.oldLevel, sdQTLU.newLevel).c_str());
        },

        [](auto)
        {
            throw fflerror("invalid quest trigger");
        },
    }, mpk.deserialize<SDQuestTriggerVar>());
}
