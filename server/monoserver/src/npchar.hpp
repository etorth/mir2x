/*
 * =====================================================================================
 *
 *       Filename: npchar.hpp
 *        Created: 04/12/2020 15:53:55
 *    Description: 
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
#include <cstdint>
#include "servicecore.hpp"
#include "servermap.hpp"
#include "charobject.hpp"

class NPChar final: public CharObject
{
    private:
        class LuaNPCModule: public ServerLuaModule
        {
            private:
                struct LuaNPCSession
                {
                    uint64_t uid;
                    std::string event;
                    std::string value;

                    LuaNPCModule *module;
                    sol::coroutine co_handler;
                };


            private:
                std::unordered_map<uint64_t, LuaNPCSession> m_sessionList;

            private:
                NPChar *m_NPChar;

            public:
                LuaNPCModule(NPChar *);

            public:
                void setEvent(uint64_t uid, std::string event, std::string value);

            public:
                void close(uint64_t uid)
                {
                    m_sessionList.erase(uid);
                }
        };

    private:
        int m_dirIndex;
        LuaNPCModule m_luaModule;

    public:
        NPChar(uint16_t, ServiceCore *, ServerMap *, int, int, int);

    public:
        bool Update() override;
        bool InRange(int, int, int) override;

    public:
        void ReportCORecord(uint64_t) override;

    public:
        bool DCValid(int, bool) override;
        DamageNode GetAttackDamage(int) override;

    public:
        bool StruckDamage(const DamageNode &) override;

    public:
        bool GoDie() override;
        bool GoGhost() override;

    public:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void On_MPK_ACTION(const MessagePack &);
        void On_MPK_NPCEVENT(const MessagePack &);
        void On_MPK_NOTIFYNEWCO(const MessagePack &);
        void On_MPK_QUERYCORECORD(const MessagePack &);
        void On_MPK_QUERYLOCATION(const MessagePack &);

    private:
        void sendXMLLayout(uint64_t, const char *);

    public:
        void OperateAM(const MessagePack &) override;
};
