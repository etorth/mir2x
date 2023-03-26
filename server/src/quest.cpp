#include "quest.hpp"

Quest::Quest(const SDInitQuest &initQuest)
    : ServerObject(uidf::getQuestUID(initQuest.questID))
    , m_scriptName(initQuest.fullScriptName)
{}

void Quest::onActivate()
{
    ServerObject::onActivate();
    m_luaRunner = std::make_unique<ServerLuaCoroutineRunner>(m_actorPod, [this](ServerLuaModule *luaModule)
    {
        luaModule->bindFunction("getNPCharUID", [this]() -> uint64_t
        {

        });

        luaModule->pfrCheck(luaModule->execFile(m_scriptName.c_str()));
    });
}
