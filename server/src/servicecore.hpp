#pragma once
#include <memory>
#include <string>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "peercore.hpp"

class PeerCore;
class ServerMap;
class EnableAddCO;
class ServiceCore final: public PeerCore
{
    private:
        friend class PeerCore;
        friend class EnableAddCO;

    private:
        struct RegisterLoadMapOpAwaiter
        {
            ServiceCore * const core;
            uint64_t      const mapUID;

            bool await_ready() const
            {
                return false;
            }

            void await_suspend(std::coroutine_handle<> h)
            {
                core->m_loadMapPendingOps[mapUID].push_back(h);
            }

            std::pair<bool, bool> await_resume() const
            {
                return {core->m_mapList.contains(mapUID), true};
            }
        };

    private:
        std::unordered_map<uint64_t, std::vector<std::coroutine_handle<>>> m_loadMapPendingOps;

    private:
        std::unordered_map<uint32_t, std::pair<uint32_t, bool>> m_dbidList; // channID -> {dbid, online}

    private:
        std::unordered_map<uint64_t, SDRegisterQuest> m_questList;
        std::unordered_map<int, std::unordered_set<uint64_t>> m_questTriggerList;

    private:
        std::unique_ptr<EnableAddCO> m_addCO;

    public:
        ServiceCore();
        ~ServiceCore() = default;

    protected:
        corof::awaitable<> onActorMsg(const ActorMsgPack &) override;
        corof::awaitable<> operateNet(uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);

    protected:
        corof::awaitable<std::pair<bool, bool>> requestLoadMap(uint64_t);

    public:
        corof::awaitable<> onActivate() override;

    private:
        std::optional<std::pair<uint32_t, bool>> findDBID(uint32_t channID) const;

    private:
        corof::awaitable<> on_AM_REGISTERQUEST         (const ActorMsgPack &);
        corof::awaitable<> on_AM_BADCHANNEL            (const ActorMsgPack &);
        corof::awaitable<> on_AM_RECVPACKAGE           (const ActorMsgPack &);
        corof::awaitable<> on_AM_LOADMAP               (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYMAPLIST          (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYCOCOUNT          (const ActorMsgPack &);
        corof::awaitable<> on_AM_MODIFYQUESTTRIGGERTYPE(const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYQUESTTRIGGERLIST (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYQUESTUID         (const ActorMsgPack &);
        corof::awaitable<> on_AM_QUERYQUESTUIDLIST     (const ActorMsgPack &);

    private:
        corof::awaitable<> net_CM_LOGIN         (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        corof::awaitable<> net_CM_ONLINE        (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        corof::awaitable<> net_CM_QUERYCHAR     (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        corof::awaitable<> net_CM_CREATECHAR    (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        corof::awaitable<> net_CM_DELETECHAR    (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        corof::awaitable<> net_CM_CREATEACCOUNT (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        corof::awaitable<> net_CM_CHANGEPASSWORD(uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
};
