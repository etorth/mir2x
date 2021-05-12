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
#include <memory>
#include <cstdint>
#include "servicecore.hpp"
#include "servermap.hpp"
#include "charobject.hpp"

class NPChar final: public CharObject
{
    public:
        class LuaNPCModule: public ServerLuaModule
        {
            private:
                friend class NPChar;

            private:
                struct LuaCallStack
                {
                    uint64_t from = 0;
                    std::string event;
                    std::string value;

                    // scenario why adding seqID:
                    // 1. received an event which triggers processNPCEvent(event)
                    // 2. inside processNPCEvent(event) the script emits query to other actor
                    // 3. when waiting for the response of the query, user clicked the close button or click init button to end up the current call stack
                    // 4. receives the query response, we should ignore it
                    //
                    // to fix this we have to give every call stack an uniq seqID
                    // and the query response needs to match the seqID

                    const uint64_t seqID;
                    sol::thread co_runner;
                    sol::coroutine co_callback;

                    LuaCallStack(LuaNPCModule *luaModulePtr)
                        : seqID(luaModulePtr->peekSeqID())
                        , co_runner(sol::thread::create(luaModulePtr->getLuaState().lua_state()))
                        , co_callback(sol::state_view(co_runner.state())["main"])
                    {}
                };

                struct NPCGLoc
                {
                    int   x = -1;
                    int   y = -1;
                    int dir = -1;
                };

            private:
                const std::string m_npcName;

            private:
                uint64_t m_seqID = 0;
                std::unordered_map<uint64_t, LuaCallStack> m_callStackList;

            private:
                int m_npcLookID = -1;               // setNPCLook
                NPCGLoc m_npcGLoc;                  // setNPCGLoc
                std::set<uint32_t> m_npcSell;       // setNPCSell

            private:
                NPChar *m_npc = nullptr;

            public:
                LuaNPCModule(const SDInitNPChar &);

            public:
                void setEvent(uint64_t callStackUID, uint64_t from, std::string event, std::string value);

            public:
                void close(uint64_t uid)
                {
                    m_callStackList.erase(uid);
                }

            public:
                uint64_t peekSeqID()
                {
                    m_seqID++;
                    if(m_seqID == 0){
                        m_seqID = 1;
                    }
                    return m_seqID;
                }

                uint64_t getCallStackSeqID(uint64_t callStackUID) const
                {
                    if(const auto p = m_callStackList.find(callStackUID); p != m_callStackList.end()){
                        return p->second.seqID;
                    }
                    else{
                        return 0;
                    }
                }

            public:
                const auto &getNPCName() const
                {
                    return m_npcName;
                }

                const auto &getNPCGLoc() const
                {
                    return m_npcGLoc;
                }

                uint16_t getNPCLookID() const
                {
                    return m_npcLookID;
                }

                const auto &getNPCSell() const
                {
                    return m_npcSell;
                }

            public:
                void bindNPC(NPChar *npc)
                {
                    fflassert(npc && !m_npc);
                    m_npc = npc;
                }
        };

    private:
        struct SellItem
        {
            SDItem item;
            bool locked = false;
            std::vector<SDCostItem> costList;
        };

    private:
        std::unique_ptr<LuaNPCModule> m_luaModulePtr;
        std::unordered_map<uint32_t, std::map<uint32_t, SellItem>> m_sellItemList;

    public:
        NPChar(const ServerMap *, std::unique_ptr<NPChar::LuaNPCModule>);

    public:
        bool update() override;
        bool InRange(int, int, int) override;

    public:
        void reportCO(uint64_t) override;

    public:
        bool DCValid(int, bool) override;
        DamageNode getAttackDamage(int) override;

    public:
        bool struckDamage(const DamageNode &) override;

    public:
        bool goDie() override;
        bool goGhost() override;

    public:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void on_AM_BUY(const ActorMsgPack &);
        void on_AM_ACTION(const ActorMsgPack &);
        void on_AM_NPCEVENT(const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO(const ActorMsgPack &);
        void on_AM_BADACTORPOD(const ActorMsgPack &);
        void on_AM_QUERYCORECORD(const ActorMsgPack &);
        void on_AM_QUERYLOCATION(const ActorMsgPack &);
        void on_AM_QUERYSELLITEMLIST(const ActorMsgPack &);

    private:
        void sendQuery(uint64_t, uint64_t, const std::string &);

    private:
        void postSell(uint64_t);
        void postGift(uint64_t, uint32_t, int);
        void postXMLLayout(uint64_t, std::string);
        void postAddMonster(uint32_t);

    public:
        void operateAM(const ActorMsgPack &) override;

    protected:
        virtual std::set<uint32_t> getDefaultSellItemIDList() const;

    private:
        std::vector<SDCostItem> getCostItemList(const SDItem &) const;

    private:
        void fillSellItemList();
        SDItem createSellItem(uint32_t, uint32_t) const;
};
