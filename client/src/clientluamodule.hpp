#pragma once
#include "luamodule.hpp"

class ProcessRun;
class ClientLuaModule: public LuaModule
{
    private:
        ProcessRun *m_proc;

    public:
        ClientLuaModule(ProcessRun *);

    protected:
        void addLogString(int, const char8_t *) override;
};