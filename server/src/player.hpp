#pragma once
#include <set>
#include <deque>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "totype.hpp"
#include "monoserver.hpp"
#include "battleobject.hpp"
#include "combatnode.hpp"

class Player final: public BattleObject
{
    public:
        friend class ServerObject;
        friend class   CharObject;
        friend class BattleObject;

    protected:
        class LuaThreadRunner: public BattleObject::LuaThreadRunner
        {
            public:
                LuaThreadRunner(Player *);

            public:
                Player *getPlayer() const
                {
                    return static_cast<Player *>(getBO());
                }
        };

        struct DBRetrieveChatMessageListParams final
        {
            const uint64_t id = 0;

            const uint32_t from = 0;
            const uint32_t to   = 0;

            const uint64_t dateFrom = 0;
            const uint64_t dataTo   = 0;

            const size_t limitCount = 0;
        };

    protected:
        // empty: hasn't bind to a channel yet
        //  zero: has received AM_BADCHANNEL and need to go offine
        //  xxxx: channel id of associated working channel
        std::optional<uint32_t> m_channID;

    protected:
        uint32_t m_exp;

    protected:
        bool m_gender;
        int  m_job;

    protected:
        std::string m_name;
        uint32_t m_nameColor;

        uint32_t m_hair;
        uint32_t m_hairColor;

    protected:
        std::set<uint64_t> m_slaveList;

    protected:
        uint64_t m_teamLeader = 0;
        std::vector<uint64_t> m_teamMemberList; // ignored if not team leader

    private:
        bool m_pickUpLock = false;

    private:
        // flag to show next hit is 攻杀剑术
        // by implementation client reports action to server, at the same time client present action gfx without wait server's response
        // so for some random action like 攻杀技术, server has to send enable flag to client in advance to show that ``next hit you can use 攻杀技术"
        bool m_nextStrike = false;

    private:
        SDFriendList       m_sdFriendList;
        SDItemStorage      m_sdItemStorage;
        SDPlayerConfig     m_sdPlayerConfig;
        SDLearnedMagicList m_sdLearnedMagicList;

    private:
        std::unordered_map<int, std::function<void()>> m_onBeltOff;
        std::unordered_map<int, std::function<void()>> m_onWLOff;

    private:
        uint64_t m_threadKey = 1;
        std::unique_ptr<Player::LuaThreadRunner> m_luaRunner;

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

        uint32_t nameColor() const
        {
            return m_nameColor;
        }

    public:
        int Speed(int) const override
        {
            return 5;
        }

        bool update() override;

    protected:
        void operateNet(uint8_t, const uint8_t *, size_t, uint64_t);

    protected:
        void operateAM(const ActorMsgPack &);

    private:
        void on_AM_EXP                (const ActorMsgPack &);
        void on_AM_ADDBUFF            (const ActorMsgPack &);
        void on_AM_REMOVEBUFF         (const ActorMsgPack &);
        void on_AM_MISS               (const ActorMsgPack &);
        void on_AM_HEAL               (const ActorMsgPack &);
        void on_AM_GIFT               (const ActorMsgPack &);
        void on_AM_ACTION             (const ActorMsgPack &);
        void on_AM_ATTACK             (const ActorMsgPack &);
        void on_AM_OFFLINE            (const ActorMsgPack &);
        void on_AM_CORECORD           (const ActorMsgPack &);
        void on_AM_METRONOME          (const ActorMsgPack &);
        void on_AM_MAPSWITCHTRIGGER   (const ActorMsgPack &);
        void on_AM_SENDPACKAGE        (const ActorMsgPack &);
        void on_AM_RECVPACKAGE        (const ActorMsgPack &);
        void on_AM_BADCHANNEL         (const ActorMsgPack &);
        void on_AM_NOTIFYDEAD         (const ActorMsgPack &);
        void on_AM_NOTIFYNEWCO        (const ActorMsgPack &);
        void on_AM_QUERYHEALTH        (const ActorMsgPack &);
        void on_AM_DEADFADEOUT        (const ActorMsgPack &);
        void on_AM_BADACTORPOD        (const ActorMsgPack &);
        void on_AM_BINDCHANNEL        (const ActorMsgPack &);
        void on_AM_CHECKMASTER        (const ActorMsgPack &);
        void on_AM_QUERYCORECORD      (const ActorMsgPack &);
        void on_AM_QUERYLOCATION      (const ActorMsgPack &);
        void on_AM_QUERYFRIENDTYPE    (const ActorMsgPack &);
        void on_AM_REMOVEGROUNDITEM   (const ActorMsgPack &);
        void on_AM_QUERYUIDBUFF       (const ActorMsgPack &);
        void on_AM_QUERYPLAYERNAME    (const ActorMsgPack &);
        void on_AM_QUERYPLAYERWLDESP  (const ActorMsgPack &);
        void on_AM_REMOTECALL         (const ActorMsgPack &);
        void on_AM_REQUESTJOINTEAM    (const ActorMsgPack &);
        void on_AM_REQUESTLEAVETEAM   (const ActorMsgPack &);
        void on_AM_QUERYTEAMPLAYER    (const ActorMsgPack &);
        void on_AM_QUERYTEAMMEMBERLIST(const ActorMsgPack &);
        void on_AM_TEAMUPDATE         (const ActorMsgPack &);

    private:
        void net_CM_ACTION                    (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_BUY                       (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_ADDFRIEND                 (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_ACCEPTADDFRIEND           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REJECTADDFRIEND           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_BLOCKPLAYER               (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_CHATMESSAGE               (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_CONSUMEITEM               (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_DROPITEM                  (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_MAKEITEM                  (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_NPCEVENT                  (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_PICKUP                    (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_PING                      (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYCORECORD             (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYGOLD                 (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYPLAYERNAME           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYPLAYERWLDESP         (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYCHATPEERLIST         (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYSELLITEMLIST         (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_QUERYUIDBUFF              (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTADDEXP             (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTEQUIPBELT          (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTEQUIPWEAR          (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTGRABBELT           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTGRABWEAR           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTJOINTEAM           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTKILLPETS           (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTLATESTCHATMESSAGE  (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTLEAVETEAM          (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTMAGICDAMAGE        (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTRETRIEVESECUREDITEM(uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_REQUESTSPACEMOVE          (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_SETMAGICKEY               (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_SETRUNTIMECONFIG          (uint8_t, const uint8_t *, size_t, uint64_t);
        void net_CM_CREATECHATGROUP           (uint8_t, const uint8_t *, size_t, uint64_t);

    protected:
        void reportGold();
        void reportStand();
        void reportHealth();
        void reportNextStrike();
        void reportTeamMemberList();
        void reportDeadUID(uint64_t);
        void reportCO(uint64_t) override;
        void reportOffline(uint64_t, uint32_t);
        void reportRemoveItem(uint32_t, uint32_t, size_t);
        void reportSecuredItemList();

    protected:
        virtual void reportAction(uint64_t, uint32_t, const ActionNode &);

    protected:
        void pullTeamMemberList(std::function<void(std::optional<SDTeamMemberList>)>);

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
        void onCMActionMine    (CMAction);
        void onCMActionStand   (CMAction);
        void onCMActionSpell   (CMAction);
        void onCMActionSpinKick(CMAction);
        void onCMActionAttack  (CMAction);
        void onCMActionPickUp  (CMAction);

    private:
        void postNetMessage(uint8_t, const void *, size_t, uint64_t = 0);

    private:
        void postNetMessage(uint8_t headCode , uint64_t respID = 0)
        {
            postNetMessage(headCode, nullptr, 0, respID);
        }

        void postNetMessage(uint8_t headCode, const std::string &buf, uint64_t respID = 0)
        {
            postNetMessage(headCode, buf.data(), buf.length(), respID);
        }

        void postNetMessage(uint8_t headCode, const std::u8string &buf, uint64_t respID = 0)
        {
            postNetMessage(headCode, buf.data(), buf.length(), respID);
        }

        template<typename T> void postNetMessage(uint8_t headCode, const T &t, uint64_t respID = 0)
        {
            static_assert(std::is_trivially_copyable_v<T>);
            postNetMessage(headCode, &t, sizeof(t), respID);
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
        luaf::luaVar                  dbGetVar   (const std::string &);
        void                          dbSetVar   (const std::string &, luaf::luaVar);
        std::pair<bool, luaf::luaVar> dbHasVar   (const std::string &);
        void                          dbRemoveVar(const std::string &);

    private:
        void dbUpdateExp();
        void dbUpdateHealth();
        void dbUpdateMapGLoc();

    private:
        void dbLoadInventory();
        void dbLoadFriendList();
        void dbUpdateInventoryItem(const SDItem &);
        void dbRemoveInventoryItem(const SDItem &);
        void dbRemoveInventoryItem(uint32_t, uint32_t);

    private:
        static std::tuple<uint64_t, uint64_t> dbSaveChatMessage(const SDChatPeerID &, const SDChatPeerID &, const std::string_view &);
        SDChatMessageList dbRetrieveLatestChatMessage(const std::span<const uint64_t> &, size_t, bool, bool);

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
        void dbLoadPlayerConfig();
        std::optional<SDChatPeer> dbLoadChatPeer(uint64_t);
        std::vector<uint32_t> dbLoadChatGroupMemberList(uint32_t);
        SDChatPeerList dbQueryChatPeerList(const std::string &, bool, bool);

    private:
        void dbLoadLearnedMagic();
        void dbUpdateMagicKey(uint32_t, char);

    private:
        void dbUpdateRuntimeConfig();

    private:
        void dbLearnMagic(uint32_t);
        void dbAddMagicExp(uint32_t, size_t);

    private:
        static bool dbHasPlayer(uint32_t);
        static bool dbBlocked(uint32_t, uint32_t);
        static SDRuntimeConfig dbGetRuntimeConfig(uint32_t);

    private:
        static std::string dbGetPlayerName(uint32_t);

    private:
        int dbAddFriend(uint32_t);
        int dbBlockPlayer(uint32_t);

    private:
        SDChatPeer dbCreateChatGroup(const char *, const std::span<const uint32_t> &);

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

        SDChatPeerID cpid() const
        {
            return SDChatPeerID(CP_PLAYER, dbid());
        }

        bool gender() const
        {
            return m_gender;
        }

        int job() const
        {
            return m_job;
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
        int maxHP() const;
        int maxMP() const;

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
        const SDChatPeer *findFriendChatPeer(const SDChatPeerID &sdCPID) const
        {
            const auto fnOp = [&sdCPID](const SDChatPeer &peer)
            {
                return peer.cpid() == sdCPID;
            };

            if(auto p = std::find_if(m_sdFriendList.begin(), m_sdFriendList.end(), fnOp); p != m_sdFriendList.end()){
                return std::addressof(*p);
            }

            return nullptr;
        }

    protected:
        void resumeCORunner(uint64_t);

    protected:
        bool consumeBook(uint32_t);
        bool consumePotion(uint32_t);
};
