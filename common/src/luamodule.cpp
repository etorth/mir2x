/*
 * =====================================================================================
 *
 *       Filename: luamodule.cpp
 *        Created: 06/03/2017 20:26:17
 *  Last Modified: 12/12/2017 22:49:35
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

#include <chrono>
#include <thread>
#include "luamodule.hpp"

LuaModule::LuaModule()
    : sol::state()
{
    open_libraries(sol::lib::base);
    open_libraries(sol::lib::math);
    open_libraries(sol::lib::string);

    // register command sleep
    // sleep current lua thread and return after the specified ms
    // can use posix.sleep(ms), but here use std::this_thread::sleep_for(x)
    set_function("sleep", [this](int nSleepMS)
    {
        if(nSleepMS > 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        }
    });
}
