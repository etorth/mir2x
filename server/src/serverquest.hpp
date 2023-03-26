#pragma once
#include <memory>
#include "serverluacoroutinerunner.hpp"

class ServerQuest final: public ServerObject
{
    private:
        const uint32_t m_id;
        const std::string m_scriptName;

    private:
        std::unique_ptr<ServerLuaCoroutineRunner> m_luaRunner;

    public:
        ServerQuest(uint32_t, std::string);
};
