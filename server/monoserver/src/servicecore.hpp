/*
 * =====================================================================================
 *
 *       Filename: servicecore.hpp
 *        Created: 04/22/2016 17:59:06
 *  Last Modified: 06/16/2017 14:25:38
 *
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

#include "netpod.hpp"
#include "activeobject.hpp"

class ServerMap;
class ServiceCore: public ActiveObject
{
    protected:
        std::map<uint32_t, ServerMap *> m_MapRecord;

    public:
        ServiceCore();
       ~ServiceCore() = default;

    protected:
        void Operate(const MessagePack &, const Theron::Address &);
        void OperateNet(uint32_t, uint8_t, const uint8_t *, size_t);

    protected:
        bool LoadMap(uint32_t);
        const ServerMap *RetrieveMap(uint32_t);

    private:
        void On_MPK_LOGIN(const MessagePack &, const Theron::Address &);
        void On_MPK_NETPACKAGE(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYMAPUID(const MessagePack &, const Theron::Address &);
        void On_MPK_TRYMAPSWITCH(const MessagePack &, const Theron::Address &);
        void On_MPK_LOGINQUERYDB(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYMAPLIST(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYCOCOUNT(const MessagePack &, const Theron::Address &);
        void On_MPK_NEWCONNECTION(const MessagePack &, const Theron::Address &);
        void On_MPK_ADDCHAROBJECT(const MessagePack &, const Theron::Address &);
        void On_MPK_QUERYMONSTERGINFO(const MessagePack &, const Theron::Address &);

    private:
        void Net_CM_Login(uint32_t, uint8_t, const uint8_t *, size_t);
};
