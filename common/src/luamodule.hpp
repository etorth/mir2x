/*
 * =====================================================================================
 *
 *       Filename: luamodule.hpp
 *        Created: 06/01/2017 23:14:26
 *  Last Modified: 06/02/2017 01:02:12
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

#pragma once
#include <selene/selene.h>

class LuaModule
{
    protected:
        sel::State m_State;

    public:
        LuaModule();
        virtual ~LuaModule();
};
