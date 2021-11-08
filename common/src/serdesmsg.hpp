/*
 * =====================================================================================
 *
 *       Filename: serdesmsg.hpp
 *        Created: 01/24/2016 19:30:45
 *    Description: net message used by client and mono-server
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
#include <array>
#include <string>
#include <numeric>
#include <variant>
#include <optional>
#include <algorithm>
#include <unordered_set>
#include "totype.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"
#include "dbcomid.hpp"
#include "dbcomrecord.hpp"

struct SDInitPlayer
{
    uint32_t dbid = 0;
    uint32_t channID = 0;

    std::string name;
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
        ar(dbid, channID, name, nameColor, x, y, mapID, exp, gold, gender, jobList, hair, hairColor);
    }
};

struct SDInitNPChar
{
    std::string filePath;
    uint32_t mapID = 0;
    std::string npcName;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(filePath, mapID, npcName);
    }

    std::string getFileName() const;
};

struct SDNPCXMLLayout
{
    uint64_t npcUID = 0;
    std::string xmlLayout;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(npcUID, xmlLayout);
    }
};

struct SDStartInvOp
{
    int invOp = INVOP_NONE;

    uint64_t uid = 0;
    std::string queryTag;
    std::string commitTag;
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
    std::string title;
    std::string commitTag;
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

using SDItemExtAttr      = std::variant<int, double, uint32_t, std::array<int, 2>, std::string>;
using SDItemExtAttrList  = std::unordered_map<int, SDItemExtAttr>;

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

    enum SDItemExtAttrType: int
    {
        EA_NONE  = 0,
        EA_BEGIN = 1,
        EA_DC    = 1,
        EA_MC,
        EA_SC,
        EA_AC,
        EA_MAC,

        EA_DCHIT,
        EA_MCHIT,

        EA_DCDODGE,
        EA_MCDODGE,

        EA_SPEED,
        EA_COMFORT,
        EA_LUCKCURSE,

        EA_HPADD,
        EA_HPSTEAL,
        EA_HPRECOVER,

        EA_MPADD,
        EA_MPSTEAL,
        EA_MPRECOVER,

        EA_DCFIRE,
        EA_DCICE,
        EA_DCLIGHT,
        EA_DCWIND,
        EA_DCHOLY,
        EA_DCDARK,
        EA_DCPHANTOM,

        EA_ACFIRE,
        EA_ACICE,
        EA_ACLIGHT,
        EA_ACWIND,
        EA_ACHOLY,
        EA_ACDARK,
        EA_ACPHANTOM,

        EA_LOADBODY,
        EA_LOADWEAPON,
        EA_LOADINVENTORY,

        EA_EXTEXP,
        EA_EXTGOLD,
        EA_EXTDROP,

        EA_BUFFID,

        EA_COLOR, // u32
        EA_END,
    };

    uint32_t itemID = 0;
    uint32_t  seqID = 0;

    size_t count = 1;
    size_t duration[2] = {0, 0};
    SDItemExtAttrList extAttrList = {};

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

    template<typename T> std::optional<T> getExtAttr(int attrType) const
    {
        fflassert(attrType >= EA_BEGIN);
        fflassert(attrType <  EA_END);

        if(const auto p = extAttrList.find(attrType); p != extAttrList.end()){
            return std::get<T>(p->second);
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
    std::string name;
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

    std::string event;
    std::optional<std::string> value;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(x, y, mapID, event, value);
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

struct SDHealth
{
    uint64_t uid = 0;

    int hp = 0;
    int mp = 0;

    int maxHP = 0;
    int maxMP = 0;

    SDTaggedValMap buffMaxHP;
    SDTaggedValMap buffMaxMP;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, hp, mp, maxHP, maxMP, buffMaxHP, buffMaxMP);
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
            || (oldHP    != hp)
            || (oldMP    != mp)
            || (oldMaxHP != maxHP)
            || (oldMaxMP != maxMP);
    }

    int getMaxHP() const
    {
        return std::max<int>(0, maxHP + buffMaxHP.sum());
    }

    int getMaxMP() const
    {
        return std::max<int>(0, maxMP + buffMaxMP.sum());
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
