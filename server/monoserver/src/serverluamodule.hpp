/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.hpp
 *        Created: 06/02/2017 17:39:56
 *  Last Modified: 06/02/2017 17:40:47
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

class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule();
       ~ServerLuaModule();
};
