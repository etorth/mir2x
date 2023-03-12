#pragma once
#include <memory>
#include <cstdint>
#include <optional>
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
                    std::optional<std::string> value;

                    // scenario why adding seqID:
                    // 1. received an event which triggers processNPCEvent(event)
                    // 2. inside processNPCEvent(event) the script emits query to other actor
                    // 3. when waiting for the response of the query, user clicked the close button or click init button to end up the current call stack
                    // 4. receives the query response, we should ignore it
                    //
                    // to fix this we have to give every call stack an uniq seqID
                    // and the query response needs to match the seqID

                    const uint64_t seqID;
                    LuaCORunner runner;

                    LuaCallStack(LuaNPCModule *luaModulePtr)
                        : seqID(luaModulePtr->peekSeqID())
                        , runner(luaModulePtr->getLuaState(), "_RSVD_NAME_coth_main")
                    {}

                    void clearEvent()
                    {
                        from = 0;
                        event.clear();
                        value.reset();
                    }
                };

            private:
                uint64_t m_seqID = 0;
                std::unordered_map<uint64_t, LuaCallStack> m_callStackList;

            private:
                std::set<uint32_t> m_npcSell;

            private:
                NPChar * const m_npc;

            public:
                LuaNPCModule(NPChar *, const std::string &);

            public:
                void setEvent(uint64_t callStackUID, uint64_t from, std::string event, std::optional<std::string> value);

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
                    return m_npc->getNPCName();
                }

                const auto &getNPCSell() const
                {
                    return m_npcSell;
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
        std::string m_npcName;
        std::unique_ptr<LuaNPCModule> m_luaModulePtr;
        std::unordered_map<uint32_t, std::map<uint32_t, SellItem>> m_sellItemList;

    public:
        NPChar(const ServerMap *, const SDInitNPChar &initNPChar);

    public:
        bool update() override;

    public:
        void reportCO(uint64_t) override;

    public:
        bool goDie() override;
        bool goGhost() override;

    private:
        void on_AM_BUY(const ActorMsgPack &);
        void on_AM_ATTACK(const ActorMsgPack &);
        void on_AM_ACTION(const ActorMsgPack &);
        void on_AM_NPCEVENT(const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO(const ActorMsgPack &);
        void on_AM_BADACTORPOD(const ActorMsgPack &);
        void on_AM_QUERYCORECORD(const ActorMsgPack &);
        void on_AM_QUERYLOCATION(const ActorMsgPack &);
        void on_AM_REMOTECALL(const ActorMsgPack &);
        void on_AM_QUERYSELLITEMLIST(const ActorMsgPack &);

    private:
        void sendRemoteCall(uint64_t, uint64_t, const std::string &);

    private:
        // NPChar::postXXX functions are for NPC -> client directly
        // for messages NPChar -> Player (then Player may react) we use uidExecute()
        void postSell(uint64_t);
        void postXMLLayout(uint64_t, std::string);
        void postAddMonster(uint32_t);
        void postInvOpCost(uint64_t, int, uint32_t, uint32_t, size_t);
        void postStartInput(uint64_t, std::string, std::string, bool);
        void postStartInvOp(uint64_t, int, std::string, std::string, std::vector<std::u8string>);

    public:
        void operateAM(const ActorMsgPack &) override;

    protected:
        virtual std::set<uint32_t> getDefaultSellItemIDList() const;

    private:
        std::vector<SDCostItem> getCostItemList(const SDItem &) const;

    public:
        const std::string &getNPCName() const
        {
            return m_npcName;
        }

    private:
        void fillSellItemList();
        SDItem createSellItem(uint32_t, uint32_t) const;
};
