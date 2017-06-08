/*
 * =====================================================================================
 *
 *       Filename: luamodule.cpp
 *        Created: 06/03/2017 20:26:17
 *  Last Modified: 06/07/2017 18:38:21
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

#include "luamodule.hpp"

LuaModule::LuaModule()
    : sol::state()
{
    open_libraries(sol::lib::base);
    open_libraries(sol::lib::math);
    open_libraries(sol::lib::string);
}
