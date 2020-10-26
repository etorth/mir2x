/*
 * =====================================================================================
 *
 *       Filename: luamodule.hpp
 *        Created: 06/03/2017 20:24:34
 *    Description:
 *                 base class to register all functions, libs.
 *                 don't call lua error("..") from C++, it calls longmp, skips all dtors
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
        sol::state m_luaState;

    public:
        LuaModule();

    public:
        virtual ~LuaModule() = default;

    public:
        sol::state &getLuaState()
        {
            return m_luaState;
        }

    protected:
        virtual void addLog(int, const char8_t *) = 0;
};
