// base class to register all functions, libs.
// don't call lua error("..") from C++, it calls longmp, skips all dtors

#pragma once
#include <sol/sol.hpp>
#include "strf.hpp"
#include "filesys.hpp"
#include "fflerror.hpp"

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
        sol::state &getState()
        {
            return m_luaState;
        }

    public:
        sol::protected_function_result execFile(const char *);
        sol::protected_function_result execRawString(const char *);

    public:
        sol::protected_function_result execString(const char *format, ...) STR_PRINTF_CHECK_FORMAT(2)
        {
            std::string s;
            str_format(format, s);
            return execRawString(s.c_str());
        }

    public:
        template<typename Callable> void bindFunction(const std::string &func, Callable && callable)
        {
            fflassert(str_haschar(func), func);
            m_luaState.set_function(func, std::forward<Callable>(callable));
        }

        template<typename Callable> void bindYielding(const std::string &func, Callable && callable)
        {
            fflassert(str_haschar(func), func);
            m_luaState[func] = sol::yielding(std::forward<Callable>(callable));
        }

    protected:
        virtual void addLogString(int, const char *) = 0;

    public:
        bool pfrCheck(const sol::protected_function_result &, const std::function<void(const std::string &)> & = nullptr); // parse if pfr is an error
};

struct LuaCORunner
{
    // sol::thread can be reused for multiple coroutines
    // but I guess changes of previous coroutine persists in the thread stack

    sol::thread runner;
    sol::coroutine callback;

    LuaCORunner(sol::state &s, sol::function &func)
        : runner(sol::thread::create(s.lua_state()))
        , callback(func)
    {
        fflassert(callback);
    }

    LuaCORunner(sol::state &s, const std::string &func)
        : runner(sol::thread::create(s.lua_state()))
        , callback(sol::state_view(runner.state())[func])
    {
        fflassert(str_haschar(func), func);
        fflassert(callback, func);
    }
};

static_assert(sizeof(lua_Integer) >= sizeof(uint64_t)); // make sure lua_Integer can hold any UID
