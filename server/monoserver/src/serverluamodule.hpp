/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.hpp
 *        Created: 06/02/2017 17:39:56
 *  Last Modified: 06/04/2017 12:31:20
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
#include "luamodule.hpp"

class CommandWindow;
class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule(CommandWindow *);
       ~ServerLuaModule() = default;
};
