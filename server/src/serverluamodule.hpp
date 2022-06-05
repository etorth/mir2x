#pragma once
#include "luamodule.hpp"

class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule();
       ~ServerLuaModule() = default;

    protected:
       void addLogString(int, const char8_t *) override;
};
