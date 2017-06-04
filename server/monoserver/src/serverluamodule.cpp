/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.cpp
 *        Created: 06/02/2017 17:40:54
 *  Last Modified: 06/04/2017 12:31:48
 *
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

#include "monoserver.hpp"
#include "serverluamodule.hpp"

ServerLuaModule::ServerLuaModule(CommandWindow *pWindow)
    : LuaModule()
{
    extern MonoServer *g_MonoServer;
    g_MonoServer->RegisterLuaExport(this, pWindow);
}
