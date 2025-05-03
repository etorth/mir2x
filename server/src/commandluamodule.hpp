#pragma once
#include <cstdint>
#include "serverluamodule.hpp"

class CommandLuaModule: public ServerLuaModule
{
    private:
        const uint32_t m_CWID;

    public:
        CommandLuaModule(uint32_t);

    public:
        uint32_t CWID() const
        {
            return m_CWID;
        }
};
