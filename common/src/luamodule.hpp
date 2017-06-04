/*
 * =====================================================================================
 *
 *       Filename: luamodule.hpp
 *        Created: 06/03/2017 20:24:34
 *  Last Modified: 06/03/2017 20:26:13
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
#include <sol.hpp>

class LuaModule: public sol::state
{
    public:
        LuaModule();
       ~LuaModule() = default;
};
