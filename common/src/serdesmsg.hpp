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
#include "colorf.hpp"
#include "cerealf.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

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
    std::vector<int> jobList = {};

    int hair = 0;
    int hairColor = 0;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(dbid, channID, name, nameColor, x, y, mapID, exp, gold, jobList, hair, hairColor);
    }
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
};

struct SDItemExtAttrList
{
    std::unordered_map<int, std::variant<int, std::array<int, 2>>> list;
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

    void checkEx() const;
    operator bool () const;
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

struct SDBuyCost
{
    SDItem item;
    std::vector<SDCostItem> costList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(item, costList);
    }
};

struct SDPickUpItemIDList
{
    uint32_t failedItemID = 0;
    std::vector<uint32_t> itemIDList;
    template<typename Archive> void serialize(Archive & ar)
    {
        ar(failedItemID, itemIDList);
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
