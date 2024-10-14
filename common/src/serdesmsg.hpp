#pragma once
#include <array>
#include <string>
#include <numeric>
#include <variant>
#include <utility>
#include <optional>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "dbcomid.hpp"
#include "luaf.hpp"
#include "sditem.hpp"
#include "sdruntimeconfig.hpp"

struct SDInitQuest
{
    uint32_t questID = 0;
    std::string fullScriptName {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(questID, fullScriptName);
    }
};

struct SDInitPlayer
{
    uint32_t dbid = 0;
    uint32_t channID = 0;

    std::string name {};
    uint32_t nameColor = 0;

    int x = 0;
    int y = 0;
    uint32_t mapID = 0;

    int hp = 0;
    int mp = 0;

    int exp = 0;
    int gold = 0;
    bool gender = true;
    int job = 0;

    int hair = 0;
    int hairColor = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(dbid, channID, name, nameColor, x, y, mapID, hp, mp, exp, gold, gender, job, hair, hairColor);
    }
};

struct SDInitNPChar
{
    uint16_t lookID = 0;
    std::string npcName {};
    std::string fullScriptName {};

    uint32_t mapID = 0;
    int x = 0;
    int y = 0;
    int gfxDir = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(lookID, npcName, fullScriptName, mapID, x, y, gfxDir);
    }
};

struct SDQuestTriggerLevelUp
{
    int oldLevel = 0;
    int newLevel = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(oldLevel, newLevel);
    }
};

struct SDQuestTriggerKill
{
    uint32_t monsterID = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(monsterID);
    }
};

struct SDQuestTriggerGainExp
{
    int addedExp = 0;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(addedExp);
    }
};

struct SDQuestTriggerGainGold
{
    int addedGold = 0;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(addedGold);
    }
};

struct SDQuestTriggerGainItem
{
    uint32_t itemID = 0;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemID);
    }
};

using SDQuestTriggerVar = std::variant<
    SDQuestTriggerLevelUp,
    SDQuestTriggerKill,
    SDQuestTriggerGainExp,
    SDQuestTriggerGainGold,
    SDQuestTriggerGainItem
>;

struct SDNPCXMLLayout
{
    uint64_t npcUID = 0;
    std::string eventPath {};
    std::string xmlLayout {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, eventPath, xmlLayout);
    }
};

struct SDChatGroupMember
{
    uint32_t dbid = 0;
    uint16_t priority = 0;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(dbid, priority);
    }
};

struct SDChatPeerPlayerVar
{
    bool gender = false;
    int job = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(gender, job);
    }
};

struct SDChatPeerGroupVar
{
    uint32_t creator = 0;
    uint64_t createtime = 0;
    std::vector<SDChatGroupMember> memberList {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(creator, createtime, memberList);
    }
};

class SDChatPeerID
{
    private:
        uint64_t m_data;

    public:
        SDChatPeerID()
            : m_data(0)
        {}

    public:
        explicit SDChatPeerID(uint64_t);

    public:
        SDChatPeerID(ChatPeerType, uint32_t);

    public:
        SDChatPeerID(const SDChatPeerID &other)
            : m_data(other.m_data)
        {}

        SDChatPeerID & operator = (const SDChatPeerID &other) noexcept
        {
            m_data = other.m_data;
            return *this;
        }

    public:
        auto operator <=> (const SDChatPeerID &) const = default;

    public:
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(m_data);
        }

    public:
        ChatPeerType type() const
        {
            return (ChatPeerType)(m_data >> 32);
        }

        uint32_t id() const
        {
            return to_u32(m_data);
        }

    public:
        bool group  () const { return type() == CP_GROUP  ; }
        bool player () const { return type() == CP_PLAYER ; }
        bool special() const { return type() == CP_SPECIAL; }

    public:
        uint64_t asU64() const
        {
            return m_data;
        }

    public:
        bool empty() const
        {
            return m_data == 0;
        }

    public:
        operator bool () const
        {
            return !empty();
        }
};

struct SDChatPeer
{
    uint32_t id = 0;
    std::string name {};
    std::optional<uint64_t> avatar {};

    std::variant<std::monostate, SDChatPeerPlayerVar, SDChatPeerGroupVar> despvar {};

    bool special() const
    {
        return std::get_if<std::monostate>(&despvar);
    }

    const SDChatPeerPlayerVar *player() const
    {
        return std::get_if<SDChatPeerPlayerVar>(&despvar);
    }

    const SDChatPeerGroupVar *group() const
    {
        return std::get_if<SDChatPeerGroupVar>(&despvar);
    }

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(id, name, avatar, despvar);
    }

    bool empty() const
    {
        return id == 0;
    }

    SDChatPeerID cpid() const
    {
        if(empty()){
            return {};
        }

        /**/ if(player()){ return {CP_PLAYER , id}; }
        else if(group ()){ return {CP_GROUP  , id}; }
        else             { return {CP_SPECIAL, id}; }
    }
};

using SDChatPeerList = std::vector<SDChatPeer>;

struct SDStartInvOp
{
    int invOp = INVOP_NONE;

    uint64_t uid = 0;
    std::string queryTag {};
    std::string commitTag {};
    std::vector<std::u8string> typeList;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(invOp, uid, queryTag, commitTag, typeList);
    }

    bool hasType(const char8_t *type) const
    {
        return std::find(typeList.begin(), typeList.end(), type) != typeList.end();
    }

    void clear()
    {
        invOp = INVOP_NONE;
        uid   = 0;

        queryTag .clear();
        commitTag.clear();
        typeList .clear();
    }
};

struct SDStartInput
{
    uint64_t uid = 0;
    std::string title {};
    std::string commitTag {};
    bool show = false;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, title, commitTag, show);
    }
};

struct SDNPCSell
{
    uint64_t npcUID = 0;
    std::vector<uint32_t> itemList;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, itemList);
    }
};

struct SDCostItem
{
    uint32_t itemID = 0;
    size_t    count = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemID, count);
    }

    bool isGold() const
    {
        return to_u8sv(DBCOM_ITEMRECORD(itemID).type) == u8"金币";
    }
};

struct SDDropItem
{
    int x = 0;
    int y = 0;
    SDItem item;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(x, y, item);
    }
};

struct SDBelt // belt items don't have seqID constraint
{
    std::array<SDItem, 6> list;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(list);
    }

    void clear()
    {
        list.fill({});
    }
};

struct SDWear // wear items don't have seqID constraint
{
    private:
        std::unordered_map<int, SDItem> m_list;

    public:
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(m_list);
        }

        void clear()
        {
            m_list.clear();
        }

    public:
        void setWLItem(int, SDItem);
        const SDItem &getWLItem(int) const;
};

struct SDShowSecuredItemList
{
    std::vector<SDItem> itemList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemList);
    }
};

struct SDSellItem
{
    SDItem item;
    std::vector<SDCostItem> costList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(item, costList);
    }
};

struct SDSellItemList
{
    uint64_t npcUID = 0;
    std::vector<SDSellItem> list;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, list);
    }

    void clear()
    {
        npcUID = 0;
        list.clear();
    }
};

struct SDUpdateItem
{
    SDItem item;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(item);
    }
};

class SDInventory final
{
    // keep the item list encapsulated
    // don't expose it since we have seqID duplication check

    private:
        std::vector<SDItem> m_list;

    public:
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(m_list);
        }

    public:
        const auto &getItemList() const
        {
            return m_list;
        }

    public:
        size_t has(uint32_t, uint32_t) const;
        const SDItem &find(uint32_t, uint32_t) const;

    public:
        const SDItem &add(SDItem, bool);

    public:
        // only do change in one item per call
        // returns:
        //          [0]: how many items get removed, 0 means can't remove any item
        //          [1]: seqID of the SDItem which applies the removal
        //          [2]: const pointer to the SDItem, if removed fully then remove nullptr
        std::tuple<size_t, uint32_t, const SDItem *> remove(uint32_t, uint32_t, size_t);

    public:
        void clear()
        {
            m_list.clear();
        }

    public:
        void merge(uint32_t, uint32_t, uint32_t);

    private:
        static uint64_t buildItemSeqID(uint32_t itemID, uint32_t seqID)
        {
            return (to_u64(itemID) << 32) | seqID;
        }

    private:
        std::unordered_set<uint64_t> getItemSeqIDSet() const;
};

struct SDPlayerName
{
    uint64_t uid = 0;
    std::string name {};
    uint32_t nameColor {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, name, nameColor);
    }
};

struct SDWLDesp
{
    SDWear wear;
    uint32_t hair;
    uint32_t hairColor;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(wear, hair, hairColor);
    }

    void clear()
    {
        hair = 0;
        hairColor = 0;
        wear.clear();
    }
};

using SDUIDList = std::vector<uint64_t>;

struct SDUIDWLDesp
{
    uint64_t uid = 0;
    SDWLDesp desp;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, desp);
    }
};

struct SDStartGameScene
{
    uint64_t uid   = 0;
    uint32_t mapID = 0;

    int x = -1;
    int y = -1;
    int direction = DIR_NONE;

    SDWLDesp desp;
    std::string name {};
    uint32_t nameColor = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, mapID, x, y, direction, desp, name);
    }
};

struct SDItemStorage
{
    size_t gold = 0;

    SDWear wear;
    SDBelt belt;
    SDInventory inventory;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(gold, wear, belt, inventory);
    }

    void clear()
    {
        gold = 0;
        wear.clear();
        belt.clear();
        inventory.clear();
    }
};

struct SDEquipWear
{
    uint64_t uid = 0;
    int wltype = WLG_NONE;
    SDItem item;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, wltype, item);
    }
};

struct SDGrabWear
{
    int wltype = WLG_NONE;
    SDItem item;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(wltype, item);
    }
};

struct SDEquipBelt
{
    int slot = -1;
    SDItem item;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(slot, item);
    }
};

struct SDGrabBelt
{
    int slot = -1;
    SDItem item;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(slot, item);
    }
};

struct SDBuyCost
{
    SDItem item;
    std::vector<SDCostItem> costList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(item, costList);
    }
};

struct SDPickUpItemList
{
    uint32_t failedItemID = 0;
    std::vector<SDItem> itemList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(failedItemID, itemList);
    }
};

struct SDGroundItemIDList
{
    struct GridItemIDList
    {
        int x = -1;
        int y = -1;
        std::vector<uint32_t> itemIDList;
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(x, y, itemIDList);
        }
    };

    uint32_t mapID = 0;
    std::vector<GridItemIDList> gridItemIDList;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(mapID, gridItemIDList);
    }

    void clear()
    {
        mapID = 0;
        gridItemIDList.clear();
    }
};

struct SDGridFireWall
{
    int x = 0;
    int y = 0;
    int count = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(x, y, count);
    }
};

struct SDGroundFireWallList
{
    uint32_t mapID = 0;
    std::vector<SDGridFireWall> fireWallList;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(mapID, fireWallList);
    }
};

struct SDLearnedMagic
{
    uint32_t magicID = 0;
    int exp = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(magicID, exp);
    }

    int level() const
    {
        return 1;
    }
};

struct SDLearnedMagicList
{
    std::vector<SDLearnedMagic> magicList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(magicList);
    }

    bool has(uint32_t magicID) const
    {
        return std::ranges::find_if(magicList, [magicID](const auto &param)
        {
            return param.magicID == magicID;
        }) != magicList.end();
    }

    void clear()
    {
        magicList.clear();
    }
};

struct SDMagicKeyList
{
    std::unordered_map<uint32_t, char> keyList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(keyList);
    }

    void clear()
    {
        keyList.clear();
    }

    bool setMagicKey(uint32_t, char);
};

struct SDPlayerConfig
{
    SDMagicKeyList magicKeyList;
    SDRuntimeConfig runtimeConfig;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(magicKeyList, runtimeConfig);
    }
};

using SDFriendList = std::vector<SDChatPeer>;
using SDXMLMessage = std::string;

struct SDAddFriendNotif
{
    int notif = 0;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(notif);
    }
};

struct SDAddBlockedNotif
{
    int notif = 0;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(notif);
    }
};

struct SDChatMessageDBSeq
{
    uint64_t id = 0;
    uint64_t timestamp = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(id, timestamp);
    }
};

struct SDChatMessage
{
    std::optional<SDChatMessageDBSeq> seq {};
    std::optional<uint64_t> refer {};

    SDChatPeerID from {};
    SDChatPeerID to   {};

    std::string message; // always serialized

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(seq, refer, from, to, message);
    }
};

using SDChatMessageList = std::vector<SDChatMessage>;

struct SDNPCEvent
{
    int x = 0;
    int y = 0;
    uint32_t mapID = 0;

    std::string path {};
    std::string event {};
    std::optional<std::string> value {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(x, y, mapID, path, event, value);
    }
};

struct SDTaggedValMap
{
    std::map<int, int> valMap;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(valMap);
    }

    int add(int val)
    {
        if(valMap.empty()){
            valMap[1] = val;
        }
        else{
            valMap[valMap.rbegin()->first + 1] = val;
        }
        return valMap.rbegin()->first;
    }

    void erase(int tag)
    {
        valMap.erase(tag);
    }

    int sum() const
    {
        return std::accumulate(valMap.begin(), valMap.end(), 0, [](const auto x, const auto y){ return x + y.second; });
    }
};

struct SDBuffedAbility
{
    SDTaggedValMap dc[2];
    SDTaggedValMap mc[2];
    SDTaggedValMap sc[2];

    SDTaggedValMap  ac[2];
    SDTaggedValMap mac[2];

    SDTaggedValMap dcHit;
    SDTaggedValMap mcHit;

    SDTaggedValMap dcDodge;
    SDTaggedValMap mcDodge;

    SDTaggedValMap speed;
    SDTaggedValMap luckCurse;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(dc[0], dc[1], mc[0], mc[1], sc[0], sc[1], ac[0], ac[1], mac[0], mac[1], dcHit, mcHit, dcDodge, mcDodge, speed, luckCurse);
    }
};

struct SDHealth
{
    uint64_t uid = 0;

    int hp = 0;
    int mp = 0;

    int maxHP = 0;
    int maxMP = 0;

    int hpRecover = 0;
    int mpRecover = 0;

    SDTaggedValMap buffedMaxHP;
    SDTaggedValMap buffedMaxMP;

    SDTaggedValMap buffedHPRecover;
    SDTaggedValMap buffedMPRecover;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, hp, mp, maxHP, maxMP, hpRecover, mpRecover, buffedMaxHP, buffedMaxMP, buffedHPRecover, buffedMPRecover);
    }

    bool updateHealth(int addHP = 0, int addMP = 0, int addMaxHP = 0, int addMaxMP = 0)
    {
        const auto oldHP    = hp;
        const auto oldMP    = mp;
        const auto oldMaxHP = maxHP;
        const auto oldMaxMP = maxMP;

        maxHP = std::max<int>(0, maxHP + addMaxHP);
        maxMP = std::max<int>(0, maxMP + addMaxMP);

        hp = std::max<int>(0, std::min<int>(hp + addHP, getMaxHP()));
        mp = std::max<int>(0, std::min<int>(mp + addMP, getMaxMP()));

        return false
            || (oldHP != hp)
            || (oldMP != mp)
            || (oldMaxHP != maxHP)
            || (oldMaxMP != maxMP);
    }

    int getMaxHP() const
    {
        return std::max<int>(0, maxHP + buffedMaxHP.sum());
    }

    int getMaxMP() const
    {
        return std::max<int>(0, maxMP + buffedMaxMP.sum());
    }

    int getHPRecover() const
    {
        return hpRecover + buffedHPRecover.sum();
    }

    int getMPRecover() const
    {
        return mpRecover + buffedMPRecover.sum();
    }
};

struct SDBuffIDList
{
    uint64_t uid = 0;
    std::vector<uint32_t> idList;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, idList);
    }
};

struct SDRemoteCall
{
    std::string code {};
    luaf::luaVar args {}; // use table.unpack() to restore all args

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(code, args);
    }
};

struct SDRemoteCallResult
{
    std::vector<std::string> error {}; // a multiline error
    std::vector<luaf::luaVar> varList {}; // lua supports return-multiple-results syntax

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(error, varList);
    }
};

struct SDSendNotify
{
    uint64_t key = 0;
    uint64_t seqID = 0;
    std::vector<luaf::luaVar> varList {};

    bool waitConsume = false; // if send response after notification has been consumed

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(key, seqID, varList, waitConsume);
    }
};

struct SDRegisterQuest
{
    std::string name {};
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(name);
    }
};

struct SDQueryQuestUID
{
    std::string name {};
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(name);
    }
};

struct SDTeamPlayer
{
    uint64_t uid = 0;
    uint32_t level = 0;
    std::string name {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, level, name);
    }
};

struct SDTeamCandidate
{
    SDTeamPlayer player {};
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(player);
    }
};

struct SDRequestJoinTeam
{
    SDTeamPlayer player {};
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(player);
    }
};

struct SDTeamMemberList
{
    uint64_t teamLeader = 0;
    std::vector<SDTeamPlayer> memberList {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(teamLeader, memberList);
    }

    void clear()
    {
        teamLeader = 0;
        memberList.clear();
    }

    bool hasMember(uint64_t uid) const
    {
        if(teamLeader){
            return std::find_if(memberList.begin(), memberList.end(), [uid](const auto &member) -> bool
            {
                return member.uid == uid;
            }) != memberList.end();
        }
        return false;
    }

    std::vector<uint64_t> getUIDList() const
    {
        if(teamLeader){
            std::vector<uint64_t> uidList;
            uidList.reserve(memberList.size());

            for(const auto &member: memberList){
                uidList.push_back(member.uid);
            }

            return uidList;
        }
        return {};
    }
};

struct SDQuestDespUpdate
{
    std::string name {};
    std::string  fsm {};

    std::optional<std::string> desp {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(name, fsm, desp);
    }
};

using SDQuestDespList = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>; // sdQDL[quest][fsm] = desp
