/*
 * =====================================================================================
 *
 *       Filename: commandluamodule.hpp
 *        Created: 06/02/2017 17:39:56
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
#include "serverluamodule.hpp"

class CommandLuaModule: public ServerLuaModule
{
    private:
        uint32_t m_CWID;

    public:
        CommandLuaModule(uint32_t);
       ~CommandLuaModule() = default;

    public:
        uint32_t CWID() const
        {
            return m_CWID;
        }
};
