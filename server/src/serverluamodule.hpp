#pragma once
#include "luamodule.hpp"

class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule();

    public:
        ~ServerLuaModule() override = default;

    protected:
       void addLogString(int, const char8_t *) override;
};
