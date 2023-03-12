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
            const uint64_t key;
            const uint64_t seqID;
            const uint64_t fromUID;
            const uint64_t msgSeqID;

            std::string event {};
            std::string value {};

            sol::thread runner;
            sol::coroutine callback;

            _CoroutineRunner(ServerLuaModule &luaModule, uint64_t argKey, uint64_t argSeqID, uint64_t argFromUID, uint64_t argMsgSeqID)
                : key(argKey)
                , seqID(argSeqID)
                , fromUID(argFromUID)
                , msgSeqID(argMsgSeqID)
                , runner(sol::thread::create(luaModule.getLuaState().lua_state()))
                , callback(sol::state_view(runner.state())["_RSVD_NAME_luaCoroutineRunner_main"])
            {
                fflassert(key);
                fflassert(seqID);

                if(fromUID) fflassert( msgSeqID, fromUID, msgSeqID);
                else        fflassert(!msgSeqID, fromUID, msgSeqID);
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
        void spawn(uint64_t, uint64_t, uint64_t, const char *);

    public:
        void clear(uint64_t key)
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

        uint64_t runnerSeqID(uint64_t key) const
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
