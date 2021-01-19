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
#include <string>
#include "cerealf.hpp"

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

struct SDSellItem
{
    uint32_t itemID = 0;
    struct SellItemSingle
    {
        uint32_t price = 0;
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(price);
        }

        void clear()
        {
            price = 0;
        }
    }single;

    struct SellItemList
    {
        struct ListNode
        {
            uint32_t price = 0;
            template<typename Archive> void serialize(Archive & ar)
            {
                ar(price);
            }
        };

        std::vector<ListNode> data;
        template<typename Archive> void serialize(Archive & ar)
        {
            ar(data);
        }

        void clear()
        {
            data.clear();
        }
    }list;

    template<typename Archive> void serialize(Archive & ar)
    {
        ar(itemID, single, list);
    }

    void clear()
    {
        itemID = 0;
        single.clear();
        list  .clear();
    }
};
