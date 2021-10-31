#pragma once
#include <string>
#include <unordered_map>
#include "serverobject.hpp"

class ServerMap;
class ServiceCore final: public ServerObject
{
    private:
        std::unordered_map<uint32_t, ServerMap *> m_mapList;

    private:
        std::unordered_map<uint32_t, uint32_t> m_dbidList; // channID -> dbid

    public:
        ServiceCore();
        ~ServiceCore() = default;

    protected:
        void operateAM(const ActorMsgPack &) override;
        void operateNet(uint32_t, uint8_t, const uint8_t *, size_t);

    protected:
        void loadMap(uint32_t);
        const ServerMap *retrieveMap(uint32_t);

    public:
        void onActivate() override;

    private:
        std::optional<uint32_t> findDBID(uint32_t channID) const;

    private:
        void on_AM_LOGIN(const ActorMsgPack &);
        void on_AM_METRONOME(const ActorMsgPack &);
        void on_AM_BADCHANNEL(const ActorMsgPack &);
        void on_AM_RECVPACKAGE(const ActorMsgPack &);
        void on_AM_LOADMAP(const ActorMsgPack &);
        void on_AM_QUERYMAPLIST(const ActorMsgPack &);
        void on_AM_QUERYCOCOUNT(const ActorMsgPack &);
        void on_AM_ADDCO(const ActorMsgPack &);

    private:
        void net_CM_LOGIN         (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_ONLINE        (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYCHAR     (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_CREATECHAR    (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_DELETECHAR    (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_CREATEACCOUNT (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_CHANGEPASSWORD(uint32_t, uint8_t, const uint8_t *, size_t);
};
