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
#include "strf.hpp"
#include "filesys.hpp"

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

    public:
        sol::protected_function_result execFile(const char *path)
        {
            fflassert(     str_haschar(path), path);
            fflassert(filesys::hasFile(path), path);

            return m_luaState.script_file(path, [](lua_State *, sol::protected_function_result result)
            {
                // default handler
                // do nothing and let the call site handle the errors
                return result;
            });
        }

        sol::protected_function_result execString(const char *format, ...)
        {
            std::string s;
            str_format(format, s);

            return m_luaState.script(s, [](lua_State *, sol::protected_function_result result)
            {
                // default handler
                // do nothing and let the call site handle the errors
                return result;
            });
        }

        sol::protected_function_result execRawString(const char *s)
        {
            // execString() fails when format contains lua string.format() string, like
            //
            //    execString(
            //        "function my_error(err)                             ""\n"
            //        "    error(string.format('dected error: %s', err))  ""\n"
            //        "end                                                ""\n");
            //
            // we would like to take the lua code as raw string
            // but it contains %s, execString() parses it incorrectly as format string

            return m_luaState.script(s, [](lua_State *, sol::protected_function_result result)
            {
                // default handler
                // do nothing and let the call site handle the errors
                return result;
            });
        }

    public:
        template<typename Callable> void bindFunction(const char *func, Callable && callable)
        {
            fflassert(str_haschar(func), func);
            m_luaState.set_function(func, std::forward<Callable>(callable));
        }

    protected:
        virtual void addLogString(int, const char8_t *) = 0;

    public:
        bool pfrCheck(const sol::protected_function_result &); // parse if pfr is an error
};

// directives to include a lua file to C++ src code
// usage example:
//
//     const auto s
//     {
//         BEGIN_LUAINC(char)
//             #include "script.lua"
//         END_LUAINC()
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

#define BEGIN_LUAINC(script_byte_type) \
[]() \
{ \
    using _INCLUA_BYTE_TYPE = script_byte_type; \
    const char8_t *_dummy_cstr = u8"?"; \
\
    const char8_t *_consume_decr_op = _dummy_cstr + 1; \
    const char8_t *_use_second_cstr[] \
    { \
        _consume_decr_op

#define END_LUAINC() \
    }; \
    return (const _INCLUA_BYTE_TYPE *)(_use_second_cstr[1]); \
}()
