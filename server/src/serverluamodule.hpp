#pragma once
#include "luamodule.hpp"

class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule();

    protected:
       void addLogString(int, const char8_t *) override;
};
