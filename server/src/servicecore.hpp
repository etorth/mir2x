/*
 * =====================================================================================
 *
 *       Filename: servicecore.hpp
 *        Created: 04/22/2016 17:59:06
 *    Description: split monoserver into actor-code and non-actor code
 *                 put all actor code in this class
 *
 *                 TODO & TBD
 *                 everytime when creating a lambda in an actor to use ThreadPN to
 *                 invoke, never use [this, ...] since this will access the internal
 *                 state from another thread
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <map>
#include <vector>
#include <unordered_map>

#include "netdriver.hpp"
#include "serverobject.hpp"
#include "serverluamodule.hpp"

class ServerMap;
class ServiceCore final: public ServerObject
{
    protected:
        std::map<uint32_t, ServerMap *> m_mapList;

    public:
        ServiceCore();
        ~ServiceCore() = default;

    protected:
        void operateAM(const ActorMsgPack &);
        void operateNet(uint32_t, uint8_t, const uint8_t *, size_t);

    protected:
        void loadMap(uint32_t);
        const ServerMap *retrieveMap(uint32_t);

    public:
        void onActivate() override;

    private:
        void on_AM_LOGIN(const ActorMsgPack &);
        void on_AM_METRONOME(const ActorMsgPack &);
        void on_AM_BADCHANNEL(const ActorMsgPack &);
        void on_AM_RECVPACKAGE(const ActorMsgPack &);
        void on_AM_QUERYMAPUID(const ActorMsgPack &);
        void on_AM_QUERYMAPLIST(const ActorMsgPack &);
        void on_AM_QUERYCOCOUNT(const ActorMsgPack &);
        void on_AM_ADDCO(const ActorMsgPack &);

    private:
        void net_CM_LOGIN  (uint32_t, uint8_t, const uint8_t *, size_t);
        void net_CM_ACCOUNT(uint32_t, uint8_t, const uint8_t *, size_t);
};
