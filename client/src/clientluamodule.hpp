/*
 * =====================================================================================
 *
 *       Filename: clientluamodule.hpp
 *        Created: 06/25/2017 18:57:17
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

class ProcessRun;
class ClientLuaModule: public LuaModule
{
    private:
        ProcessRun *m_proc;

    public:
        ClientLuaModule(ProcessRun *);

    protected:
        void addLog(int, const char8_t *) override;
};
