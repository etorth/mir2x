/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.hpp
 *        Created: 06/02/2017 17:39:56
 *  Last Modified: 06/11/2017 18:12:35
 *
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#pragma once
#include <cstdint>
#include "luamodule.hpp"

class ServerLuaModule: public LuaModule
{
    private:
        uint32_t m_CWID;

    public:
        uint32_t CWID() const { return m_CWID; }

    public:
        ServerLuaModule(uint32_t);
       ~ServerLuaModule() = default;
};
