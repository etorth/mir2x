#pragma once
#include <deque>
#include <concepts>
#include <functional>
#include <type_traits>
#include <sol/sol.hpp>
#include "sysconst.hpp"
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
        void * const m_currRunner;

    private:
        sol::function m_callback;

    private:
        const LuaCoopCallDoneFlag m_doneFlag;

    public:
        LuaCoopResumer(ServerLuaCoroutineRunner *, void *, sol::function, const LuaCoopCallDoneFlag &);

    public:
        LuaCoopResumer(const LuaCoopResumer & );
        LuaCoopResumer(      LuaCoopResumer &&);

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
                resumeRunner(m_luaRunner, m_currRunner);
            }
        }

    public:
        void pushOnClose(std::function<void()>) const;
        void  popOnClose()                      const;

    private:
        static void resumeRunner(ServerLuaCoroutineRunner *, void *); // resolve dependency
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
class ServerLuaCoroutineRunner: public ServerLuaModule
{
    protected:
        friend class LuaCoopResumer;

    protected:
        struct LuaThreadHandle
        {
            // for this class
            // the terms coroutine, runner, thread means same

            // scenario why adding seqID:
            // 1. received an event which triggers processNPCEvent(event)
            // 2. inside processNPCEvent(event) the script emits query to other actor
            // 3. when waiting for the response of the query, user clicked the close button or click init button to end up the current call stack
            // 4. receives the query response, we should ignore it
            //
            // to fix this we have to give every call stack an uniq seqID
            // and the query response needs to match the seqID

            const uint64_t key;
            const uint64_t seqID;

            // consume coroutine result
            // forward pfr to issuer as a special case
            std::function<void(const sol::protected_function_result &)> onDone;

            // thread can be closed when
            //
            //     1. it yields in C layer
            //     2. its control has been dropped and wait some callback to resume
            //
            // then if the thread is closed without calling the registered callback
            // we need some clear-functionality

            // thread can call back and forth in C/lua
            //
            // C -> lua -> C -> lua -> C -> lua
            //                         ^
            //                         |
            //                         +-- code of this layer is:
            //
            //                         bindYielding("_RSVD_NAME_pauseYielding", [](uint64_t time, uint64_t threadKey)
            //                         {
            //                             addDelay(time, [threadKey]()
            //                             {
            //                                 resume(threadKey);
            //                             });
            //                         });
            //
            // then even all C layer before this layer are not yield-able, still we know this chain may eventually get yielded
            // and each C layer may require to register a callback if closed before done, which requires a stack as
            //
            // C -> lua -> C -> lua -> C -> lua
            //                         ^
            //                         |
            //                         +-- code of this layer is:
            //
            //                         bindYielding("_RSVD_NAME_pauseYielding", [](uint64_t time, uint64_t threadKey)
            //                         {
            //                             const auto key = m_delayQueue.addDelay(time, [threadKey]()
            //                             {
            //                                 resume(threadKey);
            //                                 m_delayQueue.pop(); // no need to trigger if delayed command gets executed
            //                             });
            //
            //                             onClose.push([key]()
            //                             {
            //                                 m_delayQueue.erase(key);
            //                             })
            //                         });

            std::stack<std::function<void()>> onClose;

            sol::thread runner;
            sol::coroutine callback;

            bool needNotify = false;
            std::deque<luaf::luaVar> notifyList;

            LuaThreadHandle(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, std::function<void(const sol::protected_function_result &)> argOnDone, std::function<void()> argOnClose)
                : key(argKey)
                , seqID(argSeqID)
                , onDone(std::move(argOnDone))
                , runner(sol::thread::create(argLuaModule.getLuaState().lua_state()))
                , callback(sol::state_view(runner.state())["_RSVD_NAME_luaCoroutineRunner_main"])
            {
                fflassert(key);
                fflassert(seqID);

                if(argOnClose){
                    onClose.push(std::move(argOnClose));
                }
            }

            LuaThreadHandle(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, sol::function func, std::function<void(const sol::protected_function_result &)> argOnDone, std::function<void()> argOnClose)
                : key(argKey)
                , seqID(argSeqID)
                , onDone(std::move(argOnDone))
                , runner(sol::thread::create(argLuaModule.getLuaState().lua_state()))
                , callback(runner.state(), sol::ref_index(func.registry_index())) // callback initialized as: https://github.com/ThePhD/sol2/issues/836
            {
                fflassert(key);
                fflassert(seqID);

                if(argOnClose){
                    onClose.push(std::move(argOnClose));
                }
            }

            ~LuaThreadHandle()
            {
                while(!onClose.empty()){
                    if(onClose.top()){
                        onClose.top()(); // only do clean work, don't modify onClose stack inside
                    }
                    onClose.pop();
                }
            }
        };

    protected:
        ActorPod * const m_actorPod;

    protected:
        LuaThreadHandle *m_currRunner = nullptr;

    private:
        uint64_t m_seqID = 1;
        std::unordered_multimap<uint64_t, LuaThreadHandle> m_runnerList;

    public:
        ServerLuaCoroutineRunner(ActorPod *);

    public:
        // start a thread to run lua code
        // if need to pass compound lua struture as args, use cerealf::base64_serialize() in c++ world and decode by base64Decode in lua world, i.e.:
        //
        // in c++:
        //
        //    luaf::luaVar var = create_complicated_lua_var();
        //    spawn(threadKey, str_printf("lua_func(%s)", luaf::quotedLuaString(cerealf::base64_serialize(var).c_str())), ...);
        //
        // in lua:
        //
        //    function lua_func(base64_var)
        //        local var = base64Decode(var)
        //        ...
        //    end
        //
        // use luaf::quotedLuaString() to quote a string to be a lua string literal
        std::pair<uint64_t, uint64_t> spawn(uint64_t, std::pair<uint64_t, uint64_t>, const std::string   &, luaf::luaVar = {});
        std::pair<uint64_t, uint64_t> spawn(uint64_t,                                const std::string   &, luaf::luaVar = {}, std::function<void(const sol::protected_function_result &)> = nullptr, std::function<void()> = nullptr);
        std::pair<uint64_t, uint64_t> spawn(uint64_t,                                const sol::function &,                    std::function<void(const sol::protected_function_result &)> = nullptr, std::function<void()> = nullptr);

    public:
        std::vector<uint64_t> getSeqID(uint64_t, std::vector<uint64_t> * = nullptr) const;

    public:
        void close (uint64_t, uint64_t = 0);
        void resume(uint64_t, uint64_t = 0);

    public:
        LuaThreadHandle *hasKey(uint64_t, uint64_t = 0);

    private:
        void resumeRunner(LuaThreadHandle *, std::optional<std::pair<std::string, luaf::luaVar>> = {});

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
                return [func = std::function(std::forward<Func>(func)), this](typename _extractLambdaUserArgsAsTuple<Func>::type args, sol::function cb, sol::this_state s)
                {
                    fflassert(s.lua_state());
                    const LuaCoopCallDoneFlag doneFlag;

                    if constexpr (std::is_same_v<LuaCoopState, typename _extractLambdaSecondArg<Func>::type>){
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, m_currRunner, cb, doneFlag), LuaCoopState(s)), std::move(args)));
                    }
                    else{
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, m_currRunner, cb, doneFlag)), std::move(args)));
                    }
                };
            }(std::forward<Func>(func)));
        }
};
