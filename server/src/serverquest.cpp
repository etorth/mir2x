#include "serverquest.hpp"

ServerQuest::ServerQuest(uint32_t id, std::string scriptName)
    : m_id(id)
    , m_scriptName(std::move(scriptName))
{
}
