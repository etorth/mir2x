/*
 * =====================================================================================
 *
 *       Filename: batchluamodule.cpp
 *        Created: 12/19/2017 23:42:06
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

#include <sol/sol.hpp>
#include "monoserver.hpp"
#include "batchluamodule.hpp"

BatchLuaModule::BatchLuaModule()
    : ServerLuaModule()
    , m_BatchCmd()
{}

bool BatchLuaModule::LoopOne()
{
    if(Empty()){
        return true;
    }

    auto stCallResult = GetLuaState().script(m_BatchCmd.c_str(), [](lua_State *, sol::protected_function_result stResult)
    {
        // default handler
        // do nothing and let the call site handle the errors
        return stResult;
    });

    if(stCallResult.valid()){
        // default nothing printed
        // we can put information here to show call succeeds
        return true;
    }else{
        sol::error stError = stCallResult;

        extern MonoServer *g_MonoServer;
        g_MonoServer->addLog(LOGTYPE_WARNING, "Script error: %s", stError.what());
        return false;
    }
}
