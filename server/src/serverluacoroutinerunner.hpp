#pragma once
#include <deque>
#include <concepts>
#include <functional>
#include <type_traits>
#include <sol/sol.hpp>
#include "luaf.hpp"
#include "totype.hpp"
#include "serverluamodule.hpp"

class LuaCoopCallDoneFlag final
{
    private:
        std::shared_ptr<bool> m_flag;

    public:
        LuaCoopCallDoneFlag()
            : m_flag(std::make_shared<bool>(false))
        {}

        LuaCoopCallDoneFlag(const LuaCoopCallDoneFlag &arg)
            : m_flag(arg.m_flag)
        {}

    public:
        ~LuaCoopCallDoneFlag()
        {
            *m_flag = true;
        }

    public:
        LuaCoopCallDoneFlag(LuaCoopCallDoneFlag &&) = delete;

    public:
        LuaCoopCallDoneFlag & operator = (const LuaCoopCallDoneFlag & ) = delete;
        LuaCoopCallDoneFlag & operator = (      LuaCoopCallDoneFlag &&) = delete;

    public:
        operator bool () const
        {
            return *m_flag;
        }
};

class ServerLuaCoroutineRunner;
class LuaCoopResumer final
{
    private:
        ServerLuaCoroutineRunner * const m_luaRunner;

    private:
        sol::function m_callback;

    private:
        const uint64_t m_threadKey;
        const uint64_t m_threadSeqID;

    private:
        const LuaCoopCallDoneFlag m_doneFlag;

    public:
        LuaCoopResumer(ServerLuaCoroutineRunner *luaRunner, sol::function callback, uint64_t threadKey, uint64_t threadSeqID, const LuaCoopCallDoneFlag &doneFlag)
            : m_luaRunner(luaRunner)
            , m_callback(std::move(callback))
            , m_threadKey(threadKey)
            , m_threadSeqID(threadSeqID)
            , m_doneFlag(doneFlag)
        {}

    public:
        LuaCoopResumer(const LuaCoopResumer & resumer)
            : LuaCoopResumer(resumer.m_luaRunner, resumer.m_callback, resumer.m_threadKey, resumer.m_threadSeqID, resumer.m_doneFlag)
        {}

        LuaCoopResumer(LuaCoopResumer && resumer)
            : LuaCoopResumer(resumer.m_luaRunner, std::move(resumer.m_callback), resumer.m_threadKey, resumer.m_threadSeqID, resumer.m_doneFlag /* always copy doneFlag */)
        {}

    public:
        ~LuaCoopResumer() = default;

    public:
        LuaCoopResumer & operator = (const LuaCoopResumer & ) = delete;
        LuaCoopResumer & operator = (      LuaCoopResumer &&) = delete;

    public:
        template<typename... Args> void operator () (Args && ... args) const
        {
            m_callback(std::forward<Args>(args)...);
            if(m_doneFlag){
                resumeRunner(m_luaRunner, m_threadKey, m_threadSeqID);
            }
        }

    public:
        void pushOnClose(std::function<void()>) const;
        void  popOnClose()                      const;

    private:
        static void resumeRunner(ServerLuaCoroutineRunner *, uint64_t, uint64_t); // resolve dependency
};

class LuaCoopState final
{
    private:
        sol::this_state m_state;

    public:
        explicit LuaCoopState(sol::this_state state)
            : m_state(state)
        {}

    public:
        sol::state_view getView() const
        {
            return sol::state_view(m_state);
        }
};

class LuaCoopVargs final
{
    private:
        sol::variadic_args m_vargs;

    public:
        LuaCoopVargs(sol::variadic_args args)
            : m_vargs(args)
        {}

    public:
        std::vector<luaf::luaVar> asLuaVarList() const
        {
            return {};
        }
};

class ActorPod;
class LuaThreadHandle;
class ServerLuaCoroutineRunner: public ServerLuaModule
{
    private:
        ActorPod * const m_actorPod;

    private:
        LuaThreadHandle *m_currRunner = nullptr;

    private:
        uint64_t m_seqID = 1;
        std::unordered_map<uint64_t, std::unique_ptr<LuaThreadHandle>> m_runnerList;

    public:
        ServerLuaCoroutineRunner(ActorPod *);

    public:
        ~ServerLuaCoroutineRunner();

    public:
        uint64_t spawn(uint64_t, std::pair<uint64_t, uint64_t>, const std::string &);
        uint64_t spawn(uint64_t,                                const std::string &, std::function<void(const sol::protected_function_result &)> = nullptr, std::function<void()> = nullptr);

    public:
        uint64_t getSeqID(uint64_t) const;

    public:
        void close(uint64_t, uint64_t = 0);
        void resume(uint64_t, uint64_t = 0);
        LuaThreadHandle *hasKey(uint64_t, uint64_t = 0) const;

    public:
        void addNotify(uint64_t, uint64_t, std::vector<luaf::luaVar>);
        std::optional<bool> needNotify(uint64_t, uint64_t) const;

    public:
        void pushOnClose(uint64_t, uint64_t, std::function<void()>);
        void popOnClose(uint64_t, uint64_t);

    private:
        void resumeRunner(LuaThreadHandle *, std::optional<std::string> = {});

    private:
        static std::string concatCode(const std::string &code)
        {
            // exception thrown eventually feeds to FLTK
            // FLTK error message window doesn't accept multiline string

            std::string line;
            std::string codeStr;
            std::stringstream ss(code);

            while(std::getline(ss, line, '\n')){
                if(!codeStr.empty()){
                    codeStr += "\\n";
                }
                codeStr += line;
            }

            return codeStr;
        }

    private:
        template<typename Lambda, typename Ret, typename... Args> static std::tuple<Args...> _extractLambdaUserArgsHelper(Ret (Lambda::*)(LuaCoopResumer, Args...));
        template<typename Lambda, typename Ret, typename... Args> static std::tuple<Args...> _extractLambdaUserArgsHelper(Ret (Lambda::*)(LuaCoopResumer, Args...) const );
        template<typename Lambda, typename Ret, typename... Args> static std::tuple<Args...> _extractLambdaUserArgsHelper(Ret (Lambda::*)(LuaCoopResumer, LuaCoopState, Args...));
        template<typename Lambda, typename Ret, typename... Args> static std::tuple<Args...> _extractLambdaUserArgsHelper(Ret (Lambda::*)(LuaCoopResumer, LuaCoopState, Args...) const );

        template<typename Lambda, typename Ret                                 > static void _extractLambdaSecondArgHelper(Ret (Lambda::*)(LuaCoopResumer));
        template<typename Lambda, typename Ret                                 > static void _extractLambdaSecondArgHelper(Ret (Lambda::*)(LuaCoopResumer) const);
        template<typename Lambda, typename Ret, typename Arg2, typename... Args> static Arg2 _extractLambdaSecondArgHelper(Ret (Lambda::*)(LuaCoopResumer, Arg2, Args...));
        template<typename Lambda, typename Ret, typename Arg2, typename... Args> static Arg2 _extractLambdaSecondArgHelper(Ret (Lambda::*)(LuaCoopResumer, Arg2, Args...) const );

        template<typename Lambda> struct _extractLambdaUserArgsAsTuple
        {
            using type = decltype(_extractLambdaUserArgsHelper(&Lambda::operator()));
        };

        template<typename Ret, typename... Args> struct _extractLambdaUserArgsAsTuple<Ret(*)(LuaCoopResumer, Args...)>
        {
            using type = std::tuple<Args...>;
        };

        template<typename Ret, typename... Args> struct _extractLambdaUserArgsAsTuple<Ret(*)(LuaCoopResumer, LuaCoopState, Args...)>
        {
            using type = std::tuple<Args...>;
        };

        template<typename Lambda> struct _extractLambdaSecondArg
        {
            using type = decltype(_extractLambdaSecondArgHelper(&Lambda::operator()));
        };

        template<typename Ret> struct _extractLambdaSecondArg<Ret(*)(LuaCoopResumer)>
        {
            using type = void;
        };

        template<typename Ret, typename Arg2, typename... Args> struct _extractLambdaSecondArg<Ret(*)(LuaCoopResumer, Arg2, Args...)>
        {
            using type = Arg2;
        };

    public:
        template<typename Func> void bindFunctionCoop(std::string funcName, Func && func)
        {
            fflassert(str_haschar(funcName));
            bindFunction(funcName + SYS_COOP, [this](auto && func)
            {
                return [func = std::function(std::forward<Func>(func)), this](typename _extractLambdaUserArgsAsTuple<Func>::type args, sol::function cb, uint64_t threadKey, uint64_t threadSeqID, sol::this_state s)
                {
                    fflassert(s.lua_state());

                    fflassert(threadKey);
                    fflassert(threadSeqID); // don't allow 0 internally
                    fflassert(hasKey(threadKey, threadSeqID), threadKey, threadSeqID); // called from lua coroutine, must be valid

                    const LuaCoopCallDoneFlag doneFlag;
                    if constexpr (std::is_same_v<LuaCoopState, typename _extractLambdaSecondArg<Func>::type>){
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, cb, threadKey, threadSeqID, doneFlag), LuaCoopState(s)), std::move(args)));
                    }
                    else{
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, cb, threadKey, threadSeqID, doneFlag)), std::move(args)));
                    }
                };
            }(std::forward<Func>(func)));
        }
};
