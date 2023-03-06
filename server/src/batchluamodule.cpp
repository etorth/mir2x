#include <sol/sol.hpp>
#include "monoserver.hpp"
#include "batchluamodule.hpp"

BatchLuaModule::BatchLuaModule()
    : ServerLuaModule()
    , m_batchCmd()
{}

bool BatchLuaModule::LoopOne()
{
    if(Empty()){
        return true;
    }

    auto stCallResult = getLuaState().script(m_batchCmd.c_str(), [](lua_State *, sol::protected_function_result stResult)
    {
        // default handler
        // do nothing and let the call site handle the errors
        return stResult;
    });

    if(stCallResult.valid()){
        // default nothing printed
        // we can put information here to show call succeeds
        return true;
    }
    else{
        sol::error stError = stCallResult;

        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "Script error: %s", stError.what());
        return false;
    }
}
