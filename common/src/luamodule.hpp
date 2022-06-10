// base class to register all functions, libs.
// don't call lua error("..") from C++, it calls longmp, skips all dtors

#pragma once
#include <sol/sol.hpp>
#include "strf.hpp"
#include "filesys.hpp"

class LuaModule
{
    protected:
        sol::state m_luaState;
        sol::environment m_replaceEnv;

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
            return execRawString(s.c_str());
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

            // execRawString() is only used to include lua file:
            //
            //    execRawString(BEGIN_LUAINC(char)
            //        #include "xxx.lua"
            //    END_LUAINC());
            //
            // in xxx.lua there can be string.format() calls and execRawString() doesn't parse it

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
        bool pfrCheck(const sol::protected_function_result &, const std::function<void(const std::string &)> & = nullptr); // parse if pfr is an error
};

struct LuaCORunner
{
    // sol::thread can be reused for multiple coroutines
    // but I guess changes of previous coroutine persists in the thread stack

    sol::thread runner;
    sol::coroutine callback;

    LuaCORunner(sol::state &s, const std::string &func)
        : runner(sol::thread::create(s.lua_state()))
        , callback(sol::state_view(runner.state())[func])
    {
        fflassert(str_haschar(func), func);
        fflassert(callback, func);
    }
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
