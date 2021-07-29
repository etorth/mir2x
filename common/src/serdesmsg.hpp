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

struct SDItemExtAttrList
{
    std::unordered_map<int, std::variant<int, uint32_t, std::array<int, 2>>> list;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(list);
    }

    std::string str() const
    {
        return {};
    }

    bool empty() const
    {
        return list.empty();
    }
};

struct SDItem
{
    enum SDItemAttrType: int
    {
        NONE = 0,
        COLOR, // u32
    };

    uint32_t itemID = 0;
    uint32_t  seqID = 0;
    size_t    count = 1;
    size_t duration = 0;
    SDItemExtAttrList extAttrList = {};

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemID, seqID, count, duration, extAttrList);
    }

    std::string str() const
    {
        return str_printf("(itemID, seqID, count, duration, extAttrList) = (%llu, %llu, %zu, %zu, %s)", to_llu(itemID), to_llu(seqID), count, duration, to_cstr(extAttrList.str()));
    }

    bool isGold() const
    {
        return to_u8sv(DBCOM_ITEMRECORD(itemID).type) == u8"金币";
    }

    operator bool () const;
    static std::vector<SDItem> buildGoldItem(size_t);

    std::optional<uint32_t> getColor() const
    {
        if(extAttrList.list.count(SDItem::COLOR)){
            return std::get<uint32_t>(extAttrList.list.at(SDItem::COLOR));
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

struct SDLoginOK
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

struct SDHealth
{
    uint64_t uid = 0;

    int HP = 0;
    int MP = 0;

    int maxHP = 0;
    int maxMP = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(uid, HP, MP, maxHP, maxMP);
    }
};
