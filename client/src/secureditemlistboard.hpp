/*
 * =====================================================================================
 *
 *       Filename: secureditemlistboard.hpp
 *        Created: 07/09/2021 23:31:52
 *    Description:
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
#include <cstdint>
#include <optional>
#include <algorithm>
#include "widget.hpp"
#include "itemlistboard.hpp"

class SecuredItemListBoard: public ItemListBoard
{
    private:
        std::vector<SDItem> m_itemList;

    private:
        ProcessRun *m_processRun;

    public:
        SecuredItemListBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void setItemList(std::vector<SDItem> itemList)
        {
            m_itemList = std::move(itemList);
        }

        void removeItem(uint32_t itemID, uint32_t seqID)
        {
            m_itemList.erase(std::find_if(m_itemList.begin(), m_itemList.end(), [itemID, seqID](const auto &param)
            {
                return itemID == param.itemID && seqID == param.seqID;
            }));
        }

    public:
        size_t itemCount() const override
        {
            return m_itemList.size();
        }

    public:
        const SDItem &getItem(size_t) const override;

    public:
        std::u8string getGridHeader     (size_t) const override;
        std::u8string getGridHoverLayout(size_t) const override;

    private:
        void onSelect() override;
};
