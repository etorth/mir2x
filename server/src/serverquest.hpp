#pragma once
#include <memory>
#include "serverobject.hpp"
#include "serverluacoroutinerunner.hpp"

class ServerQuest final: public ServerObject
{
    private:
        const std::string m_scriptName;

    private:
        std::unique_ptr<ServerLuaCoroutineRunner> m_luaRunner;

    public:
        ServerQuest(uint32_t, std::string);
};
