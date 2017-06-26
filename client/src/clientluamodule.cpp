/*
 * =====================================================================================
 *
 *       Filename: clientluamodule.cpp
 *        Created: 06/25/2017 18:58:33
 *  Last Modified: 06/25/2017 22:40:10
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

#include "log.hpp"
#include "processrun.hpp"
#include "clientluamodule.hpp"

ClientLuaModule::ClientLuaModule(ProcessRun *pRun, int nOutPort)
    : LuaModule()
{
    if(true
            && pRun
            && nOutPort >= 0){
        // import all predefined lua functions / variables
        // currently I don't keep ProcessRun and OutPort internally
        // because every command executed is pre-registered by ProcessRun::RegisterLuaExport()
        // in which these information has been captured
        pRun->RegisterLuaExport(this, nOutPort);
    }
}
