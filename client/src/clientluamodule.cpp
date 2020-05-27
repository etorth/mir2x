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

#include "fflerror.hpp"
#include "processrun.hpp"
#include "clientluamodule.hpp"

ClientLuaModule::ClientLuaModule(ProcessRun *proc)
    : LuaModule()
    , m_proc(proc)
{
    if(!m_proc){
        throw fflerror("null ProcessRun pointer");
    }
    m_proc->RegisterLuaExport(this);
}

void ClientLuaModule::addLog(int logType, const char *logInfo)
{
    if(!logInfo){
        logInfo = "(null)";
    }
    m_proc->addCBLog(logType, "%s", logInfo);
}
