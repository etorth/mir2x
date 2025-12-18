#pragma once
#include <deque>
#include <concepts>
#include <functional>
#include <type_traits>
#include <sol/sol.hpp>
#include "sysconst.hpp"
#include "sgf.hpp"
#include "luaf.hpp"
#include "corof.hpp"
#include "totype.hpp"
#include "serverluamodule.hpp"

class ServerLuaCoroutineRunner;
class LuaCoopResumer final
{
    private:
        ServerLuaCoroutineRunner * const m_luaRunner;

    private:
        void * const m_currRunner;

    private:
        sol::function m_callback;

    public:
        LuaCoopResumer(ServerLuaCoroutineRunner *, void *, sol::function);

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
            resumeYieldedRunner(m_luaRunner, m_currRunner);
        }

    public:
        void pushOnClose(std::function<void()>) const;
        void  popOnClose()                      const;

    private:
        static void resumeYieldedRunner(ServerLuaCoroutineRunner *, void *); // resolve dependency
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
            // if needResume is true, means the lua layer has been yielded

            bool needResume = false;
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
            std::deque<luaf::luaVar> notifyList; // sender called table.pack(...) before pushed into this list

            LuaThreadHandle(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, std::function<void(const sol::protected_function_result &)> argOnDone, std::function<void()> argOnClose)
                : key(argKey)
                , seqID(argSeqID)
                , onDone(std::move(argOnDone))
                , runner(sol::thread::create(argLuaModule.getState().lua_state()))
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
                , runner(sol::thread::create(argLuaModule.getState().lua_state()))
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

            std::pair<uint64_t, uint64_t> keyPair() const
            {
                return {key, seqID};
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
        void close (const std::pair<uint64_t, uint64_t> &kp) { close (kp.first, kp.second); }
        void resume(const std::pair<uint64_t, uint64_t> &kp) { resume(kp.first, kp.second); }

    public:
        LuaThreadHandle *hasKey(uint64_t, uint64_t = 0);
        LuaThreadHandle *hasKeyPair(const std::pair<uint64_t, uint64_t> &kp)
        {
            return hasKey(kp.first, kp.second);
        }

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
        template<typename Lambda, typename... Args> static std::tuple<Args...> _extractLambdaUserArgsHelper(corof::awaitable<> (*)(Lambda, LuaCoopResumer, Args...));
        template<typename Lambda, typename... Args> static std::tuple<Args...> _extractLambdaUserArgsHelper(corof::awaitable<> (*)(Lambda, LuaCoopResumer, LuaCoopState, Args...));

        template<typename Lambda                                 > static void _extractLambdaThirdArgHelper(corof::awaitable<> (*)(Lambda, LuaCoopResumer));
        template<typename Lambda, typename Arg2, typename... Args> static Arg2 _extractLambdaThirdArgHelper(corof::awaitable<> (*)(Lambda, LuaCoopResumer, Arg2, Args...));

        template<typename Lambda> struct _extractLambdaUserArgsAsTuple
        {
            using type = decltype(_extractLambdaUserArgsHelper(&Lambda:: template operator()<Lambda>));
        };

        template<typename Lambda> struct _extractLambdaThirdArg
        {
            using type = decltype(_extractLambdaThirdArgHelper(&Lambda:: template operator()<Lambda>));
        };

    public:
        template<typename Func> void bindCoop(std::string funcName, Func && func)
        {
            fflassert(str_haschar(funcName));
            bindFunction(funcName + SYS_COOP, [funcName, this](auto && func)
            {
                return [funcName, func = std::forward<Func>(func), this](typename _extractLambdaUserArgsAsTuple<Func>::type args, sol::function cb, sol::this_state s)
                {
                    if(!m_currRunner){
                        throw fflerror("calling %s() without a spawned runner", to_cstr(funcName));
                    }

                    fflassert(s.lua_state());

                    m_currRunner->needResume = false;
                    const auto callDoneSg = sgf::guard([this](){ m_currRunner->needResume = true; });

                    if constexpr (std::is_same_v<LuaCoopState, typename _extractLambdaThirdArg<Func>::type>){
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, m_currRunner, cb), LuaCoopState(s)), std::move(args))).resume();
                    }
                    else{
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, m_currRunner, cb)), std::move(args))).resume();
                    }
                };
            }(std::forward<Func>(func)));
        }
};
