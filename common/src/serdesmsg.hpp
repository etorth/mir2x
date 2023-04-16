#pragma once
#include <array>
#include <string>
#include <numeric>
#include <variant>
#include <utility>
#include <optional>
#include <algorithm>
#include <unordered_set>
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "dbcomid.hpp"
#include "luaf.hpp"

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
    std::vector<int> jobList = {};

    int hair = 0;
    int hairColor = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(dbid, channID, name, nameColor, x, y, mapID, hp, mp, exp, gold, gender, jobList, hair, hairColor);
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

template<typename... Ts> struct SDQuestTriggerDispatcher: Ts...
{
    using Ts::operator()...;
};

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

struct SDItem
{
    enum SDItemXMLLayoutParamType: int
    {
        XML_NONE  = 0,
        XML_BEGIN = 1,
        XML_DC    = 1,
        XML_MC,
        XML_SC,
        XML_LEVEL,
        XML_JOB,

        XML_PRICE,
        XML_PRICECOLOR,
        XML_END,
    };

    constexpr static int EA_NONE  = 0;
    constexpr static int EA_BEGIN = 1;

    constexpr static int _ea_add_type_counter_begin = __COUNTER__;
#define _MACRO_ADD_EA_TYPE(eaType, eaValType) \
    constexpr static int eaType = __COUNTER__ - _ea_add_type_counter_begin; \
    struct eaType##_t \
    { \
        constexpr static int value = eaType; \
        using type = eaValType; \
    }; \
    template<typename ... Args> static std::pair<const int, std::string> build_##eaType(Args && ... args) \
    { \
        return {eaType, cerealf::serialize<eaValType>(eaValType(std::forward<Args>(args)...), -1)}; \
    } \

    /**/ // begin of extra-attributes
    /**/ // each line provides three types, take EA_DC as example:
    /**/ //
    /**/ //     SDItem::EA_DC           : an integer used as key
    /**/ //     SDItem::EA_DC_t         : an type contains: EA_DC_t::value and EA_DC_t::type
    /**/ //     SDItem::build_EA_DC()   : an function returns std::pair<int, std::string> where second is the serialized extra-attributes value
    /**/ //
    /**/ // don't put any other code except the macro defines and type aligns
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_SC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_AC , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MAC, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DCHIT, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MCHIT, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DCDODGE, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MCDODGE, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_SPEED, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_COMFORT, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_LUCKCURSE, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_HPADD, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_HPSTEAL, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_HPRECOVER, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_MPADD, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MPSTEAL, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_MPRECOVER, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_DCFIRE   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCICE    , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCLIGHT  , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCWIND   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCHOLY   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCDARK   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_DCPHANTOM, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_ACFIRE   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACICE    , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACLIGHT  , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACWIND   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACHOLY   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACDARK   , int)
    /**/ _MACRO_ADD_EA_TYPE(EA_ACPHANTOM, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_LOADBODY, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_LOADWEAPON, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_LOADINVENTORY, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_EXTEXP, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_EXTGOLD, int)
    /**/ _MACRO_ADD_EA_TYPE(EA_EXTDROP, int)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_BUFFID, int)
    /**/
    /**/ using _helper_type_EA_TELEPORT_t = std::tuple<uint32_t, int, int>;
    /**/ _MACRO_ADD_EA_TYPE(EA_TELEPORT, _helper_type_EA_TELEPORT_t)
    /**/
    /**/ _MACRO_ADD_EA_TYPE(EA_COLOR, uint32_t)
    /**/
    /**/ // end of extra-attributes
    /**/ // any extra-attributes should be put inside above region

#undef _MACRO_ADD_EA_TYPE
    constexpr static int EA_END = __COUNTER__ - _ea_add_type_counter_begin;

    uint32_t itemID = 0;
    uint32_t  seqID = 0;

    size_t count = 1;
    size_t duration[2] = {0, 0};
    std::unordered_map<int, std::string> extAttrList = {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemID, seqID, count, duration[0], duration[1], extAttrList);
    }

    std::string str() const
    {
        return str_printf("(name, itemID, seqID, count, duration) = (%s, %zu, %zu, %zu, (%zu, %zu))", to_cstr(DBCOM_ITEMRECORD(itemID).name), to_uz(itemID), to_uz(seqID), count, duration[0], duration[1]);
    }

    std::u8string getXMLLayout(const std::unordered_map<int, std::string> & = {}) const;

    bool isGold() const
    {
        return to_u8sv(DBCOM_ITEMRECORD(itemID).type) == u8"金币";
    }

    operator bool () const;
    static std::vector<SDItem> buildGoldItem(size_t);

    template<typename T> std::optional<typename T::type> getExtAttr() const
    {
        static_assert(T::value >= EA_BEGIN);
        static_assert(T::value <  EA_END  );

        if(const auto p = extAttrList.find(T::value); p != extAttrList.end()){
            return cerealf::deserialize<typename T::type>(p->second);
        }
        return {};
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

struct SDRuntimeConfig
{
    int mute = 0;
    SDMagicKeyList magicKeyList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(mute, magicKeyList);
    }

    void clear()
    {
        mute = 0;
        magicKeyList.clear();
    }
};

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
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(code);
    }
};

struct SDRemoteCallResult
{
    std::vector<std::string> error {}; // a multiline error
    std::string serVarList {}; // serialization of multiple results, lua supports return-multiple-results syntax

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(error, serVarList);
    }
};

struct SDQuestNotify
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
