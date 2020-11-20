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

                    struct LuaCORunner
                    {
                        sol::thread runner;
                        sol::coroutine callback;
                    } co_handler;
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
        bool update() override;
        bool InRange(int, int, int) override;

    public:
        void ReportCORecord(uint64_t) override;

    public:
        bool DCValid(int, bool) override;
        DamageNode GetAttackDamage(int) override;

    public:
        bool StruckDamage(const DamageNode &) override;

    public:
        bool goDie() override;
        bool goGhost() override;

    public:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void on_MPK_ACTION(const MessagePack &);
        void on_MPK_NPCEVENT(const MessagePack &);
        void on_MPK_NOTIFYNEWCO(const MessagePack &);
        void on_MPK_BADACTORPOD(const MessagePack &);
        void on_MPK_QUERYCORECORD(const MessagePack &);
        void on_MPK_QUERYLOCATION(const MessagePack &);

    private:
        void sendQuery(uint64_t, const std::string &);
        void sendXMLLayout(uint64_t, const char *);

    public:
        void operateAM(const MessagePack &) override;
};
