/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.hpp
 *        Created: 12/19/2017 01:07:36
 *    Description: base module for all server side lua support
 *
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
#include "luamodule.hpp"

class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule();
       ~ServerLuaModule() = default;

    protected:
       void addLogString(int, const char8_t *) override;
};
