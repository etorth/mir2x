#pragma once
#include <deque>
#include <concepts>
#include <functional>
#include <type_traits>
#include <sol/sol.hpp>
#include "sysconst.hpp"
#include "stdf.hpp"
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
        // keeps the calling lua thread alive till the coop ends, the thread can be closed while the coop is pending
        // declared before m_callback, which refers to the thread, so it's destroyed after m_callback
        sol::main_reference m_luaThread;

    private:
        sol::function m_callback;

    public:
        LuaCoopResumer(ServerLuaCoroutineRunner *, void *, sol::function, sol::this_state);

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

            // the timer of the timeout of the waitNotify() the thread waits in, see _RSVD_NAME_waitNotify
            std::optional<std::pair<uint64_t, uint64_t>> notifyTimer;

            // the thread to start once this one is closed, see closeThreadThenRun() in serverluacoroutinerunner.lua
            // a main_function, the function came from this thread, which can be freed by the time the new one starts
            std::optional<std::pair<uint64_t, sol::main_function>> afterClose;

            // onStack       : thread has frames on the C stack, it can't be resumed, closing it only sets closeRequested
            // closeRequested: asked to close while onStack, resumeRunner() closes it at its next yield
            // closing       : lua_closethread() is running its <close> handlers
            bool onStack = false;
            bool closeRequested = false;
            bool closing = false;

            LuaThreadHandle(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, std::function<void(const sol::protected_function_result &)> argOnDone, std::function<void()> argOnClose)
                : key(argKey)
                , seqID(argSeqID)
                , onDone(std::move(argOnDone))
                , runner(sol::thread::create(argLuaModule.getState().lua_state()))
                , callback(sol::state_view(runner.state())["_RSVD_NAME_luaCoroutineRunner_codeMain"])
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

        struct LuaEvalAwaitable
        {
            const bool ready;
            std::coroutine_handle<> *handle;

            bool await_ready() const noexcept
            {
                return ready;
            }

            void await_suspend(std::coroutine_handle<> h)
            {
                *handle = h;
            }

            void await_resume() const noexcept {}
        };

    protected:
        ActorPod * const m_actorPod;

    protected:
        LuaThreadHandle *m_currRunner = nullptr;

    private:
        // the thread whose <close> handlers lua_closethread() is running, see closeLuaThread()
        LuaThreadHandle *m_closingRunner = nullptr;

    private:
        uint64_t m_seqID = 1;
        std::unordered_multimap<uint64_t, LuaThreadHandle> m_runnerList;

    public:
        ServerLuaCoroutineRunner(ActorPod *);

    private:
        bool doSpawn(std::pair<uint64_t, uint64_t>, const std::string   &, luaf::luaVar, std::function<void(const sol::protected_function_result &)>, std::function<void()>);
        bool doSpawn(std::pair<uint64_t, uint64_t>, const sol::function &,               std::function<void(const sol::protected_function_result &)>, std::function<void()>);

    private:
        template<typename... Args> corof::awaitable<std::vector<luaf::luaVar>> evalImpl(uint64_t, Args && ...);

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
        corof::awaitable<std::vector<luaf::luaVar>> eval(uint64_t, const std::string   &, luaf::luaVar = {});
        corof::awaitable<std::vector<luaf::luaVar>> eval(uint64_t, const sol::function &                   );

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

    protected:
        // queues a notify for thread {key, seqID}, and resumes the thread if it waits in waitNotify()
        void addNotify(uint64_t, uint64_t, luaf::luaVar);

    protected:
        // timers of pause() and waitNotify(), fnOnTimer(timeout) is called once, with false if the timer got cancelled
        // virtual so a test without an actor pool can run them by hand
        virtual std::pair<uint64_t, uint64_t> addTimer(uint64_t, std::function<void(bool)>);
        virtual void cancelTimer(const std::pair<uint64_t, uint64_t> &);

    private:
        bool resumeRunner(LuaThreadHandle *, std::optional<std::pair<std::string, luaf::luaVar>> = {});

    private:
        // resumes a thread waiting in waitNotify(), for a notify or for its timeout, whichever comes first
        void resumeNotifyWaiter(LuaThreadHandle *);

    private:
        // spawns func as runThread() in serverluacoroutinerunner.lua does, an error of the thread is only logged
        std::pair<uint64_t, uint64_t> runThread(uint64_t, const sol::function &);

    private:
        // closeLuaThread(): lua side only, runs the <close> handlers
        // eraseRunner()   : C++ side only, takes the handle out of m_runnerList and runs its onClose callbacks
        // closeRunner()   : both, or only sets closeRequested if the thread is onStack
        int  closeLuaThread(LuaThreadHandle *);
        void closeRunner   (LuaThreadHandle *);
        void eraseRunner   (const std::pair<uint64_t, uint64_t> &);

    private:
        // nullptr if the lua code running on the given state can end m_currRunner by a yield, else why not
        const char *selfCloseError(lua_State *) const;

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
                        throw fflpanic("calling {}() without a spawned runner", to_cstr(funcName));
                    }

                    fflassert(s.lua_state());

                    m_currRunner->needResume = false;
                    const auto callDoneSg = stdf::guard([this](){ m_currRunner->needResume = true; });

                    if constexpr (std::is_same_v<LuaCoopState, typename _extractLambdaThirdArg<Func>::type>){
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, m_currRunner, cb, s), LuaCoopState(s)), std::move(args))).resume();
                    }
                    else{
                        std::apply(func, std::tuple_cat(std::tuple(LuaCoopResumer(this, m_currRunner, cb, s)), std::move(args))).resume();
                    }
                };
            }(std::forward<Func>(func)));
        }
};
