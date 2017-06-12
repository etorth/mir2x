/*
 * =====================================================================================
 *
 *       Filename: serverluamodule.cpp
 *        Created: 06/02/2017 17:40:54
 *  Last Modified: 06/11/2017 18:13:07
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

ServerLuaModule::ServerLuaModule(uint32_t nCWID)
    : LuaModule()
    , m_CWID(nCWID)
{
    extern MonoServer *g_MonoServer;
    g_MonoServer->RegisterLuaExport(this, nCWID);
}
