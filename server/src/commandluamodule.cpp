#include "monoserver.hpp"
#include "serverluamodule.hpp"

extern MonoServer *g_monoServer;
CommandLuaModule::CommandLuaModule(uint32_t cwid)
    : ServerLuaModule()
    , m_CWID(cwid)
{
    g_monoServer->regLuaExport(this, cwid);
}
