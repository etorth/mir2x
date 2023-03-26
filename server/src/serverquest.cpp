#include "serverquest.hpp"

ServerQuest::ServerQuest(uint32_t id, std::string scriptName)
    : ServerObject(uidf::getQuestUID(id))
    , m_scriptName(std::move(scriptName))
{
}
