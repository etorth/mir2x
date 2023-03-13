#pragma once
#include <functional>
#include <sol/sol.hpp>
#include "serverluamodule.hpp"

class ActorPod;
class ServerLuaCoroutineRunner: public ServerLuaModule
{
    private:
        struct _CoroutineRunner
        {
            // for this class
            // the terms: coroutine, runner, thread means same

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
            const uint64_t reqUID;      // who requested to call spawn(), zero means self
            const uint64_t msgSeqID;    // respond to which actor callback in reqUID

            // mutable area for event polling
            // UID_A send code to UID_B, and creates this coroutine in UID_B
            // during executing of given code, UID_B may call UID_C by uidExecute(UID_C, xxx), then below ``from" assigned as UID_C

            uint64_t from = 0;          // where event comes from
            std::string event {};
            std::string value {};

            sol::thread runner;
            sol::coroutine callback;

            _CoroutineRunner(ServerLuaModule &luaModule, uint64_t argKey, uint64_t argSeqID, uint64_t argReqUID, uint64_t argMsgSeqID)
                : key(argKey)
                , seqID(argSeqID)
                , reqUID(argReqUID)
                , msgSeqID(argMsgSeqID)
                , runner(sol::thread::create(luaModule.getLuaState().lua_state()))
                , callback(sol::state_view(runner.state())["_RSVD_NAME_luaCoroutineRunner_main"])
            {
                fflassert(key);
                fflassert(seqID);

                if(reqUID) fflassert( msgSeqID, reqUID, msgSeqID);
                else       fflassert(!msgSeqID, reqUID, msgSeqID);
            }

            void clearEvent()
            {
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
        uint64_t spawn(uint64_t, uint64_t, uint64_t, const char *);

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
};
