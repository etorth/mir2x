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
        void operateAM(const MessagePack &);
        void operateNet(uint32_t, uint8_t, const uint8_t *, size_t);

    protected:
        void loadMap(uint32_t);
        const ServerMap *retrieveMap(uint32_t);

    public:
        uint64_t activate() override;

    private:
        void on_MPK_LOGIN(const MessagePack &);
        void on_MPK_METRONOME(const MessagePack &);
        void on_MPK_BADCHANNEL(const MessagePack &);
        void on_MPK_NETPACKAGE(const MessagePack &);
        void on_MPK_QUERYMAPUID(const MessagePack &);
        void on_MPK_QUERYMAPLIST(const MessagePack &);
        void on_MPK_QUERYCOCOUNT(const MessagePack &);
        void on_MPK_ADDCHAROBJECT(const MessagePack &);

    private:
        void net_CM_Login(uint32_t, uint8_t, const uint8_t *, size_t);
};
