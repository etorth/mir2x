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
