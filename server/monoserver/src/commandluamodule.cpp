/*
 * =====================================================================================
 *
 *       Filename: commandluamodule.cpp
 *        Created: 06/02/2017 17:40:54
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

extern MonoServer *g_monoServer;
CommandLuaModule::CommandLuaModule(uint32_t cwid)
    : ServerLuaModule()
    , m_CWID(cwid)
{
    g_monoServer->regLuaExport(this, cwid);
}
