/*
 * =====================================================================================
 *
 *       Filename: clientluamodule.cpp
 *        Created: 06/25/2017 18:58:33
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

#include "totype.hpp"
#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientluamodule.hpp"

ClientLuaModule::ClientLuaModule(ProcessRun *proc)
    : LuaModule()
    , m_proc(proc)
{
    fflassert(m_proc);
    m_proc->registerLuaExport(this);
}

void ClientLuaModule::addLogString(int logType, const char8_t *logInfo)
{
    m_proc->addCBLog(logType, u8"%s", to_cstr(logInfo));
}
