#pragma once
#include <memory>
#include <cstdint>
#include <optional>
#include "servicecore.hpp"
#include "servermap.hpp"
#include "charobject.hpp"

class NPChar final: public CharObject
{
    private:
        struct SellItem
        {
            SDItem item;
            bool locked = false;
            std::vector<SDCostItem> costList;
        };

    protected:
        class LuaThreadRunner: public CharObject::LuaThreadRunner
        {
            public:
                LuaThreadRunner(NPChar *npc);

            public:
                NPChar *getNPChar() const
                {
                    return static_cast<NPChar *>(getCO());
                }
        };

    private:
        const std::string m_npcName;
        const std::string m_initScriptName;

    private:
        std::set<uint32_t> m_npcSell;

    private:
        std::unique_ptr<NPChar::LuaThreadRunner> m_luaRunner;
        std::unordered_map<uint32_t, std::map<uint32_t, SellItem>> m_sellItemList;

    public:
        NPChar(const ServerMap *, const SDInitNPChar &initNPChar);

    public:
        bool update() override;

    public:
        void reportCO(uint64_t) override;

    protected:
        const std::set<uint32_t> &getSellList() const
        {
            return m_npcSell;
        }

    protected:
        void onActivate() override;

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
        void postXMLLayout(uint64_t, std::string, std::string);
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
