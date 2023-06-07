#pragma once
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
class ServerLuaCoroutineRunner: public ServerLuaModule
{
    private:
        struct _CoroutineRunner
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
            std::vector<luaf::luaVar> notifyList;

            _CoroutineRunner(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, std::function<void(const sol::protected_function_result &)> argOnDone, std::function<void()> argOnClose)
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
        };

    private:
        ActorPod * const m_actorPod;

    private:
        uint64_t m_seqID = 1;
        std::unordered_map<uint64_t, std::unique_ptr<_CoroutineRunner>> m_runnerList;

    public:
        ServerLuaCoroutineRunner(ActorPod *);

    public:
        uint64_t spawn(uint64_t, std::pair<uint64_t, uint64_t>, const std::string &);
        uint64_t spawn(uint64_t,                                const std::string &, std::function<void(const sol::protected_function_result &)> = nullptr, std::function<void()> = nullptr);

    public:
        uint64_t getSeqID(uint64_t key) const
        {
            if(auto p = m_runnerList.find(key); p != m_runnerList.end()){
                return p->second->seqID;
            }
            else{
                return 0;
            }
        }

    public:
        void close(uint64_t key, uint64_t seqID = 0)
        {
            if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
                while(!p->second->onClose.empty()){
                    if(p->second->onClose.top()){
                        p->second->onClose.top()(); // only do clean work, don't modify onClose stack inside
                    }
                    p->second->onClose.pop();
                }
                m_runnerList.erase(p);
            }
        }

        void resume(uint64_t key, uint64_t seqID = 0)
        {
            if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
                resumeRunner(p->second.get());
            }
            else{
                // won't throw here
                // if needs to confirm the coroutine exists, use hasKey() first
            }
        }

        bool hasKey(uint64_t key, uint64_t seqID = 0) const
        {
            const auto p = m_runnerList.find(key);
            return (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID);
        }

    public:
        void addNotify(uint64_t key, uint64_t seqID, std::vector<luaf::luaVar> notify)
        {
            if(auto p = m_runnerList.find(key); (p != m_runnerList.end()) && (seqID == 0 || p->second->seqID == seqID)){
                p->second->notifyList.push_back(luaf::buildLuaVar(std::move(notify)));
            }
        }

    private:
        void resumeRunner(_CoroutineRunner *, std::optional<std::string> = {});

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
