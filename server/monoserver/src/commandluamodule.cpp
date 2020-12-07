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

CommandLuaModule::CommandLuaModule(uint32_t nCWID)
    : ServerLuaModule()
    , m_CWID(nCWID)
{
    extern MonoServer *g_monoServer;
    g_monoServer->regLuaExport(this, nCWID);
}
