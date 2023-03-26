#pragma once
#include <set>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "totype.hpp"
#include "monoserver.hpp"
#include "battleobject.hpp"
#include "combatnode.hpp"
#include "serverluamodule.hpp"

class Player final: public BattleObject
{
    private:
        enum ScriptEvent: int
        {
            ON_NONE  = 0,
            ON_BEGIN = 1,

            ON_KILL = ON_BEGIN,
            ON_LEVELUP,

            ON_END,
        };

    public:
        friend class ServerObject;
        friend class   CharObject;
        friend class BattleObject;

    protected:
        // empty: hasn't bind to a channel yet
        //  zero: has received AM_BADCHANNEL and need to go offine
        //  xxxx: channel id of associated working channel
        std::optional<uint32_t> m_channID;

    protected:
        uint32_t m_exp;

    protected:
        std::string m_name;
        uint32_t m_nameColor;

        uint32_t m_hair;
        uint32_t m_hairColor;

    protected:
        std::set<uint64_t> m_slaveList;

    private:
        bool m_pickUpLock = false;

    private:
        // flag to show next hit is 攻杀剑术
        // by implementation client reports action to server, at the same time client present action gfx without wait server's response
        // so for some random action like 攻杀技术, server has to send enable flag to client in advance to show that ``next hit you can use 攻杀技术"
        bool m_nextStrike = false;

    private:
        SDItemStorage      m_sdItemStorage;
        SDLearnedMagicList m_sdLearnedMagicList;
        SDRuntimeConfig    m_sdRuntimeConfig;

    private:
        std::unordered_map<int, std::function<void()>> m_onBeltOff;
        std::unordered_map<int, std::function<void()>> m_onWLOff;

    private:
        uint64_t m_runnerSeqID = 1;
        std::unique_ptr<ServerLuaCoroutineRunner> m_luaRunner;

    private:
        std::unordered_map<int, std::vector<sol::function>> m_scriptEventTriggerList;

    public:
        Player(const SDInitPlayer &, const ServerMap *);

    public:
        ~Player() = default;

    public:
        void onActivate() override;

    protected:
        uint32_t exp() const
        {
            return m_exp;
        }

        uint32_t gold() const
        {
            return m_sdItemStorage.gold;
        }

        uint32_t level() const
        {
            return to_u32(SYS_LEVEL(exp()));
        }

        std::string name() const
        {
            return m_name;
        }

    public:
        int Speed(int) const override
        {
            return 5;
        }

        bool update() override;

    protected:
        void operateNet(uint8_t, const uint8_t *, size_t);

    protected:
        void operateAM(const ActorMsgPack &);

    private:
        void on_AM_EXP              (const ActorMsgPack &);
        void on_AM_ADDBUFF          (const ActorMsgPack &);
        void on_AM_REMOVEBUFF       (const ActorMsgPack &);
        void on_AM_MISS             (const ActorMsgPack &);
        void on_AM_HEAL             (const ActorMsgPack &);
        void on_AM_GIFT             (const ActorMsgPack &);
        void on_AM_ACTION           (const ActorMsgPack &);
        void on_AM_ATTACK           (const ActorMsgPack &);
        void on_AM_OFFLINE          (const ActorMsgPack &);
        void on_AM_CORECORD         (const ActorMsgPack &);
        void on_AM_METRONOME        (const ActorMsgPack &);
        void on_AM_MAPSWITCHTRIGGER (const ActorMsgPack &);
        void on_AM_SENDPACKAGE      (const ActorMsgPack &);
        void on_AM_RECVPACKAGE      (const ActorMsgPack &);
        void on_AM_BADCHANNEL       (const ActorMsgPack &);
        void on_AM_NOTIFYDEAD       (const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO      (const ActorMsgPack &);
        void on_AM_QUERYHEALTH      (const ActorMsgPack &);
        void on_AM_DEADFADEOUT      (const ActorMsgPack &);
        void on_AM_BADACTORPOD      (const ActorMsgPack &);
        void on_AM_BINDCHANNEL      (const ActorMsgPack &);
        void on_AM_CHECKMASTER      (const ActorMsgPack &);
        void on_AM_QUERYCORECORD    (const ActorMsgPack &);
        void on_AM_QUERYLOCATION    (const ActorMsgPack &);
        void on_AM_QUERYFRIENDTYPE  (const ActorMsgPack &);
        void on_AM_REMOVEGROUNDITEM (const ActorMsgPack &);
        void on_AM_QUERYUIDBUFF     (const ActorMsgPack &);
        void on_AM_QUERYPLAYERWLDESP(const ActorMsgPack &);
        void on_AM_REMOTECALL          (const ActorMsgPack &);

    private:
        void net_CM_REQUESTADDEXP             (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTKILLPETS           (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTSPACEMOVE          (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTMAGICDAMAGE        (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTRETRIEVESECUREDITEM(uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYCORECORD             (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYSELLITEMLIST         (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYUIDBUFF              (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYPLAYERWLDESP         (uint8_t, const uint8_t *, size_t);
        void net_CM_ACTION                    (uint8_t, const uint8_t *, size_t);
        void net_CM_PICKUP                    (uint8_t, const uint8_t *, size_t);
        void net_CM_PING                      (uint8_t, const uint8_t *, size_t);
        void net_CM_CONSUMEITEM               (uint8_t, const uint8_t *, size_t);
        void net_CM_MAKEITEM                  (uint8_t, const uint8_t *, size_t);
        void net_CM_BUY                       (uint8_t, const uint8_t *, size_t);
        void net_CM_QUERYGOLD                 (uint8_t, const uint8_t *, size_t);
        void net_CM_NPCEVENT                  (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTEQUIPWEAR          (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTGRABWEAR           (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTEQUIPBELT          (uint8_t, const uint8_t *, size_t);
        void net_CM_REQUESTGRABBELT           (uint8_t, const uint8_t *, size_t);
        void net_CM_DROPITEM                  (uint8_t, const uint8_t *, size_t);
        void net_CM_SETMAGICKEY               (uint8_t, const uint8_t *, size_t);

    protected:
        void reportGold();
        void reportStand();
        void reportHealth();
        void reportNextStrike();
        void reportDeadUID(uint64_t);
        void reportCO(uint64_t) override;
        void reportOffline(uint64_t, uint32_t);
        void reportRemoveItem(uint32_t, uint32_t, size_t);
        void reportSecuredItemList();

    protected:
        virtual void reportAction(uint64_t, uint32_t, const ActionNode &);

    protected:
        void dispatchOffline();

    protected:
        bool struckDamage(uint64_t, const DamageNode &) override;

    protected:
        void RequestKillPets();

    protected:
        DamageNode getAttackDamage(int, int) const override;

    protected:
        bool dcValid(int, bool);

    protected:
        bool ActionValid(const ActionNode &);
        bool MotionValid(const ActionNode &);

    protected:
        void onCMActionMove    (CMAction);
        void onCMActionStand   (CMAction);
        void onCMActionSpell   (CMAction);
        void onCMActionSpinKick(CMAction);
        void onCMActionAttack  (CMAction);

    private:
        void postNetMessage(uint8_t, const void *, size_t);

    private:
        void postNetMessage(uint8_t headCode)
        {
            postNetMessage(headCode, nullptr, 0);
        }

        void postNetMessage(uint8_t headCode, const std::string &buf)
        {
            postNetMessage(headCode, buf.data(), buf.length());
        }

        void postNetMessage(uint8_t headCode, const std::u8string &buf)
        {
            postNetMessage(headCode, buf.data(), buf.length());
        }

        template<typename T> void postNetMessage(uint8_t headCode, const T &t)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            postNetMessage(headCode, &t, sizeof(t));
        }

    protected:
        virtual bool goDie();
        virtual bool goGhost();

    protected:
        bool goOffline();

        bool onHorse() const
        {
            return false;
        }

    protected:
        int maxStep() const override
        {
            if(onHorse()){
                return 3;
            }
            else{
                return 2;
            }
        }

    protected:
        void gainExp(int);

    protected:
        bool CanPickUp(uint32_t, uint32_t);

    private:
        void dbUpdateExp();
        void dbUpdateHealth();
        void dbUpdateMapGLoc();

    private:
        void dbLoadInventory();
        void dbUpdateInventoryItem(const SDItem &);
        void dbRemoveInventoryItem(const SDItem &);
        void dbRemoveInventoryItem(uint32_t, uint32_t);

    private:
        void dbSecureItem(uint32_t, uint32_t);
        SDItem dbRetrieveSecuredItem(uint32_t, uint32_t);
        std::vector<SDItem> dbLoadSecuredItemList() const;

    private:
        void dbLoadBelt();
        void dbUpdateBeltItem(size_t, const SDItem &);
        void dbRemoveBeltItem(size_t);

    private:
        void dbLoadWear();
        void dbUpdateWearItem(int, const SDItem &);
        void dbRemoveWearItem(int);

    private:
        void dbLoadRuntimeConfig();

    private:
        void dbLoadLearnedMagic();
        void dbUpdateMagicKey(uint32_t, char);

    private:
        void dbLearnMagic(uint32_t);
        void dbAddMagicExp(uint32_t, size_t);

    protected:
        void checkFriend(uint64_t, std::function<void(int)>) override;

    private:
        void postExp();

    private:
        void postOnlineOK();

    private:
        bool hasInventoryItem(uint32_t, uint32_t, size_t) const;
        const SDItem &addInventoryItem(SDItem, bool);

    private:
        size_t removeInventoryItem(const SDItem &);
        size_t removeInventoryItem(uint32_t, uint32_t);
        size_t removeInventoryItem(uint32_t, uint32_t, size_t);

    private:
        const SDItem &findInventoryItem(uint32_t, uint32_t) const;

    private:
        void secureItem(uint32_t, uint32_t);
        void removeSecuredItem(uint32_t, uint32_t);

    private:
        size_t getGold() const
        {
            return m_sdItemStorage.gold;
        }

        void setGold(size_t);

    protected:
        // TODO bad code need change
        // virtual function with default parameters
        bool updateHealth(
                int = 0,            // hp
                int = 0,            // mp
                int = 0,            // maxHP
                int = 0) override;  // maxMP

    public:
        uint32_t dbid() const
        {
            return uidf::getPlayerDBID(UID());
        }

        bool gender() const
        {
            return uidf::getPlayerGender(UID());
        }

    private:
        void setWLItem(int, SDItem);

    private:
        SDItem createItem(uint32_t);

    private:
        bool canWear(uint32_t, int) const;

    private:
        static std::vector<std::string> parseRemoteCall(const char *);

    private:
        template<typename... Args> void dispatchNetPackage(bool sendSelf, uint8_t type, Args && ... args)
        {
            if(sendSelf){
                postNetMessage(type, std::forward<Args>(args)...);
            }
            dispatchInViewCONetPackage(type, std::forward<Args>(args)...);
        }

    public:
        void notifySlaveGLoc();

    public:
        static int maxHP(uint64_t, uint32_t);
        static int maxMP(uint64_t, uint32_t);

    protected:
        void addWLOffTrigger(int wltype, std::function<void()> trigger)
        {
            fflassert(wltype >= WLG_BEGIN, wltype);
            fflassert(wltype <  WLG_END  , wltype);

            fflassert(trigger);
            auto lastTrigger = std::move(m_onWLOff[wltype]);

            m_onWLOff[wltype] = [lastTrigger = std::move(lastTrigger), currTrigger = std::move(trigger)]()
            {
                if(lastTrigger){
                    lastTrigger();
                }

                if(currTrigger){
                    currTrigger();
                }
            };
        }

    protected:
        template<typename... Args> void runScriptEventTrigger(int eventType, Args && ... args)
        {
            fflassert(eventType >= ON_BEGIN, eventType);
            fflassert(eventType <  ON_END  , eventType);

            if(const auto p = m_scriptEventTriggerList.find(eventType); p != m_scriptEventTriggerList.end()){
                for(auto q = p->second.begin(); q != p->second.end();){
                    if(const auto pfr = (*q)(std::forward<Args>(args)...); m_luaRunner->pfrCheck(pfr)){
                        const auto done = [&pfr]() -> bool
                        {
                            if(pfr.return_count() == 0){
                                return false;
                            }

                            // if multiple results returned
                            // we only check the first one, ignore all rest as lua does

                            if(const auto obj = (sol::object)(*(pfr.cbegin())); obj.is<bool>()){
                                return obj.as<bool>();
                            }
                            else if(obj == sol::nil){
                                return false;
                            }
                            else{
                                throw fflerror("trigger returns invalid type(s): count = %d, [0]: %s", to_d(pfr.return_count()), to_cstr(sol::type_name(obj.lua_state(), obj.get_type())));
                            }
                        }();

                        if(done){
                            std::swap(*q, *(p->second.rbegin()));
                            p->second.pop_back();
                        }
                        else{
                            ++q;
                        }
                    }
                    else{
                        throw fflerror("event trigger error: event = %d", to_d(eventType));
                    }
                }
            }
        }

    protected:
        void resumeCORunner(uint64_t);

    protected:
        bool consumeBook(uint32_t);
        bool consumePotion(uint32_t);
};
