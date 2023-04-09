#pragma once
#include <functional>
#include <sol/sol.hpp>
#include "totype.hpp"
#include "serverluamodule.hpp"

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

            // mutable area for event polling
            // UID_A send code to UID_B, and creates this coroutine in UID_B
            // during executing of given code, UID_B may call UID_C by uidExecute(UID_C, xxx), then below ``from" assigned as UID_C

            uint64_t from = 0;          // where event comes from
            std::string event {};
            std::string value {};

            sol::thread runner;
            sol::coroutine callback;

            _CoroutineRunner(ServerLuaModule &argLuaModule, uint64_t argKey, uint64_t argSeqID, std::function<void(const sol::protected_function_result &)> argOnDone)
                : key(argKey)
                , seqID(argSeqID)
                , onDone(std::move(argOnDone))
                , runner(sol::thread::create(argLuaModule.getLuaState().lua_state()))
                , callback(sol::state_view(runner.state())["_RSVD_NAME_luaCoroutineRunner_main"])
            {
                fflassert(key);
                fflassert(seqID);
            }

            void clearEvent()
            {
                this->from = 0;
                this->event.clear();
                this->value.clear();
            }
        };

    private:
        ActorPod * const m_actorPod;

    private:
        uint64_t m_seqID = 1;
        std::unordered_map<uint64_t, std::unique_ptr<_CoroutineRunner>> m_runnerList;

    public:
        ServerLuaCoroutineRunner(ActorPod *, std::function<void(ServerLuaModule *)> = nullptr);

    public:
        uint64_t spawn(uint64_t, std::pair<uint64_t, uint64_t>, const std::string &);
        uint64_t spawn(uint64_t,                                const std::string &, std::function<void(const sol::protected_function_result &)> = nullptr);

    public:
        void close(uint64_t key)
        {
            m_runnerList.erase(key);
        }

        void resume(uint64_t key)
        {
            if(auto p = m_runnerList.find(key); p != m_runnerList.end()){
                resumeRunner(p->second.get());
            }
            else{
                throw fflerror("resume non-existing coroutine: key = %llu", to_llu(key));
            }
        }

        uint64_t getSeqID(uint64_t key) const
        {
            if(auto p = m_runnerList.find(key); p != m_runnerList.end()){
                return p->second->seqID;
            }
            else{
                return 0;
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

    public:
        template<typename R, typename... Args> void bindCoop(const std::string &funcName, std::function<R(Args..., std::function<void()>, std::function<void()>)> func, std::function<void(sol::function)> onOK = nullptr, std::function<void(sol::function)> onError = nullptr)
        {
            fflassert(str_haschar(funcName));
            fflassert(func);

            bindFunction(funcName + "Coop", [func = std::move(func), onOK = std::move(onOK), onError = std::move(onError), this]()
            {
                return [func = std::move(func), onOK = std::move(onOK), onError = std::move(onError), this](Args... args, sol::function onLuaOK, sol::function onLuaError, uint64_t runnerSeqID)
                {
                    const CallDoneFlag doneFlag;
                    func(std::forward<Args>(args)..., [doneFlag, onOK = std::move(onOK), onLuaOK = std::move(onLuaOK), runnerSeqID, this]()
                    {
                        if(onOK){
                            onOK(onLuaOK);
                        }
                        else{
                            onLuaOK();
                        }

                        if(doneFlag){
                            resume(runnerSeqID);
                        }
                    },

                    [doneFlag, onError = std::move(onError), onLuaError = std::move(onLuaError), runnerSeqID, this]()
                    {
                        if(onError){
                            onError(onLuaError);
                        }
                        else{
                            onLuaError();
                        }

                        if(doneFlag){
                            resume(runnerSeqID);
                        }
                    });
                };
            }());
        }
};
