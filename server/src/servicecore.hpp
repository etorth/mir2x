#pragma once
#include <string>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "peercore.hpp"

class PeerCore;
class ServerMap;
class ServiceCore final: public PeerCore
{
    private:
        friend class PeerCore;

    private:
        struct LoadMapOp
        {
            std::vector<std::function<void(bool)>> onDone  {}; // bool: newly_loaded
            std::vector<std::function<void(    )>> onError {};
        };
        std::unordered_map<uint64_t, LoadMapOp> m_loadMapPendingOps;

    private:
        std::unordered_map<uint32_t, std::pair<uint32_t, bool>> m_dbidList; // channID -> {dbid, online}

    private:
        std::unordered_map<uint64_t, SDRegisterQuest> m_questList;
        std::unordered_map<int, std::unordered_set<uint64_t>> m_questTriggerList;

    public:
        ServiceCore();
        ~ServiceCore() = default;

    protected:
        void operateAM(const ActorMsgPack &) override;
        void operateNet(uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);

    protected:
        void requestLoadMap(uint64_t, std::function<void(bool)> = nullptr, std::function<void()> = nullptr);

    private:
        ServerGuard *addGuard  (const SDInitGuard   &);
        Player      *addPlayer (const SDInitPlayer  &);
        NPChar      *addNPChar (const SDInitNPChar  &);
        Monster     *addMonster(const SDInitMonster &);

    public:
        void onActivate() override;

    private:
        std::optional<std::pair<uint32_t, bool>> findDBID(uint32_t channID) const;

    private:
        void on_AM_REGISTERQUEST         (const ActorMsgPack &);
        void on_AM_METRONOME             (const ActorMsgPack &);
        void on_AM_BADCHANNEL            (const ActorMsgPack &);
        void on_AM_RECVPACKAGE           (const ActorMsgPack &);
        void on_AM_LOADMAP               (const ActorMsgPack &);
        void on_AM_QUERYMAPLIST          (const ActorMsgPack &);
        void on_AM_QUERYCOCOUNT          (const ActorMsgPack &);
        void on_AM_MODIFYQUESTTRIGGERTYPE(const ActorMsgPack &);
        void on_AM_QUERYQUESTTRIGGERLIST (const ActorMsgPack &);
        void on_AM_QUERYQUESTUID         (const ActorMsgPack &);
        void on_AM_QUERYQUESTUIDLIST     (const ActorMsgPack &);

    private:
        void net_CM_LOGIN         (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_ONLINE        (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYCHAR     (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_CREATECHAR    (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_DELETECHAR    (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_CREATEACCOUNT (uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_CHANGEPASSWORD(uint32_t, uint8_t, const uint8_t *, size_t, uint64_t);
};
