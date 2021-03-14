/*
 * =====================================================================================
 *
 *       Filename: luamodule.hpp
 *        Created: 06/03/2017 20:24:34
 *    Description:
 *                 base class to register all functions, libs.
 *                 don't call lua error("..") from C++, it calls longmp, skips all dtors
 *
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
#include <sol/sol.hpp>

class LuaModule
{
    protected:
        sol::state m_luaState;

    public:
        LuaModule();

    public:
        virtual ~LuaModule() = default;

    public:
        sol::state &getLuaState()
        {
            return m_luaState;
        }

    protected:
        virtual void addLogString(int, const char8_t *) = 0;
};

// directives to include a lua file to C++ src code
// usage example:
//
//     const auto s
//     {
//         INCLUA_BEGIN(char)
//             #include "script.lua"
//         INCLUA_END()
//     };
//
// the script need to be in a special format
// it requires to be with these two comment line at start and end lines to support both lua and C++
//
//     --, u8R"###(  <-- required
//
//     function f()
//     end
//
//     -- )###"      <-- required
//

#define INCLUA_BEGIN(script_byte_type) \
[]() \
{ \
    using _INCLUA_BYTE_TYPE = script_byte_type; \
    const char8_t *_dummy_cstr = u8"?"; \
\
    const char8_t *_consume_decr_op = _dummy_cstr + 1; \
    const char8_t *_use_second_cstr[] \
    { \
        _consume_decr_op

#define INCLUA_END() \
    }; \
    return (const _INCLUA_BYTE_TYPE *)(_use_second_cstr[1]); \
}()
