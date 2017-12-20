/*
 * =====================================================================================
 *
 *       Filename: luamodule.hpp
 *        Created: 06/03/2017 20:24:34
 *  Last Modified: 12/20/2017 01:40:27
 *
 *    Description: make dtor simple to use sol::state::~state()
 *                 if we do need to release anything, put in LuaModule::Release()
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
#include <sol/sol.hpp>

class LuaModule
{
    protected:
        sol::state m_LuaState;

    public:
        LuaModule();

    public:
        virtual ~LuaModule() = default;

    public:
        sol::state &GetLuaState()
        {
            return m_LuaState;
        }

    protected:
        virtual void addLog(int, const char *) = 0;
};
