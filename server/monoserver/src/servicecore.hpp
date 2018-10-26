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
        std::map<uint32_t, ServerMap *> m_MapList;

    public:
        ServiceCore();
       ~ServiceCore() = default;

    protected:
        void OperateAM(const MessagePack &);
        void OperateNet(uint32_t, uint8_t, const uint8_t *, size_t);

    protected:
        bool LoadMap(uint32_t);
        const ServerMap *RetrieveMap(uint32_t);

    private:
        void On_MPK_LOGIN(const MessagePack &);
        void On_MPK_METRONOME(const MessagePack &);
        void On_MPK_BADCHANNEL(const MessagePack &);
        void On_MPK_NETPACKAGE(const MessagePack &);
        void On_MPK_QUERYMAPUID(const MessagePack &);
        void On_MPK_TRYMAPSWITCH(const MessagePack &);
        void On_MPK_QUERYMAPLIST(const MessagePack &);
        void On_MPK_QUERYCOCOUNT(const MessagePack &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &);

    private:
        void Net_CM_Login(uint32_t, uint8_t, const uint8_t *, size_t);
};
